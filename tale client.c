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

#define MAX 8000
#define MAXX 1024000
#define MAX_PROJECTS 250
 
char buff[MAX];
char buff2[MAX];
char buff3[MAX];
char buff4[MAX];
char buff5[MAX]; /* Utilizziamo buff5 per benvenuto nello usermenu, non lo azzeriamo in maniera da poterlo riusare */
char buff_rep[MAX];
char buff_tale[MAXX];
char buff_tale2[MAXX];
int semaphore_list; /* set semafori */
int buff6=1; /* Utilizziamo buff6 per uscire dal usermenu ma rimanere con il client connesso */
int buff7=1; /* Utilizziamo buff7 per uscire dal loop del writermenu ma rimanere nell'usermenu */
int buff8=1; /* Utilizziamo buff7 per uscire dal writermenu (cioe dal loop del writernew) e rimanere nel usermenu */
int buff9=1; /* Utilizziamo Buff9 per ripeter il addnew project in caso di invio errato */
int connection;

/* -----------------------------------------------------------Prototipi--------------------------------------------------------------------- */
int socket_conn();
void login();
void signin();
void header();
void complete();
void cleaner();
void menu();
void usermenu();
void writermenu();
void writernew();
void writerold();
void writeradd();
void reader();
void initializesemaphore();

/* --------------------------------------------------------Completa Stream------------------------------------------------------------------ */
void complete()
{
	int i = 0;
	for (i = (int)strlen(buff_rep); i == 8000; i++) /* Ogni carattere eccedente il nostro input viene rimpiazzato con \0 */
		buff_rep[i]='\0';
}

/* -------------------------------------------------------L'azzera buffers------------------------------------------------------------------ */
void cleaner()
{
	int i=0;
	for (i=0; i<MAX; i++) /* Ripulisce tutti i buffer, sovrascrivendo \0 a tutti i caratteri */
	{
		buff[i]='\0';
		buff2[i]='\0';
		buff_rep[i]='\0';
	}
	for (i=0; i<MAXX; i++)
	{
		buff_tale[i]='\0';
		buff_tale2[i]='\0';
	}
}

