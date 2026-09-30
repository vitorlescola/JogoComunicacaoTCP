#include <winsock2.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <windows.h>

#pragma comment(lib, "ws2_32.lib")

#define PORTA 51171
#define BUFFER_SIZE 512


/* =========================================================
   FUNCOES AUXILIARES
   ========================================================= */

void limparTela() {
    system("cls");
}

void limparEntrada(char *str) {
    str[strcspn(str, "\r\n")] = '\0';
}


/*
    Recebe uma linha completa.
*/
int receberMensagem(
    SOCKET socket,
    char *buffer,
    int tamanho
) {

    int posicao = 0;

    while (posicao < tamanho - 1) {

        char caractere;

        int resultado = recv(
            socket,
            &caractere,
            1,
            0
        );

        if (resultado <= 0) {
            return 0;
        }

        if (caractere == '\n') {
            break;
        }

        if (caractere != '\r') {
            buffer[posicao++] = caractere;
        }
    }

    buffer[posicao] = '\0';

    return 1;
}


/*
    Envia uma linha completa.
*/
int enviarMensagem(
    SOCKET socket,
    const char *mensagem
) {

    char buffer[BUFFER_SIZE];

    snprintf(
        buffer,
        sizeof(buffer),
        "%s\n",
        mensagem
    );

    int tamanho =
        (int)strlen(buffer);

    int enviado = 0;

    while (enviado < tamanho) {

        int resultado = send(
            socket,
            buffer + enviado,
            tamanho - enviado,
            0
        );

        if (resultado == SOCKET_ERROR) {
            return 0;
        }

        enviado += resultado;
    }

    return 1;
}


/*
    Executa o cara a cara.
*/
void iniciarCaraACara(
    SOCKET clientSocket,
    const char *nomeCategoria,
    const char *nomeCliente
) {

    char buffer[BUFFER_SIZE];

    /*
        Quantidade de mensagens enviadas pelo cliente.
    */
    int qtdDeRodadasClient = 0;

    /*
        Quantidade recebida do servidor no final.
    */
    int qtdDeRodadasServer = 0;

    char nomeServidor[BUFFER_SIZE] = "";


    limparTela();


    /*
        ==============================
        CABEÇALHO
        ==============================
    */

    printf(
        "========================================\n"
    );

    printf(
        "           CATEGORIA: %s\n",
        nomeCategoria
    );

    printf(
        "           SEU NOME: %s\n",
        nomeCliente
    );

    printf(
        "========================================\n\n"
    );


    printf(
        "Aguardando mensagem do SERVER...\n"
    );


    /*
        ==============================
        COMUNICAÇÃO
        ==============================
    */

    while (1) {

        /*
            --------------------------------
            VEZ DO SERVER
            --------------------------------
        */

        if (!receberMensagem(
                clientSocket,
                buffer,
                sizeof(buffer)
            )) {

            printf(
                "\nServidor desconectou.\n"
            );

            return;
        }


        /*
            Resultado final.
        */
        if (strncmp(
                buffer,
                "RESULT:",
                7
            ) == 0) {

            /*
                Formato:

                RESULT:nomeServer|nomeClient|qtdServer|qtdClient
            */

            char dados[BUFFER_SIZE];

            strcpy(
                dados,
                buffer + 7
            );


            char *parte1 = strtok(
                dados,
                "|"
            );

            char *parte2 = strtok(
                NULL,
                "|"
            );

            char *parte3 = strtok(
                NULL,
                "|"
            );

            char *parte4 = strtok(
                NULL,
                "|"
            );


            if (
                parte1 != NULL &&
                parte2 != NULL &&
                parte3 != NULL &&
                parte4 != NULL
            ) {

                strcpy(
                    nomeServidor,
                    parte1
                );

                qtdDeRodadasServer =
                    atoi(parte3);

                qtdDeRodadasClient =
                    atoi(parte4);
            }

            continue;
        }


        /*
            Fim do jogo.
        */
        if (strcmp(
                buffer,
                "GAME_END"
            ) == 0) {

            break;
        }


        /*
            Servidor encerrou antes de uma
            mensagem normal.
        */
        if (strcmp(
                buffer,
                "GAME_EXIT"
            ) == 0) {

            break;
        }


        /*
            --------------------------------
            MENSAGEM DO SERVER
            --------------------------------
        */

        if (strncmp(
                buffer,
                "SERVER:",
                7
            ) == 0) {

            printf(
                "\nSERVER: %s\n",
                buffer + 7
            );
        }


        /*
            --------------------------------
            VEZ DO CLIENTE
            --------------------------------
        */

        printf(
            "\nSua vez (CLIENT)\n"
        );

        printf(
            "Digite sua mensagem (ou /sair): "
        );


        if (fgets(
                buffer,
                sizeof(buffer),
                stdin
            ) == NULL) {

            return;
        }

        limparEntrada(buffer);


        /*
            /sair NÃO conta.
        */
        if (strcmp(
                buffer,
                "/sair"
            ) == 0) {

            enviarMensagem(
                clientSocket,
                "GAME_EXIT"
            );

            return;
        }


        /*
            Mensagem normal do cliente.
            Conta +1.
        */
        qtdDeRodadasClient++;


        char mensagemEnvio[BUFFER_SIZE];

        snprintf(
            mensagemEnvio,
            sizeof(mensagemEnvio),
            "CLIENT:%s",
            buffer
        );


        if (!enviarMensagem(
                clientSocket,
                mensagemEnvio
            )) {

            printf(
                "\nErro ao enviar mensagem.\n"
            );

            return;
        }


        printf(
            "\nAguardando mensagem do SERVER...\n"
        );
    }


    /*
        ==============================
        RESULTADO FINAL
        ==============================
    */

    limparTela();

    printf(
        "========================================\n"
    );

    printf(
        "             FIM DO CARA A CARA\n"
    );

    printf(
        "========================================\n\n"
    );

    printf(
        "Categoria: %s\n\n",
        nomeCategoria
    );

    printf(
        "SERVER: %s\n",
        nomeServidor
    );

    printf(
        "Quantidade de mensagens: %d\n\n",
        qtdDeRodadasServer
    );

    printf(
        "CLIENT: %s\n",
        nomeCliente
    );

    printf(
        "Quantidade de mensagens: %d\n",
        qtdDeRodadasClient
    );

    printf(
        "\n========================================\n"
    );

    system("pause");
}

