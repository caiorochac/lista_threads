#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>
#define P 10
#define I 2
#define LIN 2
#define COL 2

pthread_barrier_t bar;
float Vars[I];
float Vars_prev[I];
float b[LIN] = {11, 13};
float A[LIN][COL] = {{2, 1}, {5, 7}};

struct args {
    int vars[I];
    int tam;
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

        pthread_barrier_wait(&bar);

        //atualizando o vetor auxiliar
        for(int i = 0; i < Args.tam; i++) {
            Vars_prev[Args.vars[i]] = Vars[Args.vars[i]];
        }

        pthread_barrier_wait(&bar);
    }

    pthread_exit(NULL);
}

int main() {
    int N; //numero de threads
    scanf("%d", &N);
    struct args Args[N];
    pthread_t threads[N];
    int i = I, j = 0;

    for(int k = 0; k < I; k++) {
        Vars[k] = 1;
        Vars_prev[k] = 1;
    }

    
    if(N < I) pthread_barrier_init(&bar, NULL, N);
    else pthread_barrier_init(&bar, NULL, I);

    for(int n = 0; n < N && i > 0; n++) {
        Args[n].tam = ceil((i + 0.0) / N);
        for(int k = 0; k < Args[n].tam; k++) {
            Args[n].vars[k] = I - i;
            i--;
        }
        pthread_create(&threads[n], NULL, jacobi, (void *) &Args[n]);
    }

    for(int k = 0; k < N; k++) {
        pthread_join(threads[i], NULL);
    }

    pthread_barrier_destroy(&bar);

    for(int k = 0; k < I; k++) {
        printf("X%d = %f\n", k, Vars[k]);
    }

    return 0;
}
