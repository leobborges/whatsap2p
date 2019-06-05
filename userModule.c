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
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

struct messageData {
	char senderPhoneNumber[15];
	char message[144];
};

struct sendServerData {
	char phoneNumber[15];
	int option;
	unsigned short userPort;
};

struct rcvServerData {
	struct in_addr ip;
	unsigned short port;
    int status;
};

/* Variáveis Globais */
struct sendServerData data;
char phoneNumber[15];
struct sockaddr_in p2pClient;
struct sockaddr_in p2pServer;
struct messageData message;
int p2ps;
int userSocket;
int userSocketPort;

void addContact();
void addContactGroup();
struct rcvServerData getUserInfo(int s, char phoneNumber[]);
int search(char filename[], char name[], char searchResult[]);
int searchGroup(char file[], char groupName[], char searchResult[]);
void sendMessage(struct rcvServerData userLocation, char phoneNumber[], char userMessage[]);
void showContacts();
void showGroupContacts();
void showMenu(int s);
void showMessageMenu(int s);
void *funcThread(void *nns);

/* Cliente TCP */
int main(int argc, char *argv[])
{
	unsigned short port;
	struct hostent *hostnm;
	struct sockaddr_in server;
	int s;
	char recvbuf;
	pthread_t thread_id;

	int ns;
	pthread_create(&thread_id, NULL, &funcThread, (void*)&ns);

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
		data.userPort = userSocketPort;

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

void addContact() {
	char contact[20];
	char phone[15] = "";
	char phoneNumberContact[15];
	char filename[30];
	FILE * file;
	int flag = 0;

	strcpy(filename, "");
	strcat(filename, phoneNumber);
	strcat(filename, "-contatos.txt");

	do{
		printf("Digite o nome do contato:\n");
		__fpurge(stdin);

		fgets(contact, sizeof(contact), stdin);
                flag = search(filename, contact, phone);
		
		if(flag == 1)
			printf("Contato já existente, digite novamente...\n");
			
		

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
	int flag = 0;
	char str[100];
	char filename[30];

	strcpy(filename, "");
	strcat(filename, phoneNumber);
	strcat(filename, "-grupos.txt");

	do{
		flag = 0;
	
		printf("Digite o nome do grupo:\n");	
		__fpurge(stdin);
		fgets(nomeGrupo, sizeof(nomeGrupo), stdin);

		arquivo = fopen(filename,"r");
		if (arquivo == NULL){
			break;
		}
		else {
			while (fgets(str, 100, arquivo) != NULL){
				if(strcmp(str, nomeGrupo) == 0){
					flag = 1;
					printf("\nGrupo já existente, digite outro nome..\n\n");
				}
			}
		}
		fclose(arquivo);
	}while(flag == 1);
	
	arquivo = fopen(filename, "a");
	fprintf(arquivo, "%s", nomeGrupo);

	while(sair =='s'){
		do{
			strcpy(numero, "");
			printf("\nDigite o numero que deseja adicionar:\n");	
			__fpurge(stdin);
			fgets(numero, sizeof(numero), stdin);
		}while(strlen(numero) <= 1);

		fprintf(arquivo, "%s", numero);
		printf("\nDeseja inserir outro numero? (S/N)\n");
		scanf("%c", &sair);
	} 
	fputc('\n', arquivo);
	fclose(arquivo);
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
	
	return rcvData;
}

int search(char filename[], char name[], char searchResult[]){

	char contact[20];
	FILE * file;

	file = fopen(filename,"r");
	if (file == NULL){
		return 0;
	}
	else {
		while (fgets(contact, 100, file) != NULL){
			if(strcmp(contact, name) == 0){
				fgets(searchResult, 100, file);
				strtok(searchResult, "\n");
				fclose(file);
				return 1;
			}
		}
	}
	fclose(file);
	return 0;
}

int searchGroup(char file[], char groupName[], char searchResult[]){
    FILE *fp;
    char str[100];
    int organizer = 0;
	char contactsConcate[100];	

    fp = fopen(file, "r");
    if (fp == NULL){
        printf("Nao ha grupos salvos.\n\n");
    }else{
        while (fgets(str, sizeof(str), fp) != NULL){
            if(strcmp(str, groupName) == 0){
                organizer = 1;
            }else if(organizer >= 1){
                if(strlen(str) == 1){
                    fclose(fp);
					return 1;
                }else{
					strcat(searchResult, str);
                    organizer++;
                }
            }
        }
        fclose(fp);
    }
	return 0;
}

void sendMessage(struct rcvServerData userLocation, char phoneNumber[], char userMessage[]) {
	struct sockaddr_in p2pServer;
	struct messageData message;
	int p2psocket;

	strcpy(message.senderPhoneNumber, phoneNumber);
	strcpy(message.message, userMessage);

	p2pServer.sin_family = AF_INET;
	p2pServer.sin_port = userLocation.port;
	p2pServer.sin_addr = userLocation.ip;

	/* Cria um socket TCP (stream) */
	if ((p2psocket = socket(PF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("Socket()");
		exit(3);
	}
	
	/* Estabelece conex�o com um outro cliente */
	if (connect(p2psocket, (struct sockaddr *)&p2pServer, sizeof(p2pServer)) < 0)
	{
		perror("Connect()");
		exit(4);
	}
	printf("\nEnviando mensagem...\n");
	if (send(p2psocket, &message, sizeof(message), 0) < 0) {
            perror("Send()");
            exit(5);
        }
	printf("Mensagem Enviada...\n");
	sleep(1);
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
	char filename[30];
	int organizer = 0;

	strcpy(filename, "");
	strcat(filename, phoneNumber);
	strcat(filename, "-grupos.txt");

	fp = fopen(filename, "r");
	if (fp == NULL){
		printf("Nao ha grupos salvos.\n\n");
	}else{
		printf("Lista de Grupos:\n\n");
		
		while (fgets(str, sizeof(str), fp) != NULL){
					
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
		fclose(fp);
	}
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

void showMessageMenu(int s) {
	int messageOption = 0;
	char filename[30];
	char filenameGroup[30];
	char receiverPhone[15];
	char receiverContactPhone[100];
	char message[144];

	strcpy(filename, "");
	strcat(filename, phoneNumber);
	strcat(filename, "-contatos.txt");
			
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
		//strtok(contact, "\n");
		
		struct rcvServerData userInfo;

		if(search(filename, contact, receiverPhone) == 1){
			userInfo = getUserInfo(s, receiverPhone);
		}
		else{
			userInfo = getUserInfo(s, contact);
		}
		
		if(userInfo.status == 1) {
			printf("A mensagem não pode ser enviada. O usuário está offline.\n");
		}
		else {
			printf("Digite a mensagem:\n");
			__fpurge(stdin);
			fgets(message, sizeof(message), stdin);
			strtok(message, "\n");
			sendMessage(userInfo, data.phoneNumber, message);
		}
		 
	}
	else if (messageOption == 2) {
		strcpy(filenameGroup, "");
		strcat(filenameGroup, phoneNumber);
		strcat(filenameGroup, "-grupos.txt");
		
		printf("Digite a mensagem:\n");
		__fpurge(stdin);
		fgets(message, sizeof(message), stdin);
		strtok(message, "\n");
		
		char group[20];
		printf("Para que grupo deseja enviar a mensagem?\n");
		__fpurge(stdin);
		fgets(group, sizeof(group), stdin);
		//strtok(group, "\n");

		// [WHATS-013] função que buscaria o "group" no arquivo de grupos e retornaria o todos os telefones e/ou contatos
		// chamaria a função criada na tarefa [WHATS-012], para retornar os telefones daqueles que são contatos
		// entraria em um loop que solicita as informações no servidor, recebe e envia a mensagem, até que a lista de telefones acabe.

		struct rcvServerData userInfo;

		strcpy(receiverContactPhone, "");

		if(searchGroup(filenameGroup, group, receiverContactPhone) == 1){
			char groupContact[20];
			int countGroupContact = 0;
			//strtok(receiverContactPhone, "\n");
			//userInfo = getUserInfo(s, receiverContactPhone);
			strcpy(receiverPhone, "");
			strcpy(groupContact, "");
			for(int i = 0; i < strlen(receiverContactPhone); i++){
				if (receiverContactPhone[i] == '\n'){
					groupContact[countGroupContact] = '\0';
					countGroupContact = 0;
					printf("filename= %s || groupContact = %s\n", filename, groupContact);
					if(search(filename, groupContact, receiverPhone) == 1){
						strtok(receiverPhone, "\n");
						userInfo = getUserInfo(s, receiverPhone);
					}
					else {
						userInfo = getUserInfo(s, groupContact);
					}

					if(userInfo.status == 1) {
						printf("A mensagem não pode ser enviada. O usuário %s está offline.\n", groupContact);
					}
					else {
						sendMessage(userInfo, data.phoneNumber, message);
					}

					strcpy(groupContact, "");
				}
				else {
					groupContact[countGroupContact] = receiverContactPhone[i];
					countGroupContact++;
				}
			}			
		}
		else{
			printf("A mensagem não pode ser enviada. O grupo não existe.\n");
		}
	}
}

/* Thread responsável pela recepção das mensagens de outros clientes */
void *funcThread(void *nns) {	
	/* Cria um socket TCP (stream) para aguardar conex�es */
	if ((p2ps = socket(PF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("Socket()");
		exit(2);
	}

	p2pServer.sin_family = AF_INET;
	p2pServer.sin_port = 0;
	p2pServer.sin_addr.s_addr = INADDR_ANY;


	/* Liga o servidor � porta definida anteriormente. */
	if (bind(p2ps, (struct sockaddr *)&p2pServer, sizeof(p2pServer)) < 0)
	{
		perror("Bind()");
		exit(3);
	}
	
	if (listen(p2ps, 1) != 0)
	{
		perror("Listen()");
		exit(4);
	}

	int len = sizeof(p2pServer);

	if (getsockname(p2ps, (struct sockaddr *) &p2pServer, &len) == -1){
		perror("getsockname");
		exit(1);
	}

	userSocketPort = p2pServer.sin_port;

	while(1) {
		int namelen = sizeof(p2pClient);
		if ((userSocket = accept(p2ps, (struct sockaddr *)&p2pClient, &namelen)) == -1)
		{
			perror("Accept()");
			exit(5);
		}
		/* Recebe uma mensagem do cliente atrav�s do novo socket conectado */
		if (recv(userSocket, &message, sizeof(message), 0) == -1)
		{
			perror("Recv()");
			exit(6);
		}

		printf("Mensagem de %s: %s\n", message.senderPhoneNumber, message.message);

		close(userSocket);
	}
}

