#define _XOPEN_SOURCE 500

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <unistd.h>
#include <semaphore.h>


#define max_klijenata 3
#define radno_vrijeme_u_s 20
#define rad_na_frizuri 5
int Id; /* identifikacijski broj segmenta */
sem_t *binarniSemafor1, *binarniSemafor2, *binarniSemafor3, *sjedalo, *frizerka_spava;
int *otvoreno, *krajRadnogVremena, *br_klijenata, *frizerkaSpavaPostavljeno; 


void inicijalizacija(){

    Id = shmget(IPC_PRIVATE, sizeof(int), 0600);
    otvoreno = shmat (Id, NULL, 0);
	shmctl (Id, IPC_RMID, NULL);
    *otvoreno=0;

    Id = shmget(IPC_PRIVATE, sizeof(int), 0600);
    br_klijenata = shmat (Id, NULL, 0);
	shmctl (Id, IPC_RMID, NULL);
    *br_klijenata=0;

    Id = shmget(IPC_PRIVATE, sizeof(int), 0600);
    krajRadnogVremena = shmat (Id, NULL, 0);
	shmctl (Id, IPC_RMID, NULL);
    *krajRadnogVremena=0;

    Id = shmget(IPC_PRIVATE, sizeof(int), 0600);
    frizerkaSpavaPostavljeno = shmat (Id, NULL, 0);
	shmctl (Id, IPC_RMID, NULL);
    *frizerkaSpavaPostavljeno=0;


    Id = shmget(IPC_PRIVATE, sizeof(sem_t), 0600);
    frizerka_spava = shmat (Id, NULL, 0);
	shmctl (Id, IPC_RMID, NULL);
    sem_init (frizerka_spava, 1, 0);

    Id = shmget(IPC_PRIVATE, sizeof(sem_t), 0600);
    sjedalo = shmat (Id, NULL, 0);
	shmctl (Id, IPC_RMID, NULL);
    sem_init (sjedalo, 1, 0);

    Id = shmget(IPC_PRIVATE, sizeof(sem_t), 0600);
    binarniSemafor1 = shmat (Id, NULL, 0);
	shmctl (Id, IPC_RMID, NULL);
    sem_init (binarniSemafor1, 1, 1);

    Id = shmget(IPC_PRIVATE, sizeof(sem_t), 0600);
    binarniSemafor2 = shmat (Id, NULL, 0);
	shmctl (Id, IPC_RMID, NULL);
    sem_init (binarniSemafor2, 1, 1);

    Id = shmget(IPC_PRIVATE, sizeof(sem_t), 0600);
    binarniSemafor3 = shmat (Id, NULL, 0);
	shmctl (Id, IPC_RMID, NULL);
    sem_init (binarniSemafor3, 1, 1);
}

void brisi(){
    
    shmdt(otvoreno);

    
    shmdt(br_klijenata);

   
    shmdt(krajRadnogVremena);

    shmdt(frizerkaSpavaPostavljeno);

    sem_destroy (sjedalo);
    shmdt(sjedalo);
    sem_destroy (frizerka_spava);
    shmdt(frizerka_spava);
    sem_destroy (binarniSemafor1);
    shmdt(binarniSemafor1);
    sem_destroy (binarniSemafor2);
    shmdt(binarniSemafor2);

    sem_destroy (binarniSemafor3);
    shmdt(binarniSemafor3);
}


