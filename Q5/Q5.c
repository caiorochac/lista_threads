#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>


#define buffer_size 10
#define itens 20
#define qtd_threads 5

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t empty = PTHREAD_COND_INITIALIZER;
pthread_cond_t fill = PTHREAD_COND_INITIALIZER;


typedef struct elem{
   int value;
   struct elem *prox;
}Elem;
 
typedef struct blockingQueue{
   int sizeBuffer, statusBuffer;
   Elem *head,*last;
}BlockingQueue;

typedef struct{
    BlockingQueue *bq;
    int element;
}structAux;

BlockingQueue *newBlockingQueue(int SizeBuffer){//função para criar a fila bloqueante
    BlockingQueue *aux = NULL;
    aux = (BlockingQueue *) malloc(sizeof(BlockingQueue));
    aux->head = NULL;
    aux->last = NULL;
    aux->sizeBuffer = SizeBuffer;
    aux->statusBuffer = 0;
    return aux;
}

void putBlockingQueue(BlockingQueue *Q, int intnewValue){//Colocando elementos na fila
    pthread_mutex_lock(&mutex);//bloqueio o mutex
    while(Q->statusBuffer==buffer_size){//ve se o buffer esta cheio. Caso esteja, nao posso coloca elementos na fila
        printf("Nao pode ser colocado\n");
        pthread_cond_wait(&empty, &mutex);
    }//aqui, já pode inserir elementos
    Elem *aux = NULL;
    Elem *aux2;
    aux = (Elem*) malloc(sizeof(Elem));
    aux->value = intnewValue;
    aux->prox = NULL;
    if(Q->statusBuffer==0){
        (*Q).head = aux;
        (*Q).last = aux;
        Q->statusBuffer += 1;
    }else{
        aux2 = (*Q).last;
        aux2->prox = aux;
        (*Q).last = aux;
        Q->statusBuffer += 1;
    }
    printf("Adicionado: %d\n", intnewValue);
    if(Q->statusBuffer>0){//aqui ve se o buffer tem pelo menos 1 item, caso tenha, sinaliza com o fill para saber que pode retirar elementos da fila
        pthread_cond_broadcast(&fill);
    }
    pthread_mutex_unlock(&mutex);//desbloqueia o mutex
}
void *producer(void* arg){
    structAux *myQueue;
    myQueue = (structAux *) arg;
    int i;
    for(i=0; i<itens; i++){
        putBlockingQueue(myQueue->bq, i+myQueue[0].element);
    }
    printf("Produtor encerrado\n");
    pthread_exit(NULL);
}

int takeBlockingQueue(BlockingQueue* Q){//função para remover elementos da fila bloqueante
    int result;
    pthread_mutex_lock(&mutex);//bloqueia o mutex
 
    while(Q->statusBuffer==0){//Ve se o buffer está vazio, caso esteja, espera ser colocado um item pra prosseguir
        printf("Nao pode ser feita retirada\n");
        pthread_cond_wait(&fill, &mutex);
    }//quando chega aqui, significa que tem algum item para retirar e então retira
    Elem *aux;
    aux = (*Q).head;
    (*Q).head = aux->prox;
    result = aux->value;
    if(Q->statusBuffer == 1){
        (*Q).last = NULL;
    }
    Q->statusBuffer -= 1;
    free(aux);
    if(Q->statusBuffer<Q->sizeBuffer){
        pthread_cond_broadcast(&empty);
    }
    pthread_mutex_unlock(&mutex);
    return result;
}
void *consumer(void* arg){
    structAux *myQueue;
    myQueue = (structAux *) arg;
    int i, take;
    for(i=0; i<itens; i++){
        take = takeBlockingQueue(myQueue->bq);
        printf("Removido: %d\n", take);
    }
    pthread_exit(NULL);
}

int main(int argc, char **argv) {
    long i;
    structAux structQueue[1];//ponteiro de estrutura, onde possue a fila bloqueante e o elemento que será inserido na fila
    
    structQueue->bq = newBlockingQueue(buffer_size);//cria a fila bloqueante

    pthread_t prod[qtd_threads];//cria as threads
    pthread_t cons[qtd_threads];
    int ids[qtd_threads];
	
    pthread_mutex_init(&mutex, NULL);//inicializa o mutex    
    
    for(i=0; i<qtd_threads; i++){//cria as threads
        structQueue[0].element = i;
        pthread_create(&(prod[i]), NULL, producer, (void*) &structQueue[0]);
    }
    
    for(i=0; i<qtd_threads; i++){
        structQueue[0].element = i;
        pthread_create(&(cons[i]), NULL, consumer, (void*) &structQueue[0]);
    }

    for(i=0; i<qtd_threads; i++)
        pthread_join(prod[i], NULL);

    for(i=0; i<qtd_threads; i++)
        pthread_join(cons[i], NULL);

    pthread_exit(NULL);
}
