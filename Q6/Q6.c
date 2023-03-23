#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>

#define thread_users 10 //threads usuário
#define thread_desp 10  //threads despachantes
#define tamBuffer 10    //tamanho do vetor buffer e de resultados

typedef struct{     //vetor buffer
  int ID_thread, ID_operacao, var1, var2;
}Buffer;

typedef struct{    //vetor de resultado
  int resultado, ID_operacao;
}Resultado;

//declaração de mutex e variável de condição
pthread_mutex_t buffer_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t resultado_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t full = PTHREAD_COND_INITIALIZER;
pthread_cond_t empty = PTHREAD_COND_INITIALIZER;
pthread_cond_t resFull = PTHREAD_COND_INITIALIZER;
pthread_cond_t resEmpty= PTHREAD_COND_INITIALIZER;
pthread_cond_t resInteract = PTHREAD_COND_INITIALIZER;
pthread_cond_t finalizou = PTHREAD_COND_INITIALIZER;

int count=0;    //contador da despachante 
int ID_operacao = 0; //o ID da operação que será passado pra resgatar o valor a ser executado 
bool isOver=false; //flag para verificar se todas as t_despachantes foram executadas
Buffer buffer[tamBuffer]; 
Resultado resultados[tamBuffer];
Buffer parametros[thread_users]; //vetor auxiliar onde vai ser guardado os parametros a serem passados pela funexec

int bufferIsFull(){ //verifica buffer, cheio retorna -1, caso contrario retorna a posição livre
  for(int i=0; i < tamBuffer; i++){
    if(buffer[i].ID_thread == -1){
      return i;
    }
  }
  return -1;
}

int bufferIsEmpty(){//verifica buffer, vazio retorna -1, caso contrario retorna a posição ocupada
  for(int i = 0; i < tamBuffer; i++){
    if(buffer[i].ID_thread != -1){
      return i;
    }
  }
  return -1;
}

int resultadoIsFull(){//verifica o resultado, cheio retorna -1, caso contrario retorna a posição livre
  for(int i = 0; i < tamBuffer; i++){
    if(resultados[i].ID_operacao == -1){
      return i;
    }
  }
  return -1;
}

int resultadoIsEmpty(){//verifica resultados, vazio retorna -1, caso contrario retorna a posição ocupada
  for(int i = 0; i < tamBuffer; i++){
    if(resultados[i].ID_operacao != -1){
      return i;
    }
  }
  return -1;
}

int funexec(int var1, int var2){//função funexec a ser executada
  return var1 * var2;
}

int agendarExecucao(Buffer valores){
  int pos;
  pthread_mutex_lock(&buffer_mutex); //acesso restrito ao buffer
  if(bufferIsFull() == -1){ // Buffer cheio, espera por uma posição livre
    pthread_cond_wait(&full, &buffer_mutex);
  }
  pos = bufferIsFull(); //se o buffer nao estiver cheio, retorna a posição em que ele esta vazio
  buffer[pos] = valores; //coloca os valores a serem executados da multiplicação no buffer
  buffer[pos].ID_operacao = ID_operacao; //passa o ID da operacao daqueles valores para aquela posição do buffer
  int ID_retornado=ID_operacao; //coloca o ID em uma variável auxiliar, para ser passado para a função pegarResultadoExecucao
  ID_operacao++; //incrementa o ID da operacao
  printf("A thread de ID %d finalizou, o buffer tem a requisicao de ID %d e os valores sao %d %d\n", valores.ID_thread, buffer[pos].ID_operacao, buffer[pos].var1, buffer[pos].var2);
  pthread_mutex_unlock(&buffer_mutex); 
  pthread_cond_broadcast(&empty); //sinal para as threads despachantes avisando que podem retirar um valor do buffer
  return ID_retornado; //retorna o ID da operação colocada no buffer
}

void pegarResultadoExecucao(int id){
    pthread_mutex_lock(&resultado_mutex); //torna o vetor de resultados restrito
    int resultado_id=-777;  //resultado_id sera usado para ver se o resultado correto entre o vetor de resultados é encontrado, entao é setado para um valor estranho
    while(id!=resultado_id){
        if(resultadoIsEmpty && !isOver){ //se o vetor de resultados estiver vazio, e as despachantes ainda não tiverem terminado de trabalhar, espera o preenchimento
            pthread_cond_wait(&resEmpty, &resultado_mutex);
        }
        for(int i=0; i<tamBuffer; i++){  //checa todas as posições do vetor de resultados
            if(resultados[i].ID_operacao==id){  
                resultado_id=id;    //se achar o ID sai do while 
                printf("O resultado da operação de %d foi: %d\n", resultados[i].ID_operacao, resultados[i].resultado);
                resultados[i].ID_operacao=-1; //print do valor achado e set do id_operacao para -1, para mostrar que aquela posição agora está vazia
                if(isOver && (bufferIsEmpty()==-1) && (resultadoIsEmpty()==-1)){
                    exit(0); //se acabar todo o processamento de resultados, as threads são encerradas, terminando o processo
                } 
                break; 
            }
        }
        if(resultado_id!=id){ //se não achar o ID entre o vetor resultados, espera que o vetor resultados seja preenchido novamente para evitar espera ocupada
            pthread_cond_wait(&resInteract, &resultado_mutex); 
        }
    }
    pthread_mutex_unlock(&resultado_mutex);
    pthread_cond_broadcast(&resFull); //avisa para a despachante que pode inserir no vetor resultados
    return;
}

