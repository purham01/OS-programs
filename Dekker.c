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

#define BROJ_ZASTAVICA 2

int Id1,Id2,Id3; /* identifikacijski broj segmenta */
int *A, *PRAVO, *ZASTAVICE;

int childPID=0;

void brisi(int sig)
{
   /* oslobađanje zajedničke memorije */
    if(childPID!=0){
        kill(childPID,2);
    }
    (void) shmdt((char *) A);
    (void) shmctl(Id1, IPC_RMID, NULL);
    (void) shmdt((char *) PRAVO);
    (void) shmctl(Id2, IPC_RMID, NULL);
    (void) shmdt((char *) ZASTAVICE);
    (void) shmctl(Id3, IPC_RMID, NULL);

    exit(0);
}

void inicijalizacija(){
    Id1 = shmget(IPC_PRIVATE, sizeof(int), 0600);
    Id2 = shmget(IPC_PRIVATE, sizeof(int), 0600);
    Id3 = shmget(IPC_PRIVATE, BROJ_ZASTAVICA*sizeof(int), 0600);
    
    if (Id1 == -1 || Id2 ==-1||Id3==-1)
      exit(1);  /* greška - nema zajedničke memorije */
 
    A = (int *) shmat(Id1, NULL, 0);
    *A = 0;
    //M = (int *) shmat(Id2,NULL,0);
    //*M=0;
    PRAVO = (int *) shmat(Id2, NULL, 0);
    *PRAVO = 0;
    ZASTAVICE = (int *)shmat(Id3,NULL,0);
    for(int i=0;i<BROJ_ZASTAVICA;i++){
        ZASTAVICE[i]=0;
    }
    ZASTAVICE[0]=1;
   //za brisanje u slucaju prekida
    struct sigaction act;
    act.sa_handler = brisi;
    sigaction(SIGINT, &act, NULL);
}

void udi_u_kriticni_odsjecak(int i,int j){
    ZASTAVICE[i]=1;
    //printf("Usao u kriticni odsjecak! Pravo: %d Zastavice: %d %d I:%d J:%d\n",*PRAVO, ZASTAVICE[0],ZASTAVICE [1],i,j);
    while(ZASTAVICE[j]!=0)
    {
        //printf("Pravo: %d J: %d\n",*PRAVO,j);
        if((*PRAVO)==j)
        {   
            //printf("Ja sam proces %d i cekam\n",i);
            ZASTAVICE[i]=0;
            while ((*PRAVO)==j)
            {
                ;//radimo nista
            }
            ZASTAVICE[i]=1;
        }
    }
}

void izadi_iz_kriticnog_odsjecka(int i, int j){
    (*PRAVO)=j;
    ZASTAVICE[i]=0;
    //ZASTAVICE[j]=1;
    //printf("Izasao iz kriticnog odsjecka! Pravo: %d Zastavice: %d %d I:%d J:%d\n",*PRAVO, ZASTAVICE[0],ZASTAVICE [1],i,j);
}

int main( int argc, char *argv[] )  {
    inicijalizacija();
/*
    if( argc == 2 ) {
        printf("Dani argument M je: %s\n", argv[1]);
    }
    else if( argc > 2 ) {
        printf("Previse unesenih argumenata.\n");
    }
    else {
        printf("Ocekivan jedan argument.\n");
    }

    char M_temp = *argv[1];
    int M = atoi(*M_temp);*/
    int M;
    printf("Unesite M: ");
    scanf("%d",&M);

    //printf("%c %d",M_temp, *M);

    int idOvogProcesa, idDrugogProcesa;
    switch (childPID=fork())
    {
    case -1:
        printf("Greska! Nije uspio fork!");
        break;
    case 0:
        idOvogProcesa=1;
        idDrugogProcesa=0;    
        break;
    default:
        //printf("Child PID:%d",childPID);
        idOvogProcesa=0;
        idDrugogProcesa=1;
        break;
    }
    
    
    //dekker
    for(int i=1;i<=M;i++){
        printf("M: %d\n",M);
        printf("Ja sam proces %d\n",idOvogProcesa);
        printf("%d. ponavljanje A: %d\n",i,*A);

        udi_u_kriticni_odsjecak(idOvogProcesa,idDrugogProcesa);
            (*A)++;
            printf("[Proces %d: A promijenjen na: %d]\n",idOvogProcesa,*A);
            //sleep(1);
        izadi_iz_kriticnog_odsjecka(idOvogProcesa,idDrugogProcesa);
       
        //printf("Ja sam proces %d\n",idOvogProcesa);
        //printf("Kraj %d. ponavljanja: M: %d A: %d\n",k,M,*A);
        //printf("\n");
    }

    if(idOvogProcesa==1){
        childPID=0;
        exit(0);
    }
    (void) wait(NULL);
    
    printf("\nNa kraju: A=%d\n",*A);
  
    brisi(0);
    
    return 0;
}