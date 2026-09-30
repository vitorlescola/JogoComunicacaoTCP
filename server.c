#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <winsock2.h>
#include <windows.h>

#pragma comment(lib, "ws2_32.lib")

#define PORTA 51171
#define BUFFER_SIZE 512
#define MAX_NOMES 500
#define MAX_CATEGORIAS 100
#define MAX_LINHA 256

#define PASTA_CATEGORIAS "categorias"

typedef struct {
    char nome[MAX_LINHA];
    char arquivo[MAX_LINHA];
} Categoria;


/* =========================================================
   FUNCOES AUXILIARES
   ========================================================= */

void limparTela() {
    system("cls");
}

void limparEntrada(char *str) {
    str[strcspn(str, "\r\n")] = '\0';
}

void mostrarMenuPrincipal() {
    printf("\n");
    printf("============== MENU ==============\n");
    printf("1 - Selecionar categoria\n");
    printf("2 - Criar categoria\n");
    printf("3 - Creditos\n");
    printf("4 - Sair\n");
    printf("==================================\n");
    printf("\nOpcao: ");
}

void mostrarMenuCategorias(Categoria categorias[], int quantidade) {
	int i;
	
    printf("\n");
    printf("========= CATEGORIAS =========\n");

    for (i = 0; i < quantidade; i++) {
        printf("%d - %s\n", i + 1, categorias[i].nome);
    }

    printf("%d - Sair\n", quantidade + 1);

    printf("==============================\n");
    printf("\nOpcao: ");
}


