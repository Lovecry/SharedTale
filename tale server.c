#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#define MAX 8000
#define MAXF 1024000
#define MAX_PROJECTS 250
 
char buff[MAX], buff2[MAX], buff_rep[MAX], buff_tale[MAXF], buff_tale2[MAXF];
int pid;
int sd_client;
char buff3[MAX];
char buff4[MAX]; /* Variabile utilizzata per scrivere il nuovo capitolo, contiene il titolo del capitolo */
char buff5[MAX]; /* Variabile utilizzata per scrivere il nuovo capitolo, contiene il nome dell'autore */
char project_list[MAX_PROJECTS][MAX]; /* Matrice contenente tutti i progetti */
int project_list_size=0; /* numero progetti */
int semaphore_list; /* set semafori */
int buff6=1;
int buff7=1;
int buff8=1;
int buff9=1;
char buff_div[]="\n--------------------------------------------------------------------------------------------------\n\n";
FILE *users_list, *project, *bufffile;

/* -----------------------------------------------------------Prototipi--------------------------------------------------------------------- */
int socket_conn();
void login();
void signin();
void cleaner();
void menu();
void usermenu();
void reader();
void writermenu();
void writernew();
void writerold();
void writeradd();
void initializeprojectlist();
void initializesemaphore();


/* -------------------------------------------------------L'azzera buffers------------------------------------------------------------------ */
void cleaner()
{
	int i=0;
	for (i=0; i<MAX; i++)
	{
		buff[i]='\0';
		buff2[i]='\0';
		buff_rep[i]='\0';
	}
	for (i=0; i<MAXF; i++)
	{
		buff_tale[i]='\0';
		buff_tale2[i]='\0';
	}
}


/* ----------------------------------------------------------Connessione-------------------------------------------------------------------- */
int socket_conn()
{
/* Creo due istanze (client e server) della struttura sockaddr_in la quale comprende (sin_family; sin_port; sin_addr; sin_zero) */

	struct sockaddr_in server_addr; /* indirizzo del server */
	struct sockaddr_in client_addr; /* indirizzo del client */
	int sd_server; /* i socket descriptor usati per identificare server e client */
	sd_server = socket(AF_INET, SOCK_STREAM, 0); /* Creazione socket descriptor per il server, socket ricaccia -1 se fallisce else fd */
	if (sd_server < 0)
 	       	{
 	           	perror("Errore nella creazione socket\n"); /* AF_INET + SOCK_STREAM ->TCP,utilizzo del protocollo TCP (IPPROTO_TCP). */
 	          	exit(-1);
 	       	}						      
	printf("\nSocket creato, il descrittore socket è : %d\n", sd_server);  

/* Inseriamo nella struttura alcune informazioni */
    
	server_addr.sin_family = AF_INET; /* la famiglia dei protocolli */
	server_addr.sin_port = htons(1745); /* la porta in ascolto */
	server_addr.sin_addr.s_addr = INADDR_ANY; /* dato che è un server bisogna associargli l'indirizzo della macchina su cui sta girando */
    						  /* sin_addr è una struttura e comprende s_addr, INADDR_ANY accetta ogni richiesta */

/* Assegnazione del processo alla socket tramite la funzione BIND */
    
	if(bind(sd_server, (struct sockaddr*) &server_addr, sizeof(server_addr)) < 0) /* sokaddr* = puntatore; &server_addr = indirizzo */
	{
		printf("Errore di binding\n");
		close(sd_client);
		exit(0);
	}
    
/* Si mette in ascolto con un massimo di 20 connessioni */
    
	listen (sd_server, 20);
    
/* Essendo un server multithreading, accetterà piu connessioni per volta, devo distinguere il child dal parent */
     
	unsigned int address_size = sizeof(client_addr); /* dimensione della struttura client_addr */	
	printf("In Attesa di connessioni client...\n");
	while(1)
	{
		sd_client = accept(sd_server, (struct sockaddr*) &client_addr, &address_size); /* Accettiamo il client,nuovo file descriptor */
		if (sd_client < 0)
 	       	{
 	           	perror("Connessione non accettata!\n");
 	          	exit(-1);
 	       	}
		printf("Connessione in entrata accettata, il nuovo descrittore socket è : %d\n", sd_client);
        	pid = fork(); /* Creiamo un processo figlio per comunicare con il client accettato in modo che il padre puo accettarne altri */
		if (pid < 0)
		{
			printf("Impossibile effetuare fork!\n");
			close(sd_client);
			exit(-1);
		}
		if (pid == 0)
		{    
			printf("Child born ---> Pid = %d\n", getpid());
			return;
		}
		else
		{
			printf("I'm Parent Pid = %d\n", getpid());
			printf("Io, padre, torno in ascolto nuove connessioni e chiudo la mia connessione con il client\n");
			close(sd_client);
		}
	}
	return 0;
}

