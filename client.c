#include <winsock2.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <windows.h>

#pragma comment(lib, "ws2_32.lib")

#define PORTA 51171
#define BUFFER_SIZE 512

void limparTela(){
    system("cls");
}

void limparEntrada(char *str){
    str[strcspn(str,"\r\n")]='\0';
}

int receberMensagem(SOCKET socket,char *buffer,int tamanho){
    int posicao=0;

    while(posicao<tamanho-1){
        char caractere;
        int resultado=recv(socket,&caractere,1,0);

        if(resultado<=0){
            return 0;
        }

        if(caractere=='\n'){
            break;
        }

        if(caractere!='\r'){
            buffer[posicao++]=caractere;
        }
    }

    buffer[posicao]='\0';

    return 1;
}

int enviarMensagem(SOCKET socket,const char *mensagem){
    char buffer[BUFFER_SIZE];

    snprintf(buffer,sizeof(buffer),"%s\n",mensagem);

    int tamanho=(int)strlen(buffer);
    int enviado=0;

    while(enviado<tamanho){
        int resultado=send(socket,buffer+enviado,tamanho-enviado,0);

        if(resultado==SOCKET_ERROR){
            return 0;
        }

        enviado+=resultado;
    }

    return 1;
}

void iniciarCaraACara(SOCKET clientSocket,const char *nomeCategoria,const char *nomeCliente){
    char buffer[BUFFER_SIZE];
    int qtdDeRodadasClient=0;
    int qtdDeRodadasServer = 0;
    char nomeServidor[BUFFER_SIZE]="";

    limparTela();

    printf("========================================\n");
    printf("           CATEGORIA: %s\n",nomeCategoria);
    printf("           SEU NOME: %s\n",nomeCliente);
    printf("========================================\n\n");
    printf("Aguardando mensagem do SERVER...\n");

    while(1){
        if(!receberMensagem(clientSocket,buffer,sizeof(buffer))){
            printf("\nServidor desconectou.\n");

            return;
        }

        if(strncmp(buffer,"RESULT:",7)==0){
            char dados[BUFFER_SIZE];

            strcpy(dados,buffer+7);

            char *parte1=strtok(dados,"|");
            char *parte2=strtok(NULL,"|");
            char *parte3=strtok(NULL,"|");
            char *parte4=strtok(NULL,"|");

            if(parte1!=NULL&&parte2!=NULL&&parte3!=NULL&&parte4!=NULL){
                strcpy(nomeServidor,parte1);

                qtdDeRodadasServer=atoi(parte3);
                qtdDeRodadasClient=atoi(parte4);
            }

            continue;
        }

        if(strcmp(buffer,"GAME_END")==0){
            break;
        }

        if(strcmp(buffer,"GAME_EXIT")==0){
            break;
        }

        if(strncmp(buffer,"SERVER:",7)==0){
            printf("\nSERVER: %s\n",buffer + 7);
        }

        printf("\nSua vez (CLIENT)\n");
        printf("Digite sua mensagem (ou /sair): ");

        if(fgets(buffer,sizeof(buffer),stdin)==NULL){
            return;
        }

        limparEntrada(buffer);

        if(strcmp(buffer,"/sair")==0){
            enviarMensagem(clientSocket,"GAME_EXIT");

            return;
        }

        qtdDeRodadasClient++;

        char mensagemEnvio[BUFFER_SIZE];

        snprintf(mensagemEnvio,sizeof(mensagemEnvio),"CLIENT:%s",buffer);

        if(!enviarMensagem(clientSocket,mensagemEnvio)){
            printf("\nErro ao enviar mensagem.\n");

            return;
        }

        printf("\nAguardando mensagem do SERVER...\n");
    }

    limparTela();

    printf("========================================\n");
    printf("             FIM DO CARA A CARA\n");
    printf("========================================\n\n");
    printf("Categoria: %s\n\n",nomeCategoria);
    printf("SERVER: %s\n",nomeServidor);
    printf("Quantidade de mensagens: %d\n\n",qtdDeRodadasServer);
    printf("CLIENT: %s\n",nomeCliente);
    printf("Quantidade de mensagens: %d\n",qtdDeRodadasClient);
    printf("\n========================================\n");
    system("pause");
}

int main(){
    WSADATA winsocketsDados;

    if(WSAStartup(MAKEWORD(2,2),&winsocketsDados)!=0){
        printf("Falha ao inicializar o Winsock.\n");

        return 1;
    }

    SOCKET clientSocket=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);

    if(clientSocket==INVALID_SOCKET){
        printf("Erro ao criar socket: %d\n",WSAGetLastError());

        WSACleanup();

        return 1;
    }

    struct sockaddr_in serverAddr;

    memset(&serverAddr,0,sizeof(serverAddr));
    serverAddr.sin_family=AF_INET;
    serverAddr.sin_addr.s_addr=inet_addr("127.0.0.1");
    serverAddr.sin_port=htons(PORTA);

    if(connect(clientSocket,(struct sockaddr*)&serverAddr,sizeof(serverAddr))==SOCKET_ERROR){
        printf("Erro ao conectar ao servidor: %d\n",WSAGetLastError());

        closesocket(clientSocket);
        WSACleanup();

        return 1;
    }


    limparTela();

    printf("========================================\n");
    printf("       CONECTADO AO SERVIDOR\n");
    printf("========================================\n\n");
    printf("Aguardando o servidor selecionar uma categoria...\n");

    while(1){
        char recvBuffer[BUFFER_SIZE];

        memset(recvBuffer,0,sizeof(recvBuffer));

        if(!receberMensagem(clientSocket,recvBuffer,sizeof(recvBuffer))){
            printf("\nServidor fechou a conexao.\n");

            break;
        }

        if(strcmp(recvBuffer,"EXIT")==0){
            printf("\nServidor encerrou o programa.\n");

            break;
        }

        if(strcmp(recvBuffer,"CLEAR")==0){
            limparTela();

            continue;
        }


		if(strncmp(recvBuffer,"CATEGORY:",9)==0){
    		char dados[BUFFER_SIZE];

    		strcpy(dados,recvBuffer+9);

    		char *separador=strchr(dados,'|');

    if(separador==NULL){
        printf("\nErro: dados da categoria invalidos.\n");

        continue;
    }

    *separador='\0';

    char categoria[BUFFER_SIZE];
    char nomeCliente[BUFFER_SIZE];

    strcpy(categoria,dados);
    strcpy(nomeCliente,separador+1);

    iniciarCaraACara(clientSocket,categoria,nomeCliente);
    limparTela();

    printf("========================================\n");
    printf("       AGUARDANDO NOVA CATEGORIA\n");
    printf("========================================\n");

    continue;
	}

        printf("\nMensagem do servidor: %s\n",recvBuffer);
    }


    closesocket(clientSocket);

    WSACleanup();

    printf("\nCliente encerrado.\n");

    return 0;
}
