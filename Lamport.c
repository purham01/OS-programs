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
#include <signal.h>
#include <stdatomic.h>

int Id1,Id2,Id3; /* identifikacijski broj segmenta */

atomic_int A, *ULAZ, *BROJ;
int n,m;
struct podaciZaDretvu
    {
        int BR;
        int M;
    };

void udi_u_kriticni_odsjecak(int i){
    ULAZ[i]=1;
    int max=0;
    for(int j=1;j<n;j++){
        if(BROJ[max]<BROJ[j])
            max=j;
    }
    BROJ[i]=BROJ[max]+1;
    ULAZ[i]=0;

    for(int j=0;j<n;j++){
        while(ULAZ[j]!=0)
            ;
        while (BROJ[j]!=0 && (BROJ[j]<BROJ[i] || (BROJ[j]== BROJ[i] && j < i)))
        {
            ;
        }
    }
}

void izadi_iz_kriticnog_odsjecka(int i){
    BROJ[i]=0;
}


void *Dretva(void *x){
    int id_Dretve=((struct podaciZaDretvu*)x)->BR;
    int M = ((struct podaciZaDretvu*)x)->M;
    //int id_Dretve =*((int*)x);
    printf("U dretvi smo! Ja sam dretva %d\n", id_Dretve);
    printf("M: %d\n",M);
    
    for(int i=1;i<=M;i++){
        printf("%d. ponavljanje A: %d\n",i,A);
    
        udi_u_kriticni_odsjecak(id_Dretve);
        A++;
        printf("[Dretva %d: A promijenjen na: %d]\n",id_Dretve,A);
        //sleep(1);
        izadi_iz_kriticnog_odsjecka(id_Dretve);
       
    }
}



int main( int argc, char *argv[] )  {
    
/*
    if( argc == 3 ) {
        printf("Dani argument M i N su: %s %s\n", argv[1],argv[2]);
    }
    else if( argc > 3 ) {
        printf("Previse unesenih argumenata.\n");
    }
    else {
        printf("Ocekivana dva argumenta.\n");
    }
    int m = *argv[1]-'0';
    int n = *argv[2]-'0';
    printf("M: %d N: %d\n",m,n);*/

    int m, n;
    printf("Unesite M i N: ");
    scanf("%d %d",&m, &n);

    ULAZ = malloc(n*sizeof(int));
    BROJ = malloc(n*sizeof(int));
    for(int i=0;i<n;i++){
        ULAZ[i]=0;
        BROJ[i]=0;
    }
    //printf("Test polja: %d %d %d %d\n",ULAZ[0],ULAZ[4],BROJ[0],BROJ[4]);
    
    //printf("Uspjesno stvorio polja\n");
    //printf("M: %d\n",m);
    



    pthread_t thr_id[n];
    struct podaciZaDretvu podaci[n];
    for(int i=0;i<n;i++){
        
        podaci[i].BR=i;
        podaci[i].M=m;
        

        if (pthread_create(&thr_id[i], NULL, Dretva, (void *)&podaci[i]) != 0) {
            printf("Greska pri stvaranju dretve!\n");
            exit(1);
        }
        /*if (pthread_create(&thr_id[i], NULL, Dretva, (void *)&BR[i]) != 0) {
            printf("Greska pri stvaranju dretve!\n");
            exit(1);}*/
    }
    //printf("M: %d\n",m);
    //sleep(5);
   

    for(int i=0;i<n;i++){
        pthread_join(thr_id[i],NULL);
    }
    printf("\nNa kraju: A=%d\n",A);
    printf("Uspjesno stvorio dretve\n");
    printf("Uspjesno dosao do kraja programa\n");
    return 0;
}