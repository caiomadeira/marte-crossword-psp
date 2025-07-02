#include <stdio.h>

int main(int argc, char *argv[]) {
    // Verifica se o utilizador forneceu um nome de ficheiro
    if (argc < 2) {
        fprintf(stderr, "Erro: Por favor, forneça o nome do ficheiro a ser lido.\n");
        fprintf(stderr, "Uso: %s <nome_do_ficheiro>\n", argv[0]);
        return 1;
    }

    // Tenta abrir o ficheiro em modo de leitura binária ("rb")
    FILE *file = fopen(argv[1], "rb");
    if (file == NULL) {
        perror("Não foi possível abrir o ficheiro");
        return 1;
    }

    printf("--- Conteúdo Binário de '%s' ---\n", argv[1]);

    int byte;
    int bytes_na_linha = 0;
    char decodedLine[2000];

    // Lê o ficheiro byte a byte até chegar ao fim (EOF - End Of File)
    while ((byte = fgetc(file)) != EOF) {
        // Imprime o byte em formato hexadecimal com dois dígitos (ex: 0A, FF)
        // printf("%02x ", byte);
        printf("%c ", byte);
        bytes_na_linha++;

        // A cada 16 bytes, imprime uma nova linha para organizar a visualização
        if (bytes_na_linha == 16) {
            printf("\n");
            bytes_na_linha = 0;
        }
    }

    printf("\n--- Fim do Ficheiro ---\n");

    // Fecha o ficheiro
    fclose(file);

    return 0;
}