/* ----------------------------------------------------------Semaphore---------------------------------------------------------------------- */
void initializesemaphore()
{
	semaphore_list=semget(ftok("/dev/null",1), MAX_PROJECTS, 0666);
	if(semaphore_list!=-1)
	{
		semctl(semaphore_list, 0, IPC_RMID);
		printf("Semaforo rimosso\n");
	}
	semaphore_list=semget(ftok("/dev/null",1), MAX_PROJECTS, 0666 | IPC_CREAT | IPC_EXCL);
	int i;
	for(i=0; i<MAX_PROJECTS; i++)
	{
		semctl(semaphore_list, i, SETVAL, 1);
	}
	printf("Semaforo ID : %d\n", semaphore_list);
}

/* -------------------------------------------------------------Main------------------------------------------------------------------------ */
int main()
{
	initializesemaphore();
	socket_conn();
	while(1)
	{
		menu();
	}
	return EXIT_SUCCESS;
}

/* --------------------------------------------------------Menu Principale------------------------------------------------------------------ */
void menu()
{    
	cleaner();
	printf("In attesa di ricevere dal client la scelta nel menù\n");
	recv(sd_client, buff, sizeof(char), 0);
	printf("Ricevuta la scelta del menù : %c\n", *buff);
	switch(buff[0])
	{
		case '1':
        	{
			login();
           		break;
       		}
		case '2':
        	{
            		signin();
            		break;
        	}
		case '0':
        	{
			close(sd_client);
            		exit(0);
            		break;
        	}
		default :
        	{
            		break;
        	}          
	}
	return;
}


/* ---------------------------------------------------------Registrazione------------------------------------------------------------------- */
void signin()
{
	cleaner();
	printf("In attesa di ricevere username e password nuovo iscritto\n");
	recv(sd_client, buff, MAX, 0); /* Riceviamo dal client l'username e password della nuova registrazione */
	printf("Ricevuti username e password del nuovo iscritto : %s\n", buff);
	users_list = fopen("users.txt", "a+"); /* Apriamo un file users.txt con pemessi scrittura/lettura */
	if (users_list == NULL) /* Se l'fopen non va a buon fine, ritona NULL, qui controlliamo */
	{
		send(sd_client, "0", sizeof(char), 0);
		return;
	}
	send(sd_client, "1", sizeof(char), 0);
	while(!feof(users_list))
	{
		fgets(buff2, MAX, users_list);
		if((strstr(buff2, buff)) != NULL)
		{
			send(sd_client, "0", sizeof(char), 0);
			return;
		}
	}
	fclose(users_list);
	send(sd_client, "1", sizeof(char), 0);
	users_list = fopen("users.txt", "a"); /* Apriamo il file di testo in scrittura, ci posizioniamo alla fine file */
	fwrite(buff, strlen(buff), 1, users_list); /* Scriviamo "1" elemento di dimensione strlen(buff) in user_list a partire da buff */
	fclose(users_list);
	return;
}



/* -------------------------------------------------------------Login----------------------------------------------------------------------- */
void login()
{
	cleaner();
	printf("In attesa di ricevere username e password per il login\n");
	recv(sd_client, buff, MAX, 0); /* Riceviamo username e password */
	printf("Ricevuti username e password utente per il login : %s\n", buff);
	users_list = fopen("users.txt", "r"); /* Apriamo il file users.txt in sola lettura */
	if (users_list == NULL) /* Se non si riesce ad aprire il file restituiamo un errore */
    	{
        	send(sd_client, "0", sizeof(char), 0);
        	return;
    	}
	send(sd_client, "1", sizeof(char), 0); /* Siamo riusciti ad aprire il file */
	while(!feof(users_list)) /* Passiamo in rassegna il file user_list per poterlo poi controllare */
	{
        	fgets(buff2, MAX, users_list); /* Leggiamo da user_list e mettiamo in buff2 */
		if((strstr(buff2, buff)) != NULL) /* Troviamo un riscontro di buff in buff2 e spediamo l'avvenuto login */
        	{
			send(sd_client, "1", sizeof(char), 0);
			buff6=1;
			while(buff6==1)
			{
				usermenu();
			}
			return;
        	}
	}
	send(sd_client, "0", sizeof(char), 0);
	return;
}

