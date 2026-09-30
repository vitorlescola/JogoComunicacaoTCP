#include <winsock2.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#pragma comment(lib, "ws2_32.lib")

#define PORTA 51171
#define BUFFER_SIZE 512

int main() {

    WSADATA winsocketsDados;

    // Inicializa o Winsock
    if (WSAStartup(MAKEWORD(2, 2), &winsocketsDados) != 0) {
        printf("Falha ao inicializar o Winsock\n");
        return 1;
    }

    printf("WSAStartup carregado com sucesso\n");

    // Cria o socket
    SOCKET clientSocket = socket(
        AF_INET,
        SOCK_STREAM,
        IPPROTO_TCP
    );

    if (clientSocket == INVALID_SOCKET) {
        printf(
            "Erro ao criar o socket: %d\n",
            WSAGetLastError()
        );

        WSACleanup();
        return 1;
    }

    printf("Socket criado com sucesso\n");

    // Configura o endereço do servidor
    struct sockaddr_in serverAddr;

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serverAddr.sin_port = htons(PORTA);

    // Conecta ao servidor
    if (connect(
            clientSocket,
            (struct sockaddr*)&serverAddr,
            sizeof(serverAddr)
        ) == SOCKET_ERROR) {

        printf(
            "Erro ao conectar ao servidor: %d\n",
            WSAGetLastError()
        );

        closesocket(clientSocket);
        WSACleanup();

        return 1;
    }

    printf("\nConectado ao servidor!\n");
    printf("Aguardando opcoes do servidor...\n");

    char recvBuffer[BUFFER_SIZE];

    while (1) {

        // Limpa o buffer
        memset(recvBuffer, 0, sizeof(recvBuffer));

        // Aguarda uma opção do servidor
        int bytesReceived = recv(
            clientSocket,
            recvBuffer,
            sizeof(recvBuffer) - 1,
            0
        );

        if (bytesReceived == SOCKET_ERROR) {

            printf(
                "Erro ao receber dados: %d\n",
                WSAGetLastError()
            );

            break;
        }

        if (bytesReceived == 0) {

            printf("\nServidor fechou a conexao.\n");

            break;
        }

        recvBuffer[bytesReceived] = '\0';

        // Se o servidor enviou exit
        if (strcmp(recvBuffer, "exit") == 0) {

            printf("\nServidor encerrou a conexao.\n");

            break;
        }

        // Mostra a opção recebida
        printf(
            "\nServidor selecionou a opcao: %s\n",
            recvBuffer
        );

        // Executa a ação correspondente
        if (strcmp(recvBuffer, "1") == 0) {

            printf("-> Selecionar categoria\n");

        } else if (strcmp(recvBuffer, "2") == 0) {

            printf("-> Criar categoria\n");

        } else if (strcmp(recvBuffer, "3") == 0) {

            printf("-> Creditos\n");

        } else if (strcmp(recvBuffer, "4") == 0) {

            printf("-> Sair\n");
            break;
        }
    }

    closesocket(clientSocket);
    WSACleanup();

    printf("\nCliente encerrado.\n");

    return 0;
}
