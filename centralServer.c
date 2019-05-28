/*
	Alex de Oliveira Venturini 	   15294739
	Leonardo Borges Bergamo		   15251275
	Luan Bonomi			   15108780
	Matheus Augusto Cremonez Guimarães 15004336
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <semaphore.h>

struct sockaddr_in client;

struct rcvClientData {
	char phoneNumber[15];
	int option;
};

struct location
{
	char ipAddress[16];
	int port;
};

struct serverData
{
	char userPhone[15];
	struct location userLocation;
	struct serverData *prox;
};

typedef struct serverData data;

data *users;

/* Semáforo Mutex */
sem_t mutex;
sem_t mutex2;

void *funcThread(void *nsClient);
int searchPhoneNumber(char phoneNumber[]);
void insertUser(char phoneNumber[], char ip[], int port);
void searchAndRemoveUser(char phoneNumber[]);

/* Servidor TCP */
int main(int argc, char *argv[])
{
	unsigned short port;

	struct sockaddr_in server;
	int s;  /* Socket para aceitar conex�es       */
	int ns; /* Socket conectado ao cliente        */
	int namelen, response;
	pthread_t thread_id;

	users = (data *) malloc(sizeof(data));
	if(!users){
		printf("Sem memoria disponivel!\n");
		exit(1);
	}
	users->prox = NULL;

	/*
        * O primeiro argumento (argv[1]) � a porta
        * onde o servidor aguardar� por conex�es
        */
	if (argc != 2)
	{
		fprintf(stderr, "Use: %s porta\n", argv[0]);
		exit(1);
	}

	port = (unsigned short)atoi(argv[1]);

	/* Cria um socket TCP (stream) para aguardar conex�es */
	if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("Socket()");
		exit(2);
	}

	/*
        * Define a qual endere�o IP e porta o servidor estar� ligado.
        * IP = INADDDR_ANY -> faz com que o servidor se ligue em todos
        * os endere�os IP
        */
	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	server.sin_addr.s_addr = INADDR_ANY;

	/* Liga o servidor � porta definida anteriormente. */
	if (bind(s, (struct sockaddr *)&server, sizeof(server)) < 0)
	{
		perror("Bind()");
		exit(3);
	}

	/*
     	* Prepara o socket para aguardar por conex�es e
     	* cria uma fila de conex�es pendentes.
     	*/
	if (listen(s, 1) != 0)
	{
		perror("Listen()");
		exit(4);
	}

	/* Inicia o semáforo mutex */
	sem_init(&mutex, 0, 1);
	sem_init(&mutex2, 0, 1);

	while(1){
		/*
	     	* Aceita uma conex�o e cria um novo socket atrav�s do qual
	     	* ocorrer� a comunica��o com o cliente.
	     	*/
		namelen = sizeof(client);
		if ((ns = accept(s, (struct sockaddr *)&client, &namelen)) == -1)
		{
			perror("Accept()");
			exit(5);
		}

		printf("Client IP Address: %s - Port: %d - Connected\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port));

		response = pthread_create(&thread_id, NULL, &funcThread, &ns);

		if (response)
		{
			printf("ERROR; return code from pthread_create() is %d\n", response);
			exit(-1);
		}

	}

	/* Fecha o socket aguardando por conex�es */
	close(s);

	/* Encerra o semáforo */
	sem_destroy(&mutex);
	sem_destroy(&mutex2);

	printf("Servidor terminou com sucesso.\n");
	exit(0);
}

int searchPhoneNumber(char phoneNumber[]) {
	data *tmp = users;
	while(tmp != NULL){
		if(tmp->userPhone != NULL && strcmp(tmp->userPhone, phoneNumber) == 0) 
		{
			printf("Encontrado telefone igual\n");
			return 1;
		}
		tmp = tmp->prox;
	}

	return 0;
}

void insertUser(char phoneNumber[], char ip[], int port) {
	data *newUser=(data *) malloc(sizeof(data));
	data *oldHead = users->prox;

	strcpy(newUser->userPhone, phoneNumber);
	strcpy(newUser->userLocation.ipAddress, ip);
	newUser->userLocation.port = port;
	users->prox = newUser;
	newUser->prox = oldHead;
}

void searchAndRemoveUser(char phoneNumber[]) {
	data *user, *nextUser;
	user = users;
	nextUser = users->prox;
	while (nextUser != NULL && strcmp(nextUser->userPhone,phoneNumber) != 0) {
		user = nextUser;
		nextUser = nextUser->prox;
	}
	if (nextUser != NULL) {
		user->prox = nextUser->prox;
		printf("Cliente de Endereço de IP: %s - Porta: %d - Telefone: %s -  Saindo...\n", nextUser->userLocation.ipAddress, nextUser->userLocation.port, nextUser->userPhone);
		free(nextUser);
	}
}

void *funcThread(void *nsClient)
{
	int ns = *(int *)nsClient;
	
	char sendbuf;
	struct rcvClientData clientData;
	char ip[16];
	strcpy(ip, inet_ntoa(client.sin_addr));

	int port = ntohs(client.sin_port);
	

	do
	{
		/* Recebe uma mensagem do cliente atrav�s do novo socket conectado */
		if (recv(ns, &clientData, sizeof(clientData), 0) == -1)
		{
			perror("Recv()");
			exit(6);
		}

		if (clientData.option == 0) {
			sem_wait(&mutex);

			int search = searchPhoneNumber(clientData.phoneNumber);
			if(search == 0)
			{
				
				insertUser(clientData.phoneNumber, ip, port);
				sendbuf = 'S';
			}
			else
			{
				sendbuf = 'N';
			}

			sem_post(&mutex);
		}
		else if (clientData.option == 6) {
			sem_wait(&mutex);
			searchAndRemoveUser(clientData.phoneNumber);
			sem_post(&mutex);
			sendbuf = 'Q';
		}

		/* Envia uma mensagem ao cliente atrav�s do socket conectado */
		if (send(ns, &sendbuf, sizeof(sendbuf), 0) < 0)
		{
			perror("Send()");
			exit(7);
		}
	} while(sendbuf != 'Q');

	/* Fecha o socket conectado ao cliente */
	close(ns);
}
