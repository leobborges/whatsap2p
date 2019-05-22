/*
Leonardo Borges Bergamo 15251275
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

struct sockaddr_in client;

void *funcThread(void *nsClient);

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

void *funcThread(void *nsClient)
{
	int ns = *(int *)nsClient;
	
	char sendbuf;
	char address[16];
	char clientData[16];

	strcpy(address,inet_ntoa(client.sin_addr));

	/* Recebe uma mensagem do cliente atrav�s do novo socket conectado */
	if (recv(ns, &clientData, sizeof(clientData), 0) == -1)
	{
		perror("Recv()");
		exit(6);
	}

	printf("\nClient IP Address: %s", address);
	printf("\nPhone number: %s\n", clientData);
	sendbuf = 'S';

	/* Envia uma mensagem ao cliente atrav�s do socket conectado */
	if (send(ns, &sendbuf, sizeof(sendbuf), 0) < 0)
	{
		perror("Send()");
		exit(7);
	}

	/* Fecha o socket conectado ao cliente */
	close(ns);
}
