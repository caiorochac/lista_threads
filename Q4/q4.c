#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>
#define P 10
#define I 2
#define LIN 2
#define COL 2

pthread_barrier_t bar;
float Vars[I]; //valores atuais das variaveis
float Vars_prev[I]; //valores da ultima iteracao de cada variavel
float b[LIN] = {11, 13};
float A[LIN][COL] = {{2, 1}, {5, 7}};

struct args { //parametros da thread
    int vars[I]; //variaveis que estao com a thread
    int tam; //quantidade de variaveis que estao com a thread
};

void *jacobi(void *arguments) {
    struct args Args = *((struct args *) arguments);
    int somatorio;

    for(int k = 1; k <= P; k++) {
        for(int i = 0; i < Args.tam; i++) {
            //calculando o somatorio da expressao
            somatorio = 0;
            for(int j = 0; j < I; j++) {
                if(j != Args.vars[i]) {
                    somatorio += A[Args.vars[i]][j] * Vars_prev[j];
                }
            }
            //calculando a expressao completa
            Vars[Args.vars[i]] = (1 / A[Args.vars[i]][Args.vars[i]]) * (b[Args.vars[i]] - somatorio);
        }

        pthread_barrier_wait(&bar); //espera todas as threads calcularem o resultado

        //atualizando o vetor auxiliar
        for(int i = 0; i < Args.tam; i++) {
            Vars_prev[Args.vars[i]] = Vars[Args.vars[i]];
        }

        pthread_barrier_wait(&bar); //espera todas as threads atualizarem o vetor auxiliar
    }

    pthread_exit(NULL);
}

int main() {
    int N; //numero de threads
    scanf("%d", &N);
    struct args Args[N];
    pthread_t threads[N];
    int i = I, j = 0;

    for(int k = 0; k < I; k++) { //inicializa as variaveis com o valor 1
        Vars[k] = 1;
        Vars_prev[k] = 1;
    }

    //inicializa a barreira para esperar a quantidade de threads necessarias
    if(N < I) pthread_barrier_init(&bar, NULL, N);
    else pthread_barrier_init(&bar, NULL, I);

    for(int n = 0; n < N && i > 0; n++) { //distribui as variaveis para as threads e cria a thread
        Args[n].tam = ceil((i + 0.0) / N);
        for(int k = 0; k < Args[n].tam; k++) {
            Args[n].vars[k] = I - i;
            i--;
        }
        pthread_create(&threads[n], NULL, jacobi, (void *) &Args[n]);
    }

    for(int k = 0; k < N; k++) { //espera todas as threads
        pthread_join(threads[i], NULL);
    }

    pthread_barrier_destroy(&bar); //encerra a barreira

    for(int k = 0; k < I; k++) { //imprime o valor final das variaveis
        printf("X%d = %f\n", k, Vars[k]);
    }

    return 0;
}
