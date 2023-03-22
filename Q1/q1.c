#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#define N_THREAD 100
#define LIMIT 1000000

int count = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void *inc(void *id) {
    while(1) {
        pthread_mutex_lock(&mutex);
        count++;
        if(count >= LIMIT) {
            printf("thread %d: contador = %d\n", id, count);
            exit(0);
        }
        pthread_mutex_unlock(&mutex);
    }
}

int main() {
    pthread_t threads[N_THREAD];

    for(int i = 0; i < N_THREAD; i++) {
        pthread_create(&threads[i], NULL, inc, (void *) i);
    }

    

    pthread_exit(NULL);
}