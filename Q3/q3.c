#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

struct pixel {
    int R;
    int G;
    int B;
};

struct arg { //struct para passar os parametros da thread
    struct pixel p; //pixel do ponto
    struct pixel *res; //ponteiro para a posicao que o resultado deve ser salvo
};

void *grey_scale(void *arguments) { //funcao para calcular o nivel de cinza
    struct arg myArg = *((struct arg *) arguments);
    int res = myArg.p.R * 0.3 + myArg.p.G * 0.59 + myArg.p.B * 0.11;

    (*myArg.res).R = res; //salva o resultado no endereco passado como parametro
    (*myArg.res).G = res;
    (*myArg.res).B = res;

    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    char magic_n[10];
    int LIN, COL, MAX_VALUE;
    FILE *input = fopen(argv[1], "r"); //abre arquivo passado como argumento

    //leitura do arquivo
    fscanf(input, "%s", magic_n);
    fscanf(input, "%d %d", &COL, &LIN);
    fscanf(input, "%d", &MAX_VALUE);

    struct pixel mat[LIN][COL];
    struct pixel mat_grey[LIN][COL];
    pthread_t threads[LIN*COL];
    struct arg args[LIN*COL]; //vetor com os argumentos para as threads

    for(int i = 0; i < LIN; i++) { //salva a imagem numa matriz
        for(int j = 0; j < COL; j++) {
            fscanf(input, "%d %d %d", &mat[i][j].R, &mat[i][j].G, &mat[i][j].B);
        }
    }

    fclose(input); //fim da leitura

    for(int i = 0; i < LIN; i++) { //cria uma thread para cada pixel e cada thread armazena o resultado ma matriz mat_grey
        for(int j = 0; j < COL; j++) {
            int ind = i * COL + j;
            args[ind].p = mat[i][j];
            args[ind].res = &mat_grey[i][j];
            pthread_create(&threads[ind], NULL, grey_scale, (void *) &args[ind]);
        }
    }

    for(int i = 0; i < LIN * COL; i++) { //espera todas as threads terminarem
        pthread_join(threads[i], NULL);
    }

    FILE *output = fopen(argv[2], "w"); //abre o arquivo para escrita da imagem transformada

    //escreve no arquivo
    fprintf(output, "%s\n", magic_n);
    fprintf(output, "%d %d\n", COL, LIN);
    fprintf(output, "%d\n", MAX_VALUE);
    for(int i = 0; i < LIN; i++) {
        for(int j = 0; j < COL; j++) {
            fprintf(output, "%d %d %d\n", mat_grey[i][j].R, mat_grey[i][j].G, mat_grey[i][j].B);
        }
    }

    fclose(output); //fim da escrita
    return 0;
}