/* ------------------------------------------------------Usermenu dopo Login---------------------------------------------------------------- */
void usermenu()
{ 
	cleaner();
	printf("In attesa di ricevere dal client la scelta nel UserMenù\n");
	recv(sd_client, buff, sizeof(char), 0);
	printf("Ricevuta la scelta del UserMenù : %c\n", *buff);
	switch(buff[0])
	{
		case '1':
        	{
            		reader();
            		break;
        	}
		case '2':
        	{
			buff7=1;
			while(buff7==1)
			{
            			writermenu();
			}
			break;
        	}
		case '3':
        	{
			buff6=0;
            		return;
			break;
        	}
		default :
		{
			break;
		}
	}
	return;
}


/* -----------------------------------------------------------WriterMenu-------------------------------------------------------------------- */
void writermenu()
{
	cleaner();
	printf("In attesa di ricevere dal client la scelta nel WriterMenu\n");
	recv(sd_client, buff, sizeof(char), 0);
	printf("Ricevuta la scelta del WriterMenù : %c\n", *buff);	
	switch(buff[0])
	{
		case '1':
        	{
			buff8=1;
			while(buff8==1)
			{
            			writernew();
			}
            		break;
        	}
		case '2':
        	{
			buff8=1;
			while(buff8==1)
			{
            			writerold();
			}
            		break;
        	}
		case '3':
        	{
			buff7=0;
            		return;
			break;
        	}
		default : 
		{
			break;
		}
	}
	return;
}

/* -----------------------------------------------------------WriterNew-------------------------------------------------------------------- */
void writernew()
{
	cleaner();
	initializeprojectlist();
	printf("In attesa di ricevere il nome del nuovo progetto\n");
	recv(sd_client, buff, MAX, 0); /* Riceviamo il nome del nuovo progetto */
	printf("Ricevuto il nome del nuovo progetto : %s\n", buff);
	strcat(buff,".txt");

/* Apriamo il file del nuovo progetto in lettura per vedere se esiste, se c'è, il progetto gia esiste altrimenti poi lo creiamo */

	bufffile=fopen(buff, "r");
	if (bufffile!=NULL)
	{	
		send(sd_client, "0", sizeof(char), 0); /* Il file esiste già, il nome scelto è esistente */ 
		fclose(bufffile);
		return;
	} 
	send(sd_client, "1", sizeof(char), 0); /* Il progetto non esiste e possiamo crearlo */

/* Apriamo il nuovo progetto in scrittura e lettura, creandolo ed apriamo progetti.txt per aggiungere il nuovo progetto */

	project = fopen(buff, "a+"); /* Apriamo il file di testo in lettura scrittura creandolo se non esiste */	
	if (project == NULL) /* Se non si riesce ad aprire il file restituiamo un errore */
    	{
        	send(sd_client, "0", sizeof(char), 0); /* Informiamo il client che non si è potuto aprire il progetto */
        	return;
    	}
	send(sd_client, "1", sizeof(char), 0); /* Informiamo il client dell'avvenuta creazione/apertura del progetto */
	fclose(project);
	bufffile=fopen("progetti.txt", "a+");
	fwrite(buff, strlen(buff), 1, bufffile); /* Scriviamo "1" elemento di dimensione strlen(buff) in bufffile a partire da buff */
	fwrite("\n", sizeof(char), 1, bufffile); /* Inseriamo il carattere di fine linea in quanto non viene inserito in auto */
	fclose(bufffile);
	system("sed -i \"s/.txt//g\" progetti.txt"); /* Dobbiamo togliere il .txt al nuovo progetto inserito */
	
/* Entriamo nel metodo di aggiunta nuovo capitolo ed al ritorno usciamo dai loop tornando all'usermenu */

	writeradd();
	buff8=0;
	buff7=0;
	return;
}

/* -----------------------------------------------------------WriterOld-------------------------------------------------------------------- */
void initializeprojectlist()
{
	bufffile=fopen("progetti.txt", "r");
	if (bufffile==NULL)
	{	
		system("ls *.txt -1 | grep -v users.txt | grep -v progetti.txt > progetti.txt"); /* list of txt in progetti.txt */
		system("sed -i \"s/.txt//g\" progetti.txt"); /* Togliamo dal nome dei progetti il .txt */
		printf("Il file progetti.txt è stato reinizializzato\n");
	} 
	else
	{	
		fclose(bufffile);
	}
	return;
}

