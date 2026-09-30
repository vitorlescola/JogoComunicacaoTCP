#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")

#define PORTA 51171
#define BUFFER_SIZE 512

void mostrarMenu() {
    printf("\n");
    printf("==============MENU==============\n");
    printf("1 - Selecionar categoria\n");
    printf("2 - Criar categoria\n");
    printf("3 - Creditos\n");
    printf("4 - Sair\n");
    printf("\nOpcao: ");
}

int main() {
    WSADATA winsocketsDados;
    int temp;

    // Inicializa o Winsock
    temp = WSAStartup(MAKEWORD(2, 2), &winsocketsDados);

    if (temp != 0) {
        printf("WSAStartup falhou: %d\n", temp);
        return 1;
    }

    printf("WSAStartup carregado com sucesso\n");

    // Cria o socket do servidor
    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (sock == INVALID_SOCKET) {
        printf("Erro ao criar o socket: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    printf("Socket criado com sucesso\n");

    // Configura o servidor
    struct sockaddr_in server;

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORTA);

    // Bind
    if (bind(sock, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR) {
        printf("Erro ao associar o socket: %d\n", WSAGetLastError());
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    printf("Bind realizado com sucesso\n");

    // Listen
    if (listen(sock, SOMAXCONN) == SOCKET_ERROR) {
        printf("Erro ao colocar o socket em estado de escuta: %d\n",
               WSAGetLastError());

        closesocket(sock);
        WSACleanup();
        return 1;
    }

    printf("Servidor aguardando conexao na porta %d...\n", PORTA);

    // Aceita cliente
    SOCKET clientSocket;
    struct sockaddr_in clientAddr;
    int clientAddrLen = sizeof(clientAddr);

    clientSocket = accept(
        sock,
        (struct sockaddr*)&clientAddr,
        &clientAddrLen
    );

    if (clientSocket == INVALID_SOCKET) {
        printf("Erro ao aceitar a conexao: %d\n", WSAGetLastError());

        closesocket(sock);
        WSACleanup();
        return 1;
    }

    printf("\nCliente conectado com sucesso!\n");

    char sendBuffer[BUFFER_SIZE];

    // Menu principal
    while (1) {

        int opcao;
        char entrada[50];

        while (1) {

            mostrarMenu();

            // Lê a opção digitada no servidor
            if (fgets(entrada, sizeof(entrada), stdin) == NULL) {
                continue;
            }

            // Remove o ENTER
            entrada[strcspn(entrada, "\n")] = '\0';

            // Verifica se foi digitado exatamente 1, 2, 3 ou 4
            if (strlen(entrada) == 1 &&
                entrada[0] >= '1' &&
                entrada[0] <= '4') {

                opcao = entrada[0] - '0';
                break;
            }

            printf("\nOpcao invalida!\n");
            printf("Digite somente 1, 2, 3 ou 4.\n");
        }

        // Converte a opção para texto
        sprintf(sendBuffer, "%d", opcao);

        // Envia a opção para o cliente
        int bytesSent = send(
            clientSocket,
            sendBuffer,
            strlen(sendBuffer),
            0
        );

        if (bytesSent == SOCKET_ERROR) {
            printf("Erro ao enviar dados: %d\n", WSAGetLastError());
            break;
        }

        // Se escolheu sair
        if (opcao == 4) {
            printf("\nEncerrando conexao...\n");

            // Envia mensagem de encerramento
            strcpy(sendBuffer, "exit");

            send(
                clientSocket,
                sendBuffer,
                strlen(sendBuffer),
                0
            );

            break;
        }

        // Ações temporárias das opções
        switch (opcao) {

            case 1:
                printf("\nVoce selecionou: Selecionar categoria\n");
                break;

            case 2:
                printf("\nVoce selecionou: Criar categoria\n");
                break;

            case 3:
                printf("\nVoce selecionou: Creditos\n");
                break;
        }
    }

    // Fecha os sockets
    closesocket(clientSocket);
    closesocket(sock);

    WSACleanup();

    printf("Servidor encerrado.\n");

    return 0;
}
