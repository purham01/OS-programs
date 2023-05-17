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
#include <string.h>
#include <assert.h>

#define MAXCOM 1000 // max number of letters to be supported

#define clear() printf("\033[H\033[J")

int child_PID=0;
char** var_Path;
char pocetni_direktorij[100]; 
char trenutni_direktorij[100];

int number_of_elements(char* pocetni_string, const char granicnik){
    //brojanje elemenata u stringu
    int brojElemenata     = 0;
    char* temp = pocetni_string;
    char* zadnji_granicnik = 0;
    while (*temp)
    {   
        
        if (granicnik == *temp)
        {
            brojElemenata++;
            zadnji_granicnik = temp;
        }
        temp++;
    }

    //dodavanje prostora za zadnji token, ako ovaj nije na kraju
    if (zadnji_granicnik < (pocetni_string + strlen(pocetni_string) - 1) ) 
        brojElemenata++;

    //da znamo gdje je kraj tokena
    brojElemenata++;

    return brojElemenata;
}

void str_split(char** rezultat, char* pocetni_string, const char granicnik,int brojElemenata)
{
    char granicnici[2];
    granicnici[0] = granicnik;
    granicnici[1] = 0;

    if (rezultat)
    {
        char *oznaka;
        int idx  = 0;
        char* token = strtok_r(pocetni_string, granicnici,&oznaka);

        //upisivanje tokena u rezultat
        while (token)
        {
            
            assert(idx < brojElemenata);
            *(rezultat + idx++) = strdup(token);
            token = strtok_r(NULL, granicnici, &oznaka);
            
        }
        assert(idx == brojElemenata - 1);//provjera jesmo li dosli do kraja
        *(rezultat + idx) = 0;
    }
}


void naredba_cd(char* path){
    printf("\nPath za unijeti: %s\n",path);
    if(chdir(path)!=0)
        fprintf( stderr, "cd: The directory '%s' does not exist\n",path);
    else{
        printf("Uspjesno promijenio path\n");
    }
    
}
void nas_exit(){
    printf("Unesena naredba exit. Zbogom.\n");
    //free(var_Path);
    exit(0);
}


void pokreni_vanjski_program(char** parametri){
    //int fork_rez= fork();
    //child_PID=fork_rez;
    switch (child_PID=fork())
    {
    case 0:
        //printf("Ja sam dijete!\n");
        //printf("Argumenti: %s %s\n",ulazni_string[0],*(ulazni_string+1));
        setpgid(0,0);
        if(execve(parametri[0],parametri,NULL)==-1){
            fprintf(stderr,"fsh: Unknown command '%s'\n",parametri[0]);
        }
        exit(0);
        break;
    case -1: printf("Greška! Fork nije uspio!"); break;
    default:
        break;
    }
    (void) wait(NULL);
    child_PID=0;
}

int obradi_naredbu(char** ulazni_string)
{
    int rez=0;
    int broj_mogucih_naredbi=2,switchArgument=0;
    char* lista_mogucih_naredbi[broj_mogucih_naredbi];

    lista_mogucih_naredbi[0]="cd";
    lista_mogucih_naredbi[1]="exit";

    for(int i=0;i<broj_mogucih_naredbi;i++){
        //printf("%s ? %s\n",lista_mogucih_naredbi[i],ulazni_string[0]);
        if (strcmp(lista_mogucih_naredbi[i],ulazni_string[0])==0)
        {
            
            switchArgument=i+1;
            break;
        }
    }
    //printf("Argument za switch %d\n",switchArgument);

    switch (switchArgument)
    {
    case 1:
        naredba_cd(ulazni_string[1]);
        break;
    case 2:
        nas_exit();
        break;
    default:
        if(ulazni_string[0][0]=='.' ||ulazni_string[0][0]=='/' || access(ulazni_string[0],F_OK)==0){
            pokreni_vanjski_program(ulazni_string);
        }
        else{
            int nije_nasao_nista=1;
            char* ime_naredbe= ulazni_string[0];
            /*if(strcmp(ime_naredbe,"echo")==0){
                rez=1;
            }*/
            for (int i = 0; *(var_Path + i); i++)
            {
                char new_path[MAXCOM]="";
                strcat(new_path,*(var_Path + i));
                strcat(new_path,"/");
                strcat(new_path, ime_naredbe);
                //printf("New path: %s\n",new_path);

                if(access(new_path,F_OK)==0){
                    ulazni_string[0]=new_path;
                    pokreni_vanjski_program(ulazni_string);
                    nije_nasao_nista=0;
                    rez=1;
                    break;
                }
            }
            
            if(nije_nasao_nista){
                fprintf(stderr,"fsh: Unknown command '%s'\n",ime_naredbe);
            }
        }
        break;
    }
    return rez;
}


void obradi_prekid(int sig){
    //printf("CHILD PID: %d\n",child_PID);
    if(child_PID == 0){
        printf("\n");
    }
    else{
        if(kill(child_PID,sig)==0){
            printf("\nChild process killed successfully!\n");
            child_PID=0;
        }
        else{
            printf("Failed to kill child, what now!?\n");
        }
    }
}

char** obradi_varijablu_path(){
    char *sadrzaj = getenv("PATH");
    //printf("%s\n",sadrzaj);
     

    int brojElemenata = number_of_elements(sadrzaj,':');
    char** rez = (char**)malloc(sizeof(char*) * brojElemenata);
    str_split(rez, sadrzaj,':',brojElemenata);


    return rez;
}

void inicijalizacija(){
    struct sigaction act;

    act.sa_handler = obradi_prekid;
    
    sigaction(SIGINT, &act, NULL);
    var_Path= obradi_varijablu_path();
}



int main(void)
{
    
    inicijalizacija();
    //ispis svih path varijabli
    /*for (int i = 0; *(var_Path + i); i++)
    {
        printf("PATH %d=[%s]\n",i+1, *(var_Path + i));
    }*/
    char ulaz[MAXCOM];
    
    
    getcwd(pocetni_direktorij, 100);

    while(1){
        memset(ulaz, 0, sizeof(ulaz));
        printf("fsh> ");
        getcwd(trenutni_direktorij, 100);
        //printf("Trenutni direktorij: %s Pocetni direktorij: %s: ",trenutni_direktorij,pocetni_direktorij);
        if(strcmp(pocetni_direktorij, trenutni_direktorij)!=0){
            printf("%s: ", trenutni_direktorij);
        }
        
        scanf("%[^\n]%*c", ulaz);
       


        if(strlen(ulaz)<=0){
            printf("\nUnesen prazan niz. Izlazim iz programa.\n");
            nas_exit();
        }
        else{
            //printf("[%s]\n",ulaz);
            
            int brojElemenata = number_of_elements(ulaz,' ');

            char** tokeni;
            tokeni= (char**) malloc(sizeof(char*) * brojElemenata);
            str_split(tokeni,ulaz,' ',brojElemenata);
            if(obradi_naredbu(tokeni)==0){
                //ciscenje, doduse neke naredbe nam nedaju da pocistimo za njima
                for (int i = 0; *(tokeni + i); i++)
                {
                    free(*(tokeni + i));
                }
                
            }   
            free(tokeni); 
        }

        
        
       
    }

    return 0;
}