/* -----------------------------------------------------------WriterOld-------------------------------------------------------------------- */
void writerold()
{
	cleaner();
	initializeprojectlist();
	
/* Apriamo il file progetti in lettura per vedere se esiste, se non c'è, non ci sono progetti, se c'è, inviamo al client la lista */

	project = fopen("progetti.txt", "r"); /* Apriamo progetti.txt in lettura per prendere il nome dei progetti */	
	if (project == NULL) /* Se non si riesce ad aprire il file restituiamo un errore */
    	{
        	send(sd_client, "0", sizeof(char), 0); /* Informiamo il client che non si è potuto aprire la lista progetti */
		buff8=0;
        	return;
    	}
	send(sd_client, " \n", 2, 0);
	project_list_size=0;
	while((fgets(buff_tale, MAXF, project))!=NULL) /* Passiamo in rassegna il file progetti.txt */
	{
		send(sd_client, buff_tale, strlen(buff_tale), 0); /* Inviamo al client i nomi di tutti i progetti in progetti.txt */
		strcpy(project_list[project_list_size], buff_tale);
		project_list_size++;
	}
	send(sd_client, "1", 2, 0); /* Inviamo il check di fine scansione file */
	fclose(project);

/* Riceviamo il nome del progetto da modificare, gli aggiungiamo.txt e cerchiamo di aprirlo, se ci riusciamo ok senno sped. errore  */

	printf("In attesa di ricevere il nome del progetto per l'aggiunta capitolo\n");
	recv(sd_client, buff, MAX, 0); /* Riceviamo il nome del progetto che si vuole aprire per la modifica */
	printf("Ricevuto il nome del progetto per l'aggiunta capitolo : %s\n", buff);
	int projectID=-1;
	int k=0;
	for(k=0; k<project_list_size; k++) /* Scorriamo l'array contenente il nome dei progetti */
	{
		if(strncmp(buff, project_list[k], strlen(buff))==0) /* Se il nome ricevuto corrisponde, ci segnamo l'ID corrispondete */
		{
			projectID=k;
		}
	}
	printf("L'utente ha selezionato il progetto n. %d\n", projectID);
	strcat(buff,".txt");
	project = fopen(buff, "r");
	if (project == NULL) /* Se non si riesce ad aprire il file restituiamo un errore */
    	{
        	send(sd_client, "-", sizeof(char), 0); /* Informiamo il client che non si è potuto aprire il file del progetto da modif. */
		buff8=0;
		return;
    	}
	char strID[3];
	sprintf(strID,"%d",projectID); /* Trasformo l'ID progetto in una stringa per inviarla al Client */
	send(sd_client, strID, strlen(strID), 0); /* Inviamo al Client l'ID del progetto */
	printf("Inviato al Client identificativo del progetto : n. %s\n", strID);
	fclose(project);
	buff9=1;
	while(buff9==1)
	{
		writeradd();
	}
	int val=semctl(semaphore_list, projectID, GETVAL); /* Leggiamo il valore del semaforo */
	if(val==0)
	{
		printf("Semaforo sbloccato automaticamente causa morte prematura del figlio\n", strID);
		semctl(semaphore_list, projectID, SETVAL, 1); /* Impostiamo il semaforo ad 1, il Client probabile sia Crashato */
		exit(1);
	}
	buff8=0;
	buff7=0;
	return;
}

