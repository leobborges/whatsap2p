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

void exibirMenu(int s);
void adicionarContato();

struct sendServerData {
	char phoneNumber[15];
	int option;
};

struct sendServerData data;

/* Cliente TCP */
int main(int argc, char *argv[])
{
	unsigned short port;
	struct hostent *hostnm;
	struct sockaddr_in server;
	int s;
	char recvbuf;

	// Numero de telefone do usuario
	char phoneNumber[15];

	/*
	* O primeiro argumento (argv[1]) � o hostname do servidor.
	* O segundo argumento (argv[2]) � a porta do servidor.
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

	while(recvbuf != 'S')
	{
		printf("Insira o seu numero de telefone para entrar no Whatsap2p:\n");
		__fpurge(stdin);
		fgets(phoneNumber, sizeof(phoneNumber), stdin);

		strcpy(data.phoneNumber, phoneNumber);
		data.option = 0;

		/* Envia a mensagem no buffer de envio para o servidor */
		if (send(s, &data, sizeof(data), 0) < 0)
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

	}

	exibirMenu(s);

	/* Fecha o socket */
	close(s);

	printf("Cliente terminou com sucesso.\n");
	exit(0);
}

void exibirMenu(int s) {
	int option = 0;
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
		scanf("%d", &option);

		if(option == 1) {

		}
		else if(option == 2) {
			adicionarContato();
		}
		else if(option == 3) {
			visualizarContatos();
		}
		else if(option == 4) {

		}
		else if(option == 5) {

		}
		else if(option == 6) {
			char rcvMessage;
			data.option = option;
			/* Envia a mensagem para o servidor para remover */
			if (send(s, &data, sizeof(data), 0) < 0)
			{
				perror("Send()");
				exit(5);
			}
			printf("Saindo do WhatsAp2p...\n");
			/* Recebe a mensagem do servidor no buffer de recep��o */
			if (recv(s, &rcvMessage, sizeof(rcvMessage), 0) < 0)
			{
				perror("Recv()");
				exit(6);
			}
			break;
		}
	} while (option != 6);
}

void adicionarContato() {
	char contact[20];
	char phoneNumberContact[15];
	char fileContent[1000];
	FILE * file;


	printf("Digite o nome do contato:\n");
	__fpurge(stdin);
	fgets(contact, sizeof(contact), stdin);

	printf("Digite o telefone do contato:\n");
	__fpurge(stdin);
	fgets(phoneNumberContact, sizeof(phoneNumberContact), stdin);


	if ((file = fopen("contatos.txt","r")) == NULL){
		//Criando o arquivo
		file = fopen ("contatos.txt","w");
   	} else {
		file = fopen ("contatos.txt","a");
		fscanf(file,"%s", &fileContent);
	}

	fprintf(file, "%s", contact);
	fprintf(file, "%s", phoneNumberContact);

	fclose(file);

	printf("\nContato cadastrado!\n\n");
}

void visualizarContatos(){
	FILE *fp;
	char str[100];
	char* filename = "contatos.txt";
	int organizer = 0;
			 
	fp = fopen(filename, "r");
	if (fp == NULL){
		printf("Could not open file %s",filename);
		exit(7);
	}

	printf("Lista de Contatos:\n\n");

	while (fgets(str, 100, fp) != NULL){
				
		if(organizer == 0){
			printf("Nome: %s", str);
			organizer++;
		}
		else if(organizer == 1){
			printf("Telefone: %s\n\n", str);
			organizer = 0;
		}
	}
			
	fclose(fp);
}