/*
    Envia uma mensagem inteira pelo socket.

    O TCP nao garante que um send() corresponde exatamente
    a um recv(), portanto colocamos '\n' no final e o cliente
    trata as mensagens como linhas.
*/
int enviarMensagem(SOCKET socket, const char *mensagem) {

    char buffer[BUFFER_SIZE];

    snprintf(
        buffer,
        sizeof(buffer),
        "%s\n",
        mensagem
    );

    int tamanho = (int)strlen(buffer);
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
    Recebe uma linha do cliente.
*/
int receberMensagem(SOCKET socket, char *buffer, int tamanho) {

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
    Procura os arquivos .txt dentro da pasta categorias.

    Cada arquivo encontrado vira uma categoria.
*/
int carregarCategorias(Categoria categorias[]) {

    WIN32_FIND_DATAA dados;
    HANDLE busca;

    char caminho[MAX_LINHA];

    snprintf(
        caminho,
        sizeof(caminho),
        "%s\\*.txt",
        PASTA_CATEGORIAS
    );

    busca = FindFirstFileA(caminho, &dados);

    if (busca == INVALID_HANDLE_VALUE) {
        return 0;
    }

    int quantidade = 0;

    do {

        if (dados.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            continue;
        }

        if (quantidade >= MAX_CATEGORIAS) {
            break;
        }

        /*
            Guarda o nome do arquivo.
        */
        strncpy(
            categorias[quantidade].arquivo,
            dados.cFileName,
            MAX_LINHA - 1
        );

        categorias[quantidade].arquivo[MAX_LINHA - 1] = '\0';


        /*
            Remove .txt para mostrar um nome mais bonito.
        */
        strncpy(
            categorias[quantidade].nome,
            dados.cFileName,
            MAX_LINHA - 1
        );

        categorias[quantidade].nome[MAX_LINHA - 1] = '\0';

        char *extensao = strrchr(
            categorias[quantidade].nome,
            '.'
        );

        if (extensao != NULL) {
            *extensao = '\0';
        }

        quantidade++;

    } while (FindNextFileA(busca, &dados));

    FindClose(busca);

    return quantidade;
}


/*
    Le todos os nomes de uma categoria.
*/
int carregarNomes(
    const char *arquivo,
    char nomes[][MAX_LINHA]
) {

    char caminho[MAX_LINHA];

    snprintf(
        caminho,
        sizeof(caminho),
        "%s\\%s",
        PASTA_CATEGORIAS,
        arquivo
    );

    FILE *file = fopen(caminho, "r");

    if (file == NULL) {

        printf(
            "\nErro ao abrir o arquivo: %s\n",
            caminho
        );

        return 0;
    }

    int quantidade = 0;

    while (
        quantidade < MAX_NOMES &&
        fgets(
            nomes[quantidade],
            MAX_LINHA,
            file
        ) != NULL
    ) {

        limparEntrada(nomes[quantidade]);

        /*
            Ignora linhas vazias.
        */
        if (strlen(nomes[quantidade]) == 0) {
            continue;
        }

        quantidade++;
    }

    fclose(file);

    return quantidade;
}


/*
    Mostra todos os nomes da categoria.
*/
void mostrarNomes(
    char nomes[][MAX_LINHA],
    int quantidade
) {

    printf("\n");
    printf("========== NOMES ==========\n");
    int i;
    for (i = 0; i < quantidade; i++) {
        printf("%d - %s\n", i + 1, nomes[i]);
    }

    printf("===========================\n");
}


/*
    Aguarda S ou N.
*/
char perguntarUtilizarCategoria() {

    char entrada[20];

    while (1) {

        printf("\nUtilizar esta categoria?: (S/N) ");

        if (fgets(
                entrada,
                sizeof(entrada),
                stdin
            ) == NULL) {

            continue;
        }

        limparEntrada(entrada);

        if (strlen(entrada) == 1) {

            if (
                entrada[0] == 'S' ||
                entrada[0] == 's'
            ) {
                return 'S';
            }

            if (
                entrada[0] == 'N' ||
                entrada[0] == 'n'
            ) {
                return 'N';
            }
        }

        printf("\nDigite somente S ou N.\n");
    }
}


/*
    Faz o cara a cara.

    Ordem:

    SERVER
    CLIENT
    SERVER
    CLIENT
    ...
*/
void iniciarCaraACara(
    SOCKET clientSocket,
    const char *nomeCategoria,
    char nomes[][MAX_LINHA],
    int quantidadeNomes
) {

    char buffer[BUFFER_SIZE];

    if (quantidadeNomes < 2) {
        printf("\nA categoria precisa ter pelo menos 2 nomes.\n");
        system("pause");
        return;
    }

    /*
        Sorteia dois nomes diferentes.
    */
    int indiceServidor = rand() % quantidadeNomes;
    int indiceCliente;

    do {
        indiceCliente = rand() % quantidadeNomes;
    } while (indiceCliente == indiceServidor);

    char nomeServidor[MAX_LINHA];
    char nomeCliente[MAX_LINHA];

    strcpy(nomeServidor, nomes[indiceServidor]);
    strcpy(nomeCliente, nomes[indiceCliente]);


    /*
        Quantidade de mensagens enviadas por cada jogador.
    */
    int qtdDeRodadasServer = 0;
    int qtdDeRodadasClient = 0;


    limparTela();

    enviarMensagem(
        clientSocket,
        "CLEAR"
    );


    /*
        Envia categoria e nome do cliente.
    */
    snprintf(
        buffer,
        sizeof(buffer),
        "CATEGORY:%s|%s",
        nomeCategoria,
        nomeCliente
    );

    enviarMensagem(
        clientSocket,
        buffer
    );


    /*
        ==============================
        TELA DO SERVIDOR
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
        nomeServidor
    );

    printf(
        "========================================\n\n"
    );


    /*
        ==============================
        PRIMEIRA MENSAGEM - SERVIDOR
        ==============================
    */

    printf(
        "Sua vez (SERVER)\n"
    );

    printf(
        "Digite sua mensagem: "
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
        A primeira mensagem também conta.
    */
    qtdDeRodadasServer++;


    char mensagemEnvio[BUFFER_SIZE];

    snprintf(
        mensagemEnvio,
        sizeof(mensagemEnvio),
        "SERVER:%s",
        buffer
    );

    enviarMensagem(
        clientSocket,
        mensagemEnvio
    );


    /*
        ==============================
        COMUNICAÇÃO
        ==============================
    */

    while (1) {

        /*
            --------------------------------
            VEZ DO CLIENTE
            --------------------------------
        */

        printf(
            "\nAguardando mensagem do CLIENTE...\n"
        );

        if (!receberMensagem(
                clientSocket,
                buffer,
                sizeof(buffer)
            )) {

            printf(
                "\nCliente desconectou.\n"
            );

            break;
        }


        /*
            /sair não é contado.
        */
        if (strcmp(
                buffer,
                "GAME_EXIT"
            ) == 0) {

            break;
        }


        /*
            Cliente enviou uma mensagem.
            Agora conta +1.
        */
        if (strncmp(
                buffer,
                "CLIENT:",
                7
            ) == 0) {

            qtdDeRodadasClient++;

            printf(
                "\nCLIENT: %s\n",
                buffer + 7
            );
        }


        /*
            --------------------------------
            VEZ DO SERVIDOR
            --------------------------------
        */

        printf(
            "\nSua vez (SERVER)\n"
        );

        printf(
            "Digite sua mensagem (ou /sair): "
        );

        if (fgets(
                buffer,
                sizeof(buffer),
                stdin
            ) == NULL) {

            break;
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

            break;
        }


        /*
            Mensagem normal do servidor.
            Conta +1.
        */
        qtdDeRodadasServer++;


        snprintf(
            mensagemEnvio,
            sizeof(mensagemEnvio),
            "SERVER:%s",
            buffer
        );

        if (!enviarMensagem(
                clientSocket,
                mensagemEnvio
            )) {

            printf(
                "\nErro ao enviar mensagem.\n"
            );

            break;
        }
    }


    /*
        ==============================
        ENVIA RESULTADO PARA CLIENTE
        ==============================
    */

    snprintf(
        buffer,
        sizeof(buffer),
        "RESULT:%s|%s|%d|%d",
        nomeServidor,
        nomeCliente,
        qtdDeRodadasServer,
        qtdDeRodadasClient
    );

    enviarMensagem(
        clientSocket,
        buffer
    );


    /*
        Dá um pequeno tempo para garantir
        que o cliente receba o resultado
        antes de receber GAME_END.
    */

    Sleep(100);

    enviarMensagem(
        clientSocket,
        "GAME_END"
    );


    /*
        ==============================
        RESULTADO NO SERVIDOR
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

    if (WSAStartup(
            MAKEWORD(2, 2),
            &winsocketsDados
        ) != 0) {

        printf(
            "WSAStartup falhou.\n"
        );

        return 1;
    }


    /*
        Cria a pasta categorias caso ela ainda nao exista.
    */
    CreateDirectoryA(
        PASTA_CATEGORIAS,
        NULL
    );


    /*
        Inicializa aleatoriedade.
    */
    srand(
        (unsigned int)time(NULL)
    );


    /*
        ======================================
        SOCKET DO SERVIDOR
        ======================================
    */

    SOCKET sock = socket(
        AF_INET,
        SOCK_STREAM,
        IPPROTO_TCP
    );

    if (sock == INVALID_SOCKET) {

        printf(
            "Erro ao criar socket: %d\n",
            WSAGetLastError()
        );

        WSACleanup();

        return 1;
    }


    struct sockaddr_in server;

    memset(
        &server,
        0,
        sizeof(server)
    );

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORTA);


    if (bind(
            sock,
            (struct sockaddr*)&server,
            sizeof(server)
        ) == SOCKET_ERROR) {

        printf(
            "Erro no bind: %d\n",
            WSAGetLastError()
        );

        closesocket(sock);
        WSACleanup();

        return 1;
    }


    if (listen(
            sock,
            SOMAXCONN
        ) == SOCKET_ERROR) {

        printf(
            "Erro no listen: %d\n",
            WSAGetLastError()
        );

        closesocket(sock);
        WSACleanup();

        return 1;
    }


    printf(
        "Servidor aguardando conexao na porta %d...\n",
        PORTA
    );


    /*
        ======================================
        ACEITA CLIENTE
        ======================================
    */

    SOCKET clientSocket;

    struct sockaddr_in clientAddr;

    int clientAddrLen =
        sizeof(clientAddr);

    clientSocket = accept(
        sock,
        (struct sockaddr*)&clientAddr,
        &clientAddrLen
    );

    if (clientSocket == INVALID_SOCKET) {

        printf(
            "Erro ao aceitar cliente: %d\n",
            WSAGetLastError()
        );

        closesocket(sock);
        WSACleanup();

        return 1;
    }


    printf(
        "\nCliente conectado com sucesso!\n"
    );


    /*
        ======================================
        MENU PRINCIPAL
        ======================================
    */

    while (1) {

        limparTela();

        mostrarMenuPrincipal();

        char entrada[50];

        if (fgets(
                entrada,
                sizeof(entrada),
                stdin
            ) == NULL) {

            continue;
        }

        limparEntrada(entrada);


        /*
            ----------------------------------
            OPCAO 1
            ----------------------------------
        */

        if (strcmp(entrada, "1") == 0) {

            Categoria categorias[MAX_CATEGORIAS];

            int quantidadeCategorias =
                carregarCategorias(categorias);


            if (quantidadeCategorias == 0) {

                printf(
                    "\nNenhuma categoria encontrada.\n"
                );

                printf(
                    "Coloque arquivos .txt dentro da pasta "
                    "\"categorias\".\n"
                );

                system("pause");

                continue;
            }


            /*
                Menu de categorias.
            */

            while (1) {

                limparTela();

                mostrarMenuCategorias(
                    categorias,
                    quantidadeCategorias
                );


                if (fgets(
                        entrada,
                        sizeof(entrada),
                        stdin
                    ) == NULL) {

                    continue;
                }

                limparEntrada(entrada);


                int opcaoCategoria =
                    atoi(entrada);


                /*
                    Ultima opcao = Sair.
                */
                if (
                    opcaoCategoria ==
                    quantidadeCategorias + 1
                ) {

                    break;
                }


                /*
                    Verifica se categoria existe.
                */
                if (
                    opcaoCategoria < 1 ||
                    opcaoCategoria > quantidadeCategorias
                ) {

                    printf(
                        "\nOpcao invalida!\n"
                    );

                    system("pause");

                    continue;
                }


                /*
                    --------------------------------
                    ABRE O TXT
                    --------------------------------
                */

                limparTela();

                Categoria categoriaSelecionada =
                    categorias[opcaoCategoria - 1];


                char nomes[MAX_NOMES][MAX_LINHA];

                int quantidadeNomes =
                    carregarNomes(
                        categoriaSelecionada.arquivo,
                        nomes
                    );


                if (quantidadeNomes == 0) {

                    printf(
                        "\nA categoria esta vazia.\n"
                    );

                    system("pause");

                    continue;
                }


                /*
                    Mostra todos os nomes.
                */

                printf(
                    "Categoria: %s\n",
                    categoriaSelecionada.nome
                );

                mostrarNomes(
                    nomes,
                    quantidadeNomes
                );


                /*
                    Pergunta se deseja utilizar.
                */

                char utilizar =
                    perguntarUtilizarCategoria();


                /*
                    N = volta para categorias.
                */

                if (utilizar == 'N') {
                    continue;
                }


                /*
                    S = sorteia um nome.
                */

                int indiceSorteado =
                    rand() % quantidadeNomes;


                char nomeSorteado[MAX_LINHA];

                strcpy(
                    nomeSorteado,
                    nomes[indiceSorteado]
                );


                /*
                    Inicia o cara a cara.
                */

				iniciarCaraACara(
    				clientSocket,
    				categoriaSelecionada.nome,
    				nomes,
    				quantidadeNomes
				);


                /*
                    Depois que termina,
                    volta ao menu principal.
                */

                break;
            }

            continue;
        }


        /*
            ----------------------------------
            OPCAO 2
            ----------------------------------
        */

        if (strcmp(entrada, "2") == 0) {

            limparTela();

            printf(
                "Criar categoria\n"
            );

            printf(
                "Esta funcao pode ser implementada "
                "para criar um novo TXT.\n"
            );

            system("pause");

            continue;
        }


        /*
            ----------------------------------
            OPCAO 3
            ----------------------------------
        */

        if (strcmp(entrada, "3") == 0) {

            limparTela();

            printf(
                "=================================\n"
            );

            printf(
                "             CREDITOS\n"
            );

            printf(
                "=================================\n"
            );

            printf(
                "Simulador de Cara a Cara\n"
            );

            printf(
                "Servidor / Cliente TCP\n"
            );

            printf(
                "=================================\n"
            );

            system("pause");

            continue;
        }


        /*
            ----------------------------------
            OPCAO 4
            ----------------------------------
        */

        if (strcmp(entrada, "4") == 0) {

            enviarMensagem(
                clientSocket,
                "EXIT"
            );

            break;
        }


        printf(
            "\nOpcao invalida!\n"
        );

        system("pause");
    }


    closesocket(clientSocket);
    closesocket(sock);

    WSACleanup();

    printf(
        "\nServidor encerrado.\n"
    );

    return 0;
}
