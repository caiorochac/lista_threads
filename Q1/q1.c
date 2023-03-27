#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#define N_THREAD 100 //quantidade de threads
#define LIMIT 1000000 // limite do contador

int count = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void *inc(void *id) { // funcao para incrementar o contador
    while(1) {
        pthread_mutex_lock(&mutex);
        count++;
        if(count >= LIMIT) { //se a thread chegar no limite, mostra na tela qual thread chegou e encerra o programa
            printf("thread %d: contador = %d\n", id, count);
            exit(0);
        }
        pthread_mutex_unlock(&mutex);
    }
}

int main() {
    pthread_t threads[N_THREAD];

    for(int i = 0; i < N_THREAD; i++) { //cria N_THREAD threads, cada uma com id=i
        pthread_create(&threads[i], NULL, inc, (void *) i);
    }

    

    pthread_exit(NULL);
}