void frizerka(){
    printf("Frizerka: Otvaram salon\n");
    sem_wait(binarniSemafor1);
    *otvoreno = 1;
    printf("Frizerka: Postavljam znak OTVORENO\n");
    sem_post(binarniSemafor1);

    while(1){
        if(*krajRadnogVremena && (*otvoreno)){
            sem_wait(binarniSemafor1);
            *otvoreno = 0;
            printf("Frizerka: Postavljam znak zatvoreno\n");
            sem_post(binarniSemafor1);
        }

        sem_wait(binarniSemafor2);
        int broj = (*br_klijenata);
        sem_post(binarniSemafor2);
        if(broj>0){
            //radi na frizuri
            printf("Frizerka: Idem raditi na klijentu %d\n",broj);
            sem_post(sjedalo);
            printf("Preostalo je još %d klijenata\n",broj-1);
            //sem_wait(klijenti);
            for(int i=0;i<rad_na_frizuri;i++){
                printf("Frizerka: Radim na frizuri %d klijentu %d\n", i+1, broj);
                sleep(1);
            }
            printf("Frizerka: Klijent %d gotov\n",broj);
        }
        else if(!*krajRadnogVremena){
            printf("Frizerka: Spavam dok klijenti ne dođu\n");
            sem_wait(binarniSemafor3);
            (*frizerkaSpavaPostavljeno)=1;
            sem_post(binarniSemafor3);
            sem_wait(frizerka_spava);
            
        }
        else{
            printf("Frizerka: Zatvaram salon\n");
            exit(0);
        }
    }   
}

void klijent(int klijentID){
    printf("\tKlijent(%d): Želim na frizuru\n",klijentID+1);
    sem_wait(binarniSemafor2);
    if((*otvoreno) && (*br_klijenata)<max_klijenata){
        (*br_klijenata)++;
        printf("\tKlijent(%d): Ulazim u cekaonicu (%d)\n",klijentID+1,*br_klijenata);
        sem_post(binarniSemafor2);

        sem_wait(binarniSemafor3);
        if((*frizerkaSpavaPostavljeno)==1)
        {
            sem_post(frizerka_spava);
            (*frizerkaSpavaPostavljeno)=0;
        }
        sem_post(binarniSemafor3);

        //sem_post(klijenti);
        sem_wait(sjedalo);

        sem_wait(binarniSemafor2);
        (*br_klijenata)--;
        sem_post(binarniSemafor2);
    	printf("\tKlijent(%d): Frizerka mi radi frizuru\n",klijentID+1);
        exit(0);
    }
    else{
        printf("\tKlijent(%d): Danas nista od frizure, vratit cu se sutra\n",klijentID+1);
        sem_post(binarniSemafor2);
        exit(0);
    }
}

void radno_vrijeme(){
    sleep(radno_vrijeme_u_s);
    sem_wait(binarniSemafor3);
    if((*frizerkaSpavaPostavljeno)==1)
        {
            sem_post(frizerka_spava);
            (*frizerkaSpavaPostavljeno)=0;
        }
    sem_post(binarniSemafor3);
    *krajRadnogVremena=1;
    exit(0);
}


int main(void){
    int noOfClients=0;
    int brojProcesa=0;

    inicijalizacija();
    switch (fork())
    {
    case 0:
        frizerka();
        break;
    case -1:
        printf("Nije uspjelo stvaranje procesa frizerka\n");
        break;
    default:
        brojProcesa++;
        break;
    }
    sleep(2);

    switch (fork())
    {
    case 0:
        radno_vrijeme();
        break;
    case -1:
        printf("Nije uspjelo stvaranje procesa radno vrijeme\n");
        break;
    default:
        brojProcesa++;
        break;
    }
   

    for(int i =0;i<5;i++){
        switch (fork())
            {
            case 0:
                klijent(noOfClients);
                break;
            case -1:
                printf("Stvaranje klijenta nije uspjelo\n");
                break;
            default:

                brojProcesa++;
                noOfClients++;
                sleep(1);
                
                break;
            }
            
        }

    sleep(10);
    for(int i =0;i<2;i++){
        switch (fork())
            {
            case 0:
                klijent(noOfClients);
                break;
            case -1:
                printf("Stvaranje klijenta nije uspjelo\n");
                break;
            default:

                brojProcesa++;
                noOfClients++;
                sleep(1);
                
                break;
            }
            
        }
    sleep(5);

    for(int i =0;i<2;i++){
        switch (fork())
            {
            case 0:
                klijent(noOfClients);
                break;
            case -1:
                printf("Stvaranje klijenta nije uspjelo\n");
                break;
            default:

                brojProcesa++;
                noOfClients++;
                sleep(1);
                
                break;
            }
            
        }
    for(int j=0;j<brojProcesa;j++)
        (void) wait(NULL);

    brisi();

    return 0;
}