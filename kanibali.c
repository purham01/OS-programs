#define _XOPEN_SOURCE 500

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <unistd.h>
#include <semaphore.h>
#include <pthread.h>
#include <string.h>

#define BROJ_STVORENIH_MISIONARA 20
#define BROJ_STVORENIH_KANIBALA 20

pthread_mutex_t m;
pthread_cond_t C, LO, DO, camac_ceka_putnike;

int obalaCamac=1; //lijeva=0,desna=1
int camacPrevozi=0;
int broj_putnika=0;
int broj_kanibala_u_c=0;
int broj_misionara_u_c=0;

struct Clan{
	char oznaka;
	int id;
};

typedef struct Clan Clan;

#define SIZE 20
//stog
int vrhC = -1;
Clan C_clanovi[7]; 

void push(Clan x, Clan* stog, int size, int *vrh)
{
    if ((*vrh) == size - 1)
    {
        printf("\nOverflow!!");
    }
    else
    {
        (*vrh)++;
        stog[*vrh] = x;
    }
}

void pop(Clan* stog,int *vrh)
{
    if ((*vrh) == -1)
    {
        printf("\nUnderflow!!");
    }
    else
    {
        (*vrh)--;
    }


}

void show(Clan* stog,int *vrh)
{
    if ((*vrh) == -1)
    {
        
    }
    else
    {	
        for (int i = (*vrh); i >= 0; --i)
            {
				Clan sadrzaj = stog[i];
				if(i==0)
					printf("%c%d",sadrzaj.oznaka,sadrzaj.id);
				else
					printf("%c%d ",sadrzaj.oznaka,sadrzaj.id);
			}
    }
}

//linked list

struct node {
   Clan data;
   struct node *next;
} ;

struct node *headDO = NULL;
//struct node *currentDO = NULL;
struct node *headLO = NULL;
//struct node *currentLO = NULL;

// display the list
void printList(struct node *head){
   struct node *p = head;

   //start from the beginning
   while(p != NULL) {
      printf("%c%d ",p->data.oznaka,p->data.id);
      p = p->next;
   }
}

//insertion at the beginning
void insertatbegin(Clan data, struct node **head){

   //create a link
   struct node *lk = (struct node*) malloc(sizeof(struct node));
   	lk->data.id = data.id;
	lk->data.oznaka=data.oznaka;

   // point it to old first node
   lk->next = *head;

   //point first to new first node
   *head = lk;

   //printf("Uspjesno dodao clan u listu! %c%d\n",(*head)->data.oznaka,(*head)->data.id);
}


void deletenode(Clan key,struct node **head){
  	struct node *temp = *head, *prev;

	if (temp != NULL && temp->data.id == key.id && temp->data.oznaka == key.oznaka) {
		*head = temp->next;
		return;
	}

	// Find the key to be deleted
	while (temp != NULL && (temp->data.id != key.id || temp->data.oznaka != key.oznaka)) {
		//printf("Trazim clana\n");
		prev = temp;
		temp = temp->next;
	}

	// If the key is not present
	if (temp == NULL) 
	{
		//printf("Clan ne postoji!");
		return;
	}


	// Remove the node
	
	prev->next = temp->next;
	//printf("Clan maknut\n");

}

/*
int searchlist(int key){
   struct node *temp = head;
   while(temp != NULL) {
      if (temp->data == key) {
         return 1;
      }
      temp=temp->next;
   }
   return 0;
}*/

void ispis_stanja(){
	if(obalaCamac){
		printf("C[D]=");
	}
	else{
		printf("C[L]=");
	}
	printf("{ ");
	show(C_clanovi,&vrhC);

	printf(" } LO={ ");

	//show(LO_clanovi,&vrhLO);
	printList(headLO);
	printf("} DO={ ");

	//show(DO_clanovi,&vrhDO);
	printList(headDO);
	printf("}\n\n");

}

void *camac ()
{	
	pthread_mutex_lock (&m);
	while(1){
		broj_putnika=0;
		broj_kanibala_u_c=0;
		broj_misionara_u_c=0;
		camacPrevozi=0;
		if(obalaCamac){
			printf("C: prazan na desnoj obali\n");
		}
		else{
			printf("C: prazan na lijevoj obali\n");
		}
		ispis_stanja();


		while (broj_putnika<3)
			pthread_cond_wait (&camac_ceka_putnike, &m);
		
		printf("C: tri putnika ukrcana, polazim za jednu sekundu\n");
		ispis_stanja();
		pthread_mutex_unlock (&m);
		sleep(1);
		

		pthread_mutex_lock (&m);
		if(obalaCamac){
			printf("C: prevozim s desne na lijevu obalu: ");

		}
		else{
			printf("C: prevozim s lijeve na desnu obalu: ");
		}
		show(C_clanovi,&vrhC);
		printf("\n\n");
		camacPrevozi=1;
		pthread_mutex_unlock (&m);
		sleep(2);

		pthread_mutex_lock (&m);
		if(obalaCamac)
			printf("C: preveo s desne na lijevu obalu: ");
		else
			printf("C: preveo s lijeve na desnu obalu: ");
		show(C_clanovi,&vrhC);
		printf("\n");
		vrhC=-1;

		obalaCamac=1-obalaCamac;
		pthread_cond_broadcast (&C);
		if(obalaCamac){
			pthread_cond_broadcast(&DO);
		}
		else{
			pthread_cond_broadcast(&LO);
		}
		
	}
	
}