/* =========================================================
   MAIN
   ========================================================= */

int main() {

    WSADATA winsocketsDados;


    /*
        ======================================
        WINSOCK
        ======================================
    */

    if (WSAStartup(
            MAKEWORD(2, 2),
            &winsocketsDados
        ) != 0) {

        printf(
            "Falha ao inicializar o Winsock.\n"
        );

        return 1;
    }


    /*
        ======================================
        SOCKET
        ======================================
    */

    SOCKET clientSocket = socket(
        AF_INET,
        SOCK_STREAM,
        IPPROTO_TCP
    );


    if (clientSocket == INVALID_SOCKET) {

        printf(
            "Erro ao criar socket: %d\n",
            WSAGetLastError()
        );

        WSACleanup();

        return 1;
    }


    /*
        ======================================
        SERVIDOR
        ======================================
    */

    struct sockaddr_in serverAddr;

    memset(
        &serverAddr,
        0,
        sizeof(serverAddr)
    );

    serverAddr.sin_family = AF_INET;

    serverAddr.sin_addr.s_addr =
        inet_addr("127.0.0.1");

    serverAddr.sin_port =
        htons(PORTA);


    /*
        ======================================
        CONNECT
        ======================================
    */

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


    limparTela();

    printf(
        "========================================\n"
    );

    printf(
        "       CONECTADO AO SERVIDOR\n"
    );

    printf(
        "========================================\n\n"
    );

    printf(
        "Aguardando o servidor selecionar uma categoria...\n"
    );


    /*
        ======================================
        LOOP PRINCIPAL DO CLIENTE
        ======================================
    */

    while (1) {

        char recvBuffer[BUFFER_SIZE];

        memset(
            recvBuffer,
            0,
            sizeof(recvBuffer)
        );


        /*
            Aguarda comando do servidor.
        */

        if (!receberMensagem(
                clientSocket,
                recvBuffer,
                sizeof(recvBuffer)
            )) {

            printf(
                "\nServidor fechou a conexao.\n"
            );

            break;
        }


        /*
            ==================================
            EXIT
            ==================================
        */

        if (strcmp(
                recvBuffer,
                "EXIT"
            ) == 0) {

            printf(
                "\nServidor encerrou o programa.\n"
            );

            break;
        }


        /*
            ==================================
            CLEAR
            ==================================
        */

        if (strcmp(
                recvBuffer,
                "CLEAR"
            ) == 0) {

            limparTela();

            continue;
        }


        /*
            ==================================
            CATEGORY
            ==================================
        */

if (strncmp(
        recvBuffer,
        "CATEGORY:",
        9
    ) == 0) {

    /*
        Formato recebido:

        CATEGORY:nome_da_categoria|nome_do_cliente
    */

    char dados[BUFFER_SIZE];

    strcpy(
        dados,
        recvBuffer + 9
    );


    /*
        Separa categoria e nome.
    */

    char *separador = strchr(
        dados,
        '|'
    );


    if (separador == NULL) {

        printf(
            "\nErro: dados da categoria invalidos.\n"
        );

        continue;
    }


    /*
        Troca | por \0.

        Antes:

        categoria|nome

        Depois:

        categoria\0nome
    */

    *separador = '\0';


    char categoria[BUFFER_SIZE];
    char nomeCliente[BUFFER_SIZE];


    strcpy(
        categoria,
        dados
    );

    strcpy(
        nomeCliente,
        separador + 1
    );


    /*
        Inicia o cara a cara.
    */

    iniciarCaraACara(
        clientSocket,
        categoria,
        nomeCliente
    );


    /*
        Depois volta a aguardar
        uma nova categoria.
    */

    limparTela();

    printf(
        "========================================\n"
    );

    printf(
        "       AGUARDANDO NOVA CATEGORIA\n"
    );

    printf(
        "========================================\n"
    );

    continue;
}



        /*
            Caso chegue uma mensagem desconhecida.
        */

        printf(
            "\nMensagem do servidor: %s\n",
            recvBuffer
        );
    }


    closesocket(clientSocket);

    WSACleanup();

    printf(
        "\nCliente encerrado.\n"
    );

    return 0;
}