void *func_despachante(void *arg){
    int posBuffer;
    int posResultado;
    int threadID = *((int*)arg);
    while(!isOver){ //enquanto as despachantes não terminaram de executar
        pthread_mutex_lock(&buffer_mutex);

        if(bufferIsEmpty() == -1){ // Buffer está vazio, thread despachante dorme
            if(isOver){
                pthread_cond_broadcast(&resEmpty);
            }
            pthread_cond_wait(&empty, &buffer_mutex); // despachante_mutex não está sendo usado 
        }

        posBuffer=bufferIsEmpty(); //guarda a posição ocupada do buffer
        Buffer temp;        
        temp=buffer[posBuffer];    
        buffer[posBuffer].ID_thread=-1; //depois de passar os valores para temp, diz que aquela posição está vazia

        pthread_mutex_unlock(&buffer_mutex);
        
        pthread_cond_broadcast(&full);  //diz para agendar a funçao que agora pode preencher o buffer

        pthread_mutex_lock(&resultado_mutex); // Mutex para bloquear o uso do array resultados 
        
        if(resultadoIsFull() == -1){ // Array de resultados cheio, espera liberar espaço
            pthread_cond_wait(&resFull, &resultado_mutex);
        }
        posResultado=resultadoIsFull(); //pega o espaço vazio de resultados
        resultados[posResultado].resultado = funexec(temp.var1, temp.var2); //executa a funexec e guarda o valor
        resultados[posResultado].ID_operacao = temp.ID_operacao;  
        count++; //adiciona no contador de execuções da despachante
        if(count>=thread_users) isOver=true;  //se as despachantes despacharam todos os usários, então seta isOver para true
        pthread_mutex_unlock(&resultado_mutex);
        
        pthread_cond_broadcast(&resEmpty); //avisa para pegarResultadoExecucao que resultados nao esta mais vazia
        pthread_cond_broadcast(&resInteract); //avisa que um valor novo foi colocado em resultados, permitindo uma nova checagem do ID

    }
    pthread_exit(NULL);
}

void *func_user(void *arg){ //gera os valores a serem passados para o buffer
    int threadID = *((int*)arg);

    parametros[threadID].ID_thread = threadID;
    parametros[threadID].var1 = rand()%100;
    parametros[threadID].var2 = rand()%100;

    int id = agendarExecucao(parametros[threadID]);
    pegarResultadoExecucao(id); 
    pthread_exit(NULL);
}

int main(){     //declaração das threads

    pthread_t usuario[thread_users];
    pthread_t despachantes[thread_desp];
    pthread_t sair;
    int *ids_users[thread_users], *ids_desp[thread_desp];

    for(int i = 0; i < tamBuffer; i++){ //seta todas as posições de Buffer e Resultados como desocupadas
      resultados[i].ID_operacao = -1;
      buffer[i].ID_thread = -1;
      buffer[i].ID_operacao = buffer[i].var1 = buffer[i].var2 = 0;
    }

    for(int i = 0; i < thread_users; i++){//cria as threads usuario
      ids_users[i] = (int*) malloc(sizeof(int));
      *ids_users[i] = i;
      if(pthread_create(&usuario[i], NULL, &func_user, (void*)ids_users[i]) != 0){
          perror("Falha ao criar thread");
      }
    }

    for(int i = 0; i < thread_desp; i++){ //cria as threads despachantes
      ids_desp[i] = (int*) malloc(sizeof(int));
      *ids_desp[i] = i;
      if(pthread_create(&despachantes[i], NULL, &func_despachante, (void*)ids_desp[i]) != 0){
          perror("Falha ao criar thread");
      }
    }

    for(int i = 0; i < thread_users; i++){ //da join em todas as threads usuario que já foram criadas
        if(pthread_join(usuario[i], NULL) != 0){
            perror("Falha ao dar join na thread");
        }
    }

    for(int i = 0; i < thread_desp; i++){ //da join em todas as threads despachantes que já foram criadas
        if(pthread_join(despachantes[i], NULL) != 0){
            perror("Falha ao dar join na thread");
        }
    }
    pthread_exit(NULL);
    return 0;
}
