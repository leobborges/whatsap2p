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

struct sendServerData {
	char phoneNumber[15];
	int option;
};

struct rcvServerData {
	char ip[15];
	int port;
};

/* Variáveis Globais */
struct sendServerData data;
/* Numero de telefone do usuario */
char phoneNumber[15];

/* Protótipos das funções */
void addContact();
void addContactGroup();
struct rcvServerData getUserInfo(int s, char phoneNumber[]);
void showContacts();
void showGroupContacts();
void showMenu(int s);
void showMessageMenu(int s);

/* Cliente TCP */
int main(int argc, char *argv[])
{
	unsigned short port;
	struct hostent *hostnm;
	struct sockaddr_in server;
	int s;
	char recvbuf;

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
		strtok(phoneNumber, "\n");

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

	showMenu(s);

	/* Fecha o socket */
	close(s);

	printf("Cliente terminou com sucesso.\n");
	exit(0);
}

void showMenu(int s) {
	int option = 0;
	do
	{
		printf("Menu: \n");
		printf("1) Enviar mensagem\n");
		printf("2) Adicionar Contato\n");
		printf("3) Visualizar Contatos\n");
		printf("4) Adicionar Grupo de Contatos\n");
		printf("5) Visualizar Grupos de Contatos\n");
		printf("6) Sair\n\n");
		printf("Digite a opcao escolhida:\n");

		__fpurge(stdin);
		scanf("%d", &option);

		if(option == 1) {
			showMessageMenu(s);
		}
		else if(option == 2) {
			addContact();
		}
		else if(option == 3) {
			showContacts();
		}
		else if(option == 4) {
			addContactGroup();
		}
		else if(option == 5) {
			showGroupContacts();
		}
		else if(option == 6) {
			char rcvMessage;
			data.option = option;
			strcpy(data.phoneNumber, phoneNumber);
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

void addContact() {
	char contact[20];
	char str[20];
	char phoneNumberContact[15];
	char fileContent[1000];
	char filename[30];
	FILE * file;
	int flag;

	strcpy(filename, "");
	strcat(filename, phoneNumber);
	strcat(filename, "-contatos.txt");

	do{
		flag = 0;
		printf("Digite o nome do contato:\n");
		__fpurge(stdin);

		fgets(contact, sizeof(contact), stdin);

		file = fopen(filename,"r");
		if (file == NULL){
			break;
		}
		else {
			while (fgets(str, 100, file) != NULL){
				if(strcmp(str, contact) == 0){
					flag = 1;
					printf("\nContato já existente, digite outro nome..\n\n");
				}
			}
		}
		fclose(file);

	}while(flag == 1);

	printf("Digite o telefone do contato:\n");
	__fpurge(stdin);

	fgets(phoneNumberContact, sizeof(phoneNumberContact), stdin);

	file = fopen(filename, "a");

	fprintf(file, "%s", contact);
	fprintf(file, "%s", phoneNumberContact);

	fclose(file);

	printf("\nContato cadastrado!\n\n");
}

void addContactGroup(){
	FILE * arquivo;
	char numero[15];
	char nomeGrupo[50];
	char sair = 's';
	char leitura[100];
	int comparer = 1;

	printf("Digite o nome do grupo:\n");	
	__fpurge(stdin);
	fgets(nomeGrupo, sizeof(nomeGrupo), stdin);

	if(strlen(nomeGrupo) != 1){
		arquivo = fopen("grupos.txt", "a");
		fprintf(arquivo, "%s", nomeGrupo);
	}else{
		printf("O nome do grupo nao pode ser nulo\n");
		exit(7);
	}
	
	while(sair =='s'){
		strcpy(numero, "");
		printf("Digite o numero que deseja adicionar:\n");	
		__fpurge(stdin);
		fgets(numero, sizeof(numero), stdin);

		if(strlen(numero) != 1){
			fprintf(arquivo, "%s", numero);
		}else{
			printf("O contato nao pode ser nulo\n");
			exit(7);
		}

		printf("\nDeseja inserir outro numero? (S/N)\n");
		scanf("%c", &sair);

	} 
	fputc('\n', arquivo);
	fclose(arquivo);
}

void showContacts() {
	FILE *fp;
	char str[100];
	char filename[30];
	int organizer = 0;
			
	strcpy(filename, "");
	strcat(filename, phoneNumber);
	strcat(filename, "-contatos.txt");
 
	fp = fopen(filename, "r");
	if (fp == NULL){
		printf("\nNão existem contatos cadatrados.\n\n");
	}
	else{
		printf("Lista de Contatos:\n\n");

		while (fgets(str, sizeof(str), fp) != NULL){
					
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
			
}

void showGroupContacts(){
	FILE *fp;
	char str[100];
	char* filename = "grupos.txt";
	int organizer = 0;
			 
	fp = fopen(filename, "r");
	if (fp == NULL){
		printf("Could not open file %s",filename);
		exit(7);
	}

	if (fgets(str, 100, fp) == NULL){
		printf("Nao ha grupos de contatos salvos.\n\n");
	}else{
		printf("Lista de Contatos:\n\n");
		
		while (fgets(str, 100, fp) != NULL){
					
			if(organizer == 0){
				printf("Nome do grupo: %s", str);
				organizer=1;
			}
			else if(strlen(str) != 1){
				printf("Telefone %i: %s",organizer, str);
				organizer++;
			}
			else{
				printf("\n");
				organizer = 0;
			}
		}
	}
	fclose(fp);
}

void showMessageMenu(int s) {
	int messageOption = 0;
			
	do {
		system("clear");
		printf("1) Enviar mensagem individualmente.\n");
		printf("2) Enviar mensagem para grupo.\n");
		printf("3) Voltar\n");
		printf("Digite a opcao desejada para o envio de mensagem: \n");

		__fpurge(stdin);
		scanf("%d", &messageOption);
	} while (messageOption < 1 && messageOption > 3);

	if (messageOption == 1) {
		char contact[20];
		printf("Para quem deseja enviar a mensagem?\n");
		__fpurge(stdin);
		fgets(contact, sizeof(contact), stdin);
		strtok(contact, "\n");
		

		// [WHATS-012] função que buscaria o "contact" no arquivo de contatos e retornaria o telefone
		struct rcvServerData userInfo;
		userInfo = getUserInfo(s, contact);
		 
	}
	else if (messageOption == 2) {
		char group[20];
		printf("Para que grupo deseja enviar a mensagem?\n");
		__fpurge(stdin);
		fgets(group, sizeof(group), stdin);
		strtok(group, "\n");

		// [WHATS-013] função que buscaria o "group" no arquivo de grupos e retornaria o todos os telefones e/ou contatos
		// chamaria a função criada na tarefa [WHATS-012], para retornar os telefones daqueles que são contatos
		// entraria em um loop que solicita as informações no servidor, recebe e envia a mensagem, até que a lista de telefones acabe.
	}
}

struct rcvServerData getUserInfo(int s, char phoneNumber[]) {
	struct rcvServerData rcvData;
	data.option = 1;
	strcpy(data.phoneNumber, phoneNumber);
	/* Envia a mensagem para o servidor para remover */
	if (send(s, &data, sizeof(data), 0) < 0)
	{
		perror("Send()");
		exit(5);
	}
	printf("Buscando dados no servidor...\n");
	/* Recebe a mensagem do servidor no buffer de recep��o */
	if (recv(s, &rcvData, sizeof(rcvData), 0) < 0)
	{
		perror("Recv()");
		exit(6);
	}
	
	if(rcvData.port > 0) {
		/* Print auxiliar enquanto a função de enviar a mensagem não for desenvolvida */
		printf("\nUsuário online:\nTelefone: %s\nEndereço de IP: %s\nPorta: %d\n\n", data.phoneNumber, rcvData.ip, rcvData.port);
	} 
	else {
		printf("A mensagem não pode ser enviada. O usuário está offline.\n");
	}
}