/* ----------------------------------------------------------Connessione-------------------------------------------------------------------- */
int socket_conn()
{
	int sd_client, result; /* Il socket descriptor del client */
	int i=0;
	struct sockaddr_in server_addr; /* l'indirizzo del server */
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(1745);

    /* Utilizzando la struttura hostent si definisce l'indirizzo del server */

	struct hostent *hp;
	hp = gethostbyname("127.0.0.1");

    /* successivamente viene memorizzato nella struttura server_addr */

	server_addr.sin_addr.s_addr = ((struct in_addr*)(hp->h_addr)) -> s_addr; /* un Cast, visto che apriamo una struttura non definita */
    
    /* Viene creato il socket descriptor */

	if((sd_client = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		printf("Errore nella creazione della socket\n");
    
    /* Viene connesso al server */

	printf("Connessione al server in corso...\n");
	printf("\n\n\nPremi invio per continuare\n");
	getchar();
	result = connect(sd_client, (struct sockaddr*) &server_addr, sizeof(server_addr));
	if (result == -1)
	{
		for (i=0; i==5; i++)
		{
			printf("Errore di connessione al server, tentativo %d di 5\n", i);
			printf("\n\n\nPremi invio per continuare\n");
			getchar();
		}
	printf("Connessione al server non riuscita!\n");
	exit(-1);
	}
	header();
	printf("Connessione al server riuscita!\n");
	printf("\n\n\nPremi invio per continuare\n");
	getchar();
	return sd_client;
}

/* ----------------------------------------------------------Semaphore---------------------------------------------------------------------- */
void initializesemaphore()
{
	semaphore_list=semget(ftok("/dev/null",1), MAX_PROJECTS, 0666);
	if (semaphore_list==-1)
	{
		printf("Il semaforo non è stato inizializzato dal server\n");
	}
}


/* -------------------------------------------------------------Main------------------------------------------------------------------------ */
int main()
{
	header();
	initializesemaphore();
	connection = socket_conn();
	while(1)
	{
		menu();
	}
	return 1;  
}

/* ------------------------------------------------------------Header----------------------------------------------------------------------- */
void header()
{
	system("clear");
	printf("**************** SHARED TALE V0.1 *******************\n");
	printf(".......perchè la fantasia può essere stimolata.......\n");
	printf("*****************************************************\n\n");
}


/* --------------------------------------------------------Menu Principale------------------------------------------------------------------ */
void menu()
{
	header();
	cleaner();
	printf("Menu Principale:\n1. Login\n2. Registrazione\n0. Esci\n");
	fgets(buff, 2, stdin); /* Legge max 2 caratteri da standard input (stdin) e li memorizza nell'array buff */
	switch(buff[0])
	{
		case '1': 
		{
			getchar();
            		login();
            		break;
        	}
		case '2': 
		{
			getchar();
            		signin();
            		break;
        	}
		case '0': 
		{
			getchar();
	           	header();
			send(connection, "0", sizeof(char), 0);
	            	printf("Grazie per aver contribuito alla diffusionne della fantasia! e di averlo fatto con SHARED TALE\n");
	            	printf("\n\n\nPremi invio per continuare\n");
			getchar();
	            	system("clear");
	            	exit(0);
	            	break;
	        }
		default: 
		{
            	    	header();
            		printf("Scelta non riconosciuta\n");
			printf("\n\n\nPremi invio per continuare\n");
			getchar();
			return;
        	}
	}
	return;
}


/* ---------------------------------------------------------Registrazione------------------------------------------------------------------- */
void signin()
{
	header();
	cleaner();
	send(connection, "2", sizeof(char), 0);
	printf("Modulo di registrazione a SHARED TALE\n\n");
	printf("Username (max 32 caratteri): \n");
	fgets(buff, 34, stdin); /* Legge size-1 (33) caratteri da standard input (stdin) e li memorizza nell'array buff, att. legge invio */
	buff[strlen(buff)-1] = '\0'; /* Inseriamo \0 a fine stringa, marcatore di fine stringa necessario, lo ssostituiamo al car. invio */
	strcat(buff, ":"); /* Aggiungiamo il carattere ":" alla stringa buff (per ora è solo username), concateniamo ":" a buff */
	printf("Password (max 8 caratteri): \n");
	fgets(buff2, 9, stdin); /* Legge size-1 (8) caratteri da standard input (stdin) e li memorizza nell'array buff2, la password */
	strcat(buff, buff2); /* concateniamo buff2 (password) a buff (username), strcat aggiunge già il carattere terminale /0 */
	strcpy(buff_rep, buff); /* copiamo buff (passw+usern) in buff_rep */
	complete();
	send(connection, buff_rep, MAX, 0);
	cleaner();
	printf ("Attendiamo la risposta alla richiesta di apertura database\n");
	recv(connection, buff, sizeof(char), 0); /* controlliamo se la registrazione è andata a buon fine */
	printf ("Ricevuto carattere di controllo apertura database : %c\n", *buff);
	if (buff[0]=='0')
	{
		system("echo \"\\nErrore registrazione\\n\"");
		printf("\n\n\nPremi invio per continuare\n");
		getchar();
		return;
	}
	cleaner();
	printf ("Attendiamo il controllo del server di username già esistente\n");
	recv(connection, buff, sizeof(char), 0); /* Controlliamo se l'username esiste già, riceviamo dal server il carattere 0 o 1 */
	printf ("Ricevuto carattere di controllo username esistente : %c\n", *buff);
	if (buff[0]=='0')
	{
		system("echo \"\\nUsername già esistente!\\n\"");
		printf("\n\n\nPremi invio per continuare\n");
		getchar();
		return;
	}	
	system("echo \"\\nRegistrazione Effettuata!\\n\"");
	printf("\n\n\nPremi invio per continuare\n");
	getchar();
	return;
}

/* -------------------------------------------------------------Login----------------------------------------------------------------------- */
void login()
{
	header();
	cleaner();
	send(connection, "1", sizeof(char), 0);
	printf("Login\n");
	printf("Username (max 32 caratteri): \n");
	fgets(buff, 34, stdin);
	buff[strlen(buff)-1] = '\0';
	strcpy(buff5, buff);
	strcat(buff, ":");
	printf("Password (max 8 caratteri): \n");
	fgets(buff2, 9, stdin);
	strcat(buff, buff2);
	strcpy(buff_rep, buff);
	complete();
	send(connection, buff_rep, MAX, 0);
	printf ("Attendiamo la risposta alla richiesta di apertura database\n");
	recv(connection, buff3, sizeof(char), 0);
	printf ("Ricevuto carattere di controllo apertura database : %c\n", *buff3);
	if (buff3[0]=='0')
    	{
        	system("echo \"\\nErrore Login\\n\"");
        	printf("\n\n\nPremi invio per continuare\n");
		getchar();
        	return;
    	}
	printf ("Attendiamo il controllo del server di username o password errate\n");
	recv(connection, buff4, sizeof(char), 0);
	printf ("Ricevuto carattere di controllo username o password errate : %c\n", *buff4);
	if (buff4[0]=='0')     
    	{
        	system("echo \"\\nPassword o Username errate\\n\"");
        	printf("\n\n\nPremi invio per continuare\n");
		getchar();
        	return;
    	}
	system("echo \"\\nLogin Effettuato!\\n\"");
	printf("\n\n\nPremi invio per continuare\n");
	getchar();
	buff6=1; /* Questa variabile flag serve per uscire dal prossimo ciclo */
	while(buff6==1)
	{
    		usermenu();
	}	
	return;
}

/* ------------------------------------------------------Usermenu dopo Login---------------------------------------------------------------- */
void usermenu()
{ 
	cleaner();
	header();
	printf("Bentornato %s, quale ruolo interpreterai oggi?\n", buff5);
	printf("\n1. Lettore\n2. Scrittore\n3. Logout\n");
	fgets(buff, 2, stdin);
	switch(buff[0])
	{
		case '1':
        	{
			getchar();
            		send(connection, "1", sizeof(char), 0);
            		reader();
            		break;
        	}
		case '2':
        	{
			getchar();
            		send(connection, "2", sizeof(char), 0);
			buff7=1;
			while(buff7==1)
			{
            			writermenu();
			}
			break;
        	}
		case '3':
        	{
			getchar();
            		send(connection, "4", sizeof(char), 0);
			buff6=0;
            		return;
			break;
        	}
		default :
		{
			header();
			printf("Scelta non riconosciuta\n");
			printf("\n\n\nPremi invio per continuare\n");
			getchar();
			break;
		}
	}
	return;
}

/* -----------------------------------------------------------WriterMenu-------------------------------------------------------------------- */
void writermenu()
{
	header();
	cleaner();
	printf("Benvenuto %s, su che progetto hai intenzione di lavorare?\n\n", buff5);
	printf("1. Nuovo progetto\n2. Progetto già esistente\n3. Indietro\n");
	fgets(buff, 2, stdin);
	switch(buff[0])
	{
		case '1':
        	{
			getchar();
            		send(connection, "1", sizeof(char), 0);
			buff8=1;
			while(buff8==1)
			{
            			writernew();
			}
            		break;
        	}
		case '2':
        	{
			getchar();
            		send(connection, "2", sizeof(char), 0);
            		buff8=1;
			while(buff8==1)
			{
            			writerold();
			}
			break;
        	}
		case '3':
        	{
			getchar();
            		send(connection, "3", sizeof(char), 0);
			buff7=0;
            		return;
			break;
        	}
		default : 
		{
			printf("Scelta non riconosciuta!\n");
			break;
		}
	}
	return;
}

/* -----------------------------------------------------------WriterNew-------------------------------------------------------------------- */
void writernew()
{
	header();
	cleaner();
	printf("Inserire il nome del nuovo progetto (32 caratteri max) :\n");
	fgets(buff, 34, stdin);
	buff[strlen(buff)-1] = '\0';
	send(connection, buff, MAX, 0);
	printf ("Attendiamo la risposta al controllo progetto esistente\n");
	recv(connection, buff2, sizeof(char), 0);
	printf ("Ricevuto carattere di controllo progetto esistente : %c\n", *buff2);
	if (buff2[0]=='0')
	{
		printf("Il nome scelto appartiene ad un progetto già esistente, scegliere un nome differente\n");
		printf("\n\n\nPremi invio per continuare\n");
		getchar();
		return;
	}
	printf ("Attendiamo la risposta alla richiesta di creazione file\n");
	recv(connection, buff3, sizeof(char), 0);
	printf ("Ricevuto carattere di controllo creazione file : %c\n", *buff3);
	if (buff3[0]=='0')
	{
		printf("Errore nella creazione del progetto\n");
		printf("\n\n\nPremi invio per continuare\n");
		getchar();
		return;
	}
	printf("Progetto << %s >> creato!\n", buff);
	writeradd();
	buff8=0; /* Per uscire dal loop che mi riporta nel Writernew */
	buff7=0; /* Per uscire dal loop che mi riporta nel WriterMenu */
	return;
}

/* -----------------------------------------------------------WriterOld-------------------------------------------------------------------- */
void writerold()
{
	header();
	cleaner();
	printf ("Attendiamo la risposta alla richiesta di controllo database progetti\n");
	recv(connection, buff, sizeof(char), 0);
	printf ("Ricevuto carattere di controllo database progetti : %c\n", *buff);
	if (buff[0]=='0')
	{
		printf("Il database non contiene nessun progetto modificabile, creane uno nuovo\n");
		printf("\n\n\nPremi invio per continuare\n");
		getchar();
		buff8=0;
		return;
	}
	header();
	printf("In quale progetto vuoi contribuire con la tua fantasia?\n");	
	while(recv(connection, buff_tale, MAXX, 0))
	{
		if (buff_tale[strlen(buff_tale)-1] == '1')
		{
			int i=0;
			for(i=0; i<strlen(buff_tale)-1; i++)
			printf("%c", buff_tale[i]);
		}
		else
		{
			printf("%s", buff_tale);
		}
		if (buff_tale[strlen(buff_tale)-1] == '1')
		{
			break;
		}
	}
	printf("\nNome progetto : ");
	fgets(buff, 34, stdin);
	buff[strlen(buff)-1] = '\0';
	send(connection, buff, MAX, 0);
	printf ("Attendiamo la risposta alla richiesta di apertura progetto da modificare\n");
	recv(connection, buff, MAX, 0); /* Riceviamo l'ID del progetto da modificare oppure "-" */
	if (buff[0]=='-')
	{
		printf("Impossibile aprire il progetto scelto\n");
		printf("\n\n\nPremi invio per continuare\n");
		getchar();
		buff8=0;
		return;
	}
	int projectID = atoi(buff); /* Ritrasformiamo l'ID in un intero */
	struct sembuf actions[1]; /* Creiamo la struttura actions per le azioni sui semafori (struct "sembuf"), con un azione */
	actions[0].sem_num=projectID; /* Assegnamo al numero semaforo l'ID del progetto */
	actions[0].sem_flg=IPC_NOWAIT; /* Diciamo che non deve attendere */
	actions[0].sem_op=-1; /* Come operazione gli assegnamo di fare -1 */
	int op=semop(semaphore_list, actions, 1); /* Lanciamo l'azione */
	if (op==-1) /* Se il semaforo arriva a -1 significa che c'è già una persona dentro a modificare il progetto */
	{
		header();
		printf("Qualcun'altro sta già modificando questo progetto\n");
		printf("\n\n\nPremi invio per continuare\n");
		getchar();
		buff7=0;
		buff8=0;
		return;
	}
	buff9=1;
	while(buff9==1)
	{
		writeradd();
	}
	actions[0].sem_op=1; /* Assegnamo l'operazione di settare il semaforo ad 1 */
	op=semop(semaphore_list, actions, 1); /* Lanciamo l'operazione di settaggio semaforo ad 1 */
	buff8=0; /* Per uscire dal loop che mi riporta nel Writernew */
	buff7=0; /* Per uscire dal loop che mi riporta nel WriterMenu */
	return;
}

/* -----------------------------------------------------------WriterAdd--------------------------------------------------------------------- */
void writeradd()
{
	header();
	printf("Benvenuto scrittore, qui puoi aggiungere un nuovo capitolo\n");
	printf("Titolo capitolo (max 32 caratteri) : ");
	fgets(buff, 34, stdin);
	buff[strlen(buff)-1] = '\0';
	send(connection, buff, MAX, 0);
	printf ("Attendiamo conferma di ricezione Titolo\n");
	recv(connection, buff2, sizeof(char), 0);
	printf ("Ricevuto carattere di conferma ricezione Titolo : %c\n", *buff2);
	if (buff2[0]=='0')
	{
		printf("Problemi di comunicazione con il server, prova a inviarlo di nuovo\n");
		printf("\n\n\nPremi invio per continuare\n");
		getchar();
		return;
	}
	buff5[strlen(buff5)+1] = '\0';
	send(connection, buff5, MAX, 0);
	printf ("Attendiamo conferma di ricezione Autore\n");
	recv(connection, buff2, sizeof(char), 0);
	printf ("Ricevuto carattere di conferma ricezione Autore : %c\n", *buff2);
	if (buff2[0]=='0')
	{
		printf("Problemi di comunicazione con il server, impossibile determinare l'autore del capitolo, riprova\n");
		printf("\n\n\nPremi invio per continuare\n");
		getchar();
		return;
	}
	header();
	printf("Autore : %s\n", buff5);
	printf("Titolo : %s\n\n", buff);
	printf("Testo (massimo 1024000 caratteri, premi invio quando finito) :\n");
	fgets(buff_tale, MAXX, stdin);
	buff_tale[strlen(buff_tale)+1] = '\0';
	send(connection, buff_tale, strlen(buff_tale)+1, 0);
	printf ("Attendiamo conferma di ricezione Testo\n");
	recv(connection, buff2, sizeof(char), 0);
	printf ("Ricevuto carattere di conferma ricezione Testo : %c\n", *buff2);
	if (buff2[0]=='0')
	{
		printf("Problemi di comunicazione con il server, riprova\n");
		printf("\n\n\nPremi invio per continuare\n");
		getchar();
		return;
	}	
	buff9=0;
	return;
}

/* -------------------------------------------------------------Reader---------------------------------------------------------------------- */
void reader()
{
	header();
	cleaner();
	printf ("Attendiamo la risposta alla richiesta di controllo database progetti\n");
	recv(connection, buff, sizeof(char), 0);
	printf ("Ricevuto carattere di controllo database progetti : %c\n", *buff);
	if (buff[0]=='0')
	{
		printf("Il database non contiene nessun progetto da leggere, creane uno nuovo\n");
		printf("\n\n\nPremi invio per continuare\n");
		getchar();
		return;
	}
	header();
	printf("Ciao lettore, quale progetto vuoi visionare?\n");
	while(recv(connection, buff_tale, MAXX, 0))
	{
		if (buff_tale[strlen(buff_tale)-1] == '1')
		{
			int i=0;
			for(i=0; i<strlen(buff_tale)-1; i++)
			printf("%c", buff_tale[i]);
		}
		else
		{
			printf("%s", buff_tale);
		}
		if (buff_tale[strlen(buff_tale)-1] == '1')
		{
			break;
		}
	}
	printf("\nNome progetto : ");
	fgets(buff2, 34, stdin);
	buff2[strlen(buff2)-1] = '\0';
	send(connection, buff2, strlen(buff2), 0);
	printf ("Attendiamo la risposta alla richiesta di apertura progetto da leggere\n");
	recv(connection, buff3, sizeof(char), 0);
	printf ("Ricevuto carattere di controllo di apertura progetto da leggere : %c\n", *buff3);
	if (buff3[0]=='0')
	{
		printf("Impossibile aprire il progetto scelto\n");
		printf("\n\n\nPremi invio per continuare\n");
		getchar();
		return;
	}
	header();
	while(recv(connection, buff_tale2, MAXX, 0))
	{
		if (buff_tale2[strlen(buff_tale2)-1] == '1')
		{
			int i=0;
			for(i=0; i<strlen(buff_tale2)-1; i++)
			printf("%c", buff_tale2[i]);
		}
		else
		{
			printf("%s", buff_tale2);
		}
		if (buff_tale2[strlen(buff_tale2)-1] == '1')
		{
			break;
		}
	}
	printf("\n\n\nPremi invio per continuare\n");
	getchar();
	return;
}
