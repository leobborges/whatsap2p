/*
	Alex de Oliveira Venturini 	   15294739
	Leonardo Borges Bergamo		   15251275
	Luan Bonomi			   15108780
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

void exibirMenu(){
	// opcao do menu
	int opcaoMenu = 0;

	//Exibi o menu para o cliente
	do
	{
		printf("Menu: \n");
		printf("1) Enviar mensagem\n");
		printf("2) Adicionar Contato\n");
		printf("3) Visualizar Contatos\n");
		printf("4) Adicionar Grupo de Contatos\n");
		printf("5) Visualizar Grupo de Contatos\n");
		printf("6) Sair\n\n");
		printf("Digite a opcao escolhida:\n");
		__fpurge(stdin);
		scanf("%d", &opcaoMenu);
	} while (opcaoMenu != 6);
}

/* Cliente TCP */
int main(int argc, char *argv[])
{
    unsigned short port;
    struct hostent *hostnm;
    struct sockaddr_in server;
    int s;
    char recvbuf;
    char sendTo[16];
    
    // Numero de telefone do usuario
    char phoneNumber[15];

    /*
    * O primeiro argumento (argv[1]) � o hostname do servidor.
    * O segundo argumento (argv[2]) � a porta do servidor.
    * O terceiro argumento (argv[3]) � nome do cliente.
    */
    if (argc != 3)
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

    printf("Insira o seu numero de telefone para entrar no Whatsap2p:\n");
    __fpurge(stdin);
    fgets(phoneNumber, sizeof(phoneNumber), stdin);

	exibirMenu();

    /* Envia a mensagem no buffer de envio para o servidor */
    if (send(s, &phoneNumber, sizeof(phoneNumber), 0) < 0)
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



