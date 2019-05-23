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


struct location
{
	char ipAddress[16];
	int port;
};

struct serverData
{
	char userPhone[15];
	struct location userLocation;
};

struct serverData serverD[3];


/* Semáforo Mutex */
sem_t mutex;

void *funcThread(void *nsClient);
int searchPhoneNumber(char phoneNumber[]);

/* Servidor TCP */
int main(int argc, char *argv[])
{
	unsigned short port;

	struct sockaddr_in server;
	int s;  /* Socket para aceitar conex�es       */
	int ns; /* Socket conectado ao cliente        */
	int namelen, response;
	pthread_t thread_id;

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

	for (int i = 0; i < 3; i++)
	{
		strcpy(serverD[i].userPhone, "");
		strcpy(serverD[i].userLocation.ipAddress, "");
	}

	/* Inicia o semáforo mutex */
	sem_init(&mutex, 0, 1);

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

	printf("Servidor terminou com sucesso.\n");
	exit(0);
}

int searchPhoneNumber(char phoneNumber[]) {
	for(int sizeOfServer = 0; sizeOfServer < 3; sizeOfServer++) 
	{
		if(strcmp(serverD[sizeOfServer].userPhone, phoneNumber) == 0) 
		{
			printf("Encontrado telefone igual\n");
			return 1;
		}
		
	}

	return 0;
}

void *funcThread(void *nsClient)
{
	int ns = *(int *)nsClient;
	
	char sendbuf;
	char clientData[16];

	do
	{
		/* Recebe uma mensagem do cliente atrav�s do novo socket conectado */
		if (recv(ns, &clientData, sizeof(clientData), 0) == -1)
		{
			perror("Recv()");
			exit(6);
		}

		sem_wait(&mutex);

		int search = searchPhoneNumber(clientData);
		if(search == 0)
		{
			for(int sizeOfServer = 0; sizeOfServer < 3; sizeOfServer++) 
			{
				if(strcmp(serverD[sizeOfServer].userPhone, "") == 0)
				{
					strcpy(serverD[sizeOfServer].userPhone, clientData);
					strcpy(serverD[sizeOfServer].userLocation.ipAddress, inet_ntoa(client.sin_addr));
					serverD[sizeOfServer].userLocation.port = ntohs(client.sin_port);

					sendbuf = 'S';
					break;
				}
			}
		}
		else
		{
			sendbuf = 'N';
		}

		sem_post(&mutex);

		/* Envia uma mensagem ao cliente atrav�s do socket conectado */
		if (send(ns, &sendbuf, sizeof(sendbuf), 0) < 0)
		{
			perror("Send()");
			exit(7);
		}
	} while(sendbuf != 'S');

	/* Fecha o socket conectado ao cliente */
	close(ns);
}
