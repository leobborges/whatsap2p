/*
Leonardo Borges Bergamo 15251275
Matheus Augusto Cremonez Guimarães 15004336
*/

#include <stdio.h>
#include <stdio_ext.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* Cliente TCP */
int main(int argc, char *argv[])
{
	unsigned short port;
	struct hostent *hostnm;
	struct sockaddr_in server;
	int s;
	char recvbuf;
	char sendTo[16];

	/*
     * O primeiro argumento (argv[1]) � o hostname do servidor.
     * O segundo argumento (argv[2]) � a porta do servidor.
     * O terceiro argumento (argv[3]) � nome do cliente.
     */
	if (argc != 4)
	{
		fprintf(stderr, "Use: %s hostname porta\n", argv[0]);
		exit(1);
	}

	/* Obtendo o endere�o IP do servidor */
	hostnm = gethostbyname(argv[1]);
	if (hostnm == (struct hostent *)0)
	{
		fprintf(stderr, "Gethostbyname failed\n");
		exit(2);
	}
	port = (unsigned short)atoi(argv[2]);

	/* Define o endere�o IP e a porta do servidor */
	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	server.sin_addr.s_addr = *((unsigned long *)hostnm->h_addr);

	/* Cria um socket TCP (stream) */
	if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("Socket()");
		exit(3);
	}

	/* Estabelece conex�o com o servidor */
	if (connect(s, (struct sockaddr *)&server, sizeof(server)) < 0)
	{
		perror("Connect()");
		exit(4);
	}

	strcpy(sendTo, argv[3]);

	/* Envia a mensagem no buffer de envio para o servidor */
	if (send(s, &sendTo, sizeof(sendTo), 0) < 0)
	{
		perror("Send()");
		exit(5);
	}

	/* Recebe a mensagem do servidor no buffer de recep��o */
	if (recv(s, &recvbuf, sizeof(recvbuf), 0) < 0)
	{
		perror("Recv()");
		exit(6);
	}

	/* Fecha o socket */
	close(s);

	printf("Cliente terminou com sucesso.\n");
	exit(0);
}