/* -----------------------------------------------------------WriterAdd--------------------------------------------------------------------- */
void writeradd()
{
	int i;
	i=0;
	printf("In attesa di ricevere il nome del capitolo della sezione da creare\n");
	i = recv(sd_client, buff2, MAX, 0); /* Riceviamo il titolo del capitolo della sezione titolo = buff2 */
	if(strcmp(buff2,"")==0)
	{
		buff9=0;
		return;
	}
	printf("Ricevuto il nome del capitolo della sezione da creare : %s\n", buff2);
	if (i == -1) /* Se il recv da errore, non siamo riusciti a ricevere dal client il titolo del capitolo */
    	{
        	send(sd_client, "0", sizeof(char), 0);
        	return;
    	}
	send(sd_client, "1", sizeof(char), 0);
	i=0;
	printf("In attesa di ricevere il nome dell'autore del capitolo\n");
	i = recv(sd_client, buff3, MAX, 0); /* Riceviamo l'autore del capitolo Autore = buff3 */
	if(strcmp(buff3,"")==0)
	{
		buff9=0;
		return;
	}
	printf("Ricevuto il nome del dell'autore : %s\n", buff3);
	if (i == -1) /* Se il recv da errore, non siamo riusciti a ricevere dal client l'autore del capitolo */
    	{
        	send(sd_client, "0", sizeof(char), 0);
        	return;
    	}
	send(sd_client, "1", sizeof(char), 0);
	i=0;
	printf("In attesa di ricevere il testo del capitolo\n");
	i = recv(sd_client, buff_tale, MAXF, 0); /* Riceviamo il testo del capitolo Testo = buff_tale */
	if(strcmp(buff_tale,"")==0)
	{
		buff9=0;
		return;
	}
	printf("Ricevuto il testo del capitolo\n");
	if (i == -1) /* Se il recv da errore, non siamo riusciti a ricevere dal client il testo */
    	{
        	send(sd_client, "0", sizeof(char), 0);
        	return;
    	}
	send(sd_client, "1", sizeof(char), 0); /* Avvisiamo il client che abbiamo ricevuto il testo. */
	sprintf(buff4, "Titolo Capitolo : %s\n", buff2);
	sprintf(buff5, "Autore : %s\n\n", buff3);
	project=fopen(buff, "a+");	
	fwrite(buff4, strlen(buff4), 1, project); /* Scriviamo "1" elemento di dimensione strlen(buff2) in project a partire da buff4 */
	fwrite(buff5, strlen(buff5), 1, project); /* Scriviamo "1" elemento di dimensione strlen(buff2) in project a partire da buff5 */
	fwrite(buff_tale, strlen(buff_tale), 1, project); /* Scriviamo "1" elemento di dimensione in project a partire da buff_tale */
	fwrite(buff_div, strlen(buff_div), 1, project); /* Aggiungiamo un divisore a fine capitolo */
	fclose(project);
	buff9=0;
	return;
}

/* -------------------------------------------------------------Reader---------------------------------------------------------------------- */
void reader()
{
	cleaner();
	initializeprojectlist();
	
/* Apriamo il file progetti in lettura per vedere se esiste, se non c'è, non ci sono progetti, se c'è, inviamo al client la lista */

	project = fopen("progetti.txt", "r"); /* Apriamo progetti.txt in lettura per prendere il nome dei progetti */	
	if (project == NULL) /* Se non si riesce ad aprire il file restituiamo un errore */
    	{
        	send(sd_client, "0", sizeof(char), 0); /* Informiamo il client che non si è potuto aprire la lista progetti */
		buff8=0;
        	return;
    	}
	send(sd_client, " \n", 2, 0);
	while((fgets(buff_tale, MAXF, project))!=NULL) /* Passiamo in rassegna il file progetti.txt */
	{
		send(sd_client, buff_tale, strlen(buff_tale), 0); /* Inviamo al client i nomi di tutti i progetti in progetti.txt */
	}
	send(sd_client, "1", 2, 0); /* Inviamo il check di fine scansione file */
	fclose(project);

/* Dopo inizializzato array con progetti, inizializziamo il semaforo */

printf("Sono stati letti ed inseriti %d progetti\n", project_list_size);


/* Riceviamo il nome del progetto da aprire, gli aggiungiamo.txt e cerchiamo di aprirlo, se ci riusciamo ok senno sped. errore  */
	
	printf("In attesa di ricevere il nome del progetto per l'apertura e lettura\n");
	recv(sd_client, buff2, MAX, 0); /* Riceviamo il nome del progetto che si vuole aprire per la modifica */
	printf("Ricevuto il nome del progetto per l'apertura e lettura : %s\n", buff2);
	strcat(buff2,".txt");
	bufffile = fopen(buff2, "r");
	if (bufffile == NULL) /* Se non si riesce ad aprire il file restituiamo un errore */
    	{
        	send(sd_client, "0", sizeof(char), 0); /* Informiamo il client che non si è potuto aprire il file richiesto per lettura */
        	return;
    	}
	send(sd_client, "1", sizeof(char), 0); /* Avvisiamo il client che abbiamo aperto il file che voleva. */
	send(sd_client, " \n", 2, 0);
	while((fgets(buff_tale2, MAXF, bufffile))!=NULL) /* Passiamo in rassegna il file che il client vuole leggere */
	{
		send(sd_client, buff_tale2, strlen(buff_tale2), 0);
	}	
	send(sd_client, "1", 2, 0); /* Inviamo il check di fine scansione file */
	fclose(bufffile);
	return;
}