void *kanibal (void *p){
	pthread_mutex_lock (&m);
	int id= *((int*)p);
	
	int obala=rand()%2;
	//printf("Obala: %d\n",obala);
	
	Clan clan = {'K',id};

	if(obala){
		printf("K%d: došao na desnu obalu\n",id);	
		insertatbegin(clan,&headDO);
		//printf("%c%d",headDO->data.oznaka,headDO->data.id);
		ispis_stanja();
	}
	else{
		printf("K%d: došao na lijevu obalu\n",id);	
		//push(clan,LO_clanovi,SIZE,&vrhLO);
		insertatbegin(clan,&headLO);
		//printf("%c%d",headLO->data.oznaka,headLO->data.id);
		ispis_stanja();
	}

	while ( obalaCamac!=obala ||
	broj_putnika==7 || 
	(((broj_kanibala_u_c+1) > broj_misionara_u_c)&&broj_misionara_u_c>0) ||
	camacPrevozi==1){
		//printf("K%d zeli u camac\n",id);
		if(obala){
			//printf("K%d ceka na desnoj obali\n",id);
			pthread_cond_wait (&DO, &m);
		}
		else{
			//printf("K%d ceka na lijevoj obali\n",id);
			pthread_cond_wait (&LO, &m);
		}
	}
	//printf("K%d ulazi u camac\n",id);
	broj_putnika++;
	broj_kanibala_u_c++;
	push(clan,C_clanovi,7,&vrhC);
	if(obala){
		deletenode(clan,&headDO);
	}
	else{
		deletenode(clan,&headLO);
	}
	

	printf("K%d: usao u camac\n",id);	
	ispis_stanja();

	pthread_cond_signal (&camac_ceka_putnike); 
	if(obala){
		pthread_cond_broadcast (&DO);		
	}
	else{
		pthread_cond_broadcast (&LO);  
	}
	


	pthread_cond_wait (&C, &m);
	//otpustanje
	pthread_mutex_unlock (&m);
	
	
}

void *misionar(void *p ){
	pthread_mutex_lock (&m);
	int id= *((int*)p);
	int obala=rand()%2;
	//printf("Obala: %d\n",obala);
	
	Clan clan = {'M',id};

	if(obala){
		printf("M%d: došao na desnu obalu\n",id);	
		//push(clan,DO_clanovi,SIZE,&vrhDO);
		insertatbegin(clan,&headDO);
		ispis_stanja();
		
	}
	else{
		printf("M%d: došao na lijevu obalu\n",id);	
		//push(clan,LO_clanovi,SIZE,&vrhLO);
		insertatbegin(clan,&headLO);
		ispis_stanja();
	}

	while ((broj_misionara_u_c==0 && broj_kanibala_u_c>1) || obalaCamac!=obala ||
	broj_putnika==7 || camacPrevozi==1
	){
		//printf("M%d zeli u camac\n",id);
		if(obala){
			pthread_cond_wait (&DO, &m);
		}
		else{
			pthread_cond_wait (&LO, &m);
		}
	}
	//printf("M%d ulazi u camac\n",id);

	broj_putnika++;
	broj_misionara_u_c++;

	push(clan,C_clanovi,7,&vrhC);
	if(obala){
		
		deletenode(clan,&headDO);
		
		
	}
	else{
		
		deletenode(clan,&headLO);
		
	}

	printf("M%d: usao u camac\n",id);	
	ispis_stanja();

	pthread_cond_signal (&camac_ceka_putnike); 
	if(obala){
		pthread_cond_broadcast(&DO);
	}
	else{
		pthread_cond_broadcast(&LO);
	}
	
	pthread_cond_wait (&C, &m);
	//otpustanje
	pthread_mutex_unlock (&m);
	
}

void *misionar_generator( )  {
	int BR[BROJ_STVORENIH_MISIONARA]; pthread_t t[BROJ_STVORENIH_MISIONARA];
	int i=0;
	while(i<BROJ_STVORENIH_MISIONARA){
		//printf("Stvaram novog misionara %d\n",i+1);
		BR[i]=i+1;
		pthread_create(&t[i], NULL, misionar, (void*)&BR[i]);
		i++;
		
		sleep(2);
		
	}
	
}

void *kanibal_generator( )  {
	int BR[BROJ_STVORENIH_KANIBALA]; pthread_t t[BROJ_STVORENIH_KANIBALA];
	int i=0;

	while(i<BROJ_STVORENIH_KANIBALA){
		//printf("Stvaram novog kanibala %d\n",i+1);
		BR[i]=i+1;
		pthread_create(&t[i], NULL, kanibal, (void*)&BR[i]);
		i++;
		sleep(1);
		
	}
	
}


int main ()
{	
	pthread_mutex_init (&m, NULL);
	pthread_cond_init (&C, NULL);
	pthread_cond_init (&LO, NULL);
	pthread_cond_init (&DO, NULL);
	pthread_cond_init(&camac_ceka_putnike,NULL);

	pthread_t dretve[3];
	
	if(pthread_create(&dretve[0], NULL, camac, NULL)) {
		printf("Ne mogu stvoriti novu dretvu!\n");
	}
	

	if(pthread_create(&dretve[1], NULL, misionar_generator, NULL)) {
		printf("Ne mogu stvoriti novu dretvu!\n");
	}

	if(pthread_create(&dretve[2], NULL, kanibal_generator, NULL)) {
		printf("Ne mogu stvoriti novu dretvu!\n");
	}
	
	for(int i=0;i<3;i++)
		pthread_join(dretve[i],NULL);
	
	return 0;
}