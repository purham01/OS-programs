#define _XOPEN_SOURCE 700

#include <stdio.h>
#include <time.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <stdbool.h>

#define spavaj 8

#define SIZE 3

struct sigaction act;
struct sigaction tempHelp;

//stog
int vrh = -1, stog[SIZE];
void push(int x)
{
    if (vrh == SIZE - 1)
    {
        printf("\nOverflow!!");
    }
    else
    {
        vrh = vrh + 1;
        stog[vrh] = x;
    }
}

int pop()
{
    if (vrh == -1)
    {
        printf("\nUnderflow!!");
		return -1;
    }
    else
    {
		int rezultat=stog[vrh];
        vrh = vrh - 1;
		return rezultat;
    }


}

void show()
{
    if (vrh == -1)
    {
        printf("-");
    }
    else
    {
        for (int i = vrh; i >= 0; --i)
            {
				int sadrzaj = stog[i];
				printf("%d, reg[%d]", sadrzaj, sadrzaj);
				if(i>0)
					printf(";");
			}
		
    }
	printf("\n\n");
}

int nije_kraj = 1;
int poljeKZ[3] = {0,0,0};
int T_P=0;

struct timespec t0;  //vrijeme pocetka programa 

// postavlja trenutno vrijeme u t0 
void postavi_pocetno_vrijeme()
{
	clock_gettime(CLOCK_REALTIME, &t0);
}

//dohvaca vrijeme proteklo od pokretanja programa 
void vrijeme(void)
{
	struct timespec t;

	clock_gettime(CLOCK_REALTIME, &t);

	t.tv_sec -= t0.tv_sec;
	t.tv_nsec -= t0.tv_nsec;
	if (t.tv_nsec < 0) {
		t.tv_nsec += 1000000000;
		t.tv_sec--;
	}

	printf("%03ld.%03ld:\t", t.tv_sec, t.tv_nsec/1000000);
}

#define PRINTF(format, ...)       \
do {                              \
  vrijeme();                      \
  printf(format, ##__VA_ARGS__);  \
}                                 \
while(0)

void ispisiInfo(){
	PRINTF("K_Z = %d%d%d, T_P=%d, stog: ",poljeKZ[0],poljeKZ[1],poljeKZ[2],T_P);
	show();
}


void ispisiKraj(int gotovo,int nastavlja){
	PRINTF("Zavrsila obrada prekida %d. razine\n",gotovo);
	if(poljeKZ[gotovo-1]==0){
		if(nastavlja==0) PRINTF("Nastavlja se izvođenje glavnog programa\n");
		else{
			PRINTF("Nastavlja se obrada prekida %d. razine\n",nastavlja);
		}
	}
	ispisiInfo();
}

void obradi_prekid(int index, int signalT_P){
	poljeKZ[index]=1;
	if(T_P >= signalT_P){
		PRINTF("Primljen signal %d. razine, ali se samo pamti i ne prosljeduje se procesoru\n", index+1);
		ispisiInfo();
	}
	else
	{
	PRINTF("Primio prekid %d. razine\n",index+1);
	ispisiInfo();

	//spremamo kontekst
	push(T_P);
	T_P = signalT_P;
	poljeKZ[index]=0;

	PRINTF("Pocela obrada prekida %d. razine\n",index+1);
	ispisiInfo();

	for (int i = 1; i <= spavaj; i++) {
		PRINTF("Obraduje se prekid %d. razine: %d\n",index+1,i);
		sleep(1);
	}

	//vracamo kontekst i zavrsavamo
	T_P = pop();
	ispisiKraj(signalT_P, T_P);
	if(poljeKZ[index]==1){
		obradi_prekid(index,signalT_P);
	}
	
	}
}

void obradi_prekid_1_razine(){
	obradi_prekid(0,1);
}

void obradi_prekid_2_razine(){
	obradi_prekid(1,2);
}

void obradi_prekid_3_razine(){
	obradi_prekid(2,3);
}

void kraj_programa(){
	nije_kraj=0;
}

void inicijalizacija(){
	//struct sigaction act;

	//prekid 1 razine (SIGUSR1)
	act.sa_handler = obradi_prekid_1_razine;
	sigemptyset(&act.sa_mask);
	act.sa_flags = SA_NODEFER;
	//maskiranje
	sigaction(SIGUSR1, &act, NULL);


	//prekid 2 razine (SIGUSR2)
	act.sa_handler = obradi_prekid_2_razine;
	sigaction(SIGUSR2, &act, NULL);


	//prekid 3 razine (SIGTERM)
	act.sa_handler = obradi_prekid_3_razine;
	sigaction(SIGTERM,&act,NULL);

	//samo nacin da mirno izademo iz programa, bez CTRL-C
	act.sa_handler = kraj_programa;
	sigaction(SIGTSTP, &act, NULL);
}

int main(void){
	postavi_pocetno_vrijeme();
	inicijalizacija();

	PRINTF("Program s PID=%ld krenuo s radom\n", (long) getpid());
	ispisiInfo();

	/* neki posao koji program radi; ovdje samo simulacija */
	int i = 1;
	while(nije_kraj) {

		//PRINTF("Program: iteracija %d\n", i++);
		// za prekid trece razine se ne moze nikad dogoditi
		if (poljeKZ[1]==1)
		{
			PRINTF("Promijenio se T_P, prosljeđujem prekid 2. razine procesoru\n");
			obradi_prekid_2_razine();
		}
		else if (poljeKZ[0]==1)
		{
			PRINTF("Promijenio se T_P, prosljeđujem prekid 1. razine procesoru\n");
			obradi_prekid_1_razine();
		}

		
		sleep(1);
	}

	PRINTF("Program s PID=%ld zavrsio s radom\n", (long) getpid());
	return 0;
}