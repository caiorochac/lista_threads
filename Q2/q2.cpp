#include <iostream>
#include <list>
#include <stack> // pilha para usar na DFS
#include <vector>
#include <pthread.h>

using namespace std;

struct Grafo{
	int V;
	vector <list<int>> adj;
};

Grafo grafo;

void criarGrafo(int V){
	grafo.V = V;
	grafo.adj.resize(V);
}

void* dfs(void* thread_id){
	int tid = *((int*) thread_id);
	int v = tid;
	
	stack<int> pilha;
	bool visitados[grafo.V], pilha_rec[grafo.V];

	// inicializa visitados e pilha_rec com false
	for(int i = 0; i < grafo.V; i++)
		visitados[i] = pilha_rec[i] = false;

	// faz uma DFS
	while(true){
		
		bool achou_vizinho = false;
		list<int>::iterator it;

		if(!visitados[v]){
			pilha.push(v);
			visitados[v] = pilha_rec[v] = true;
		}

		for(it = grafo.adj[v].begin(); it != grafo.adj[v].end(); it++){
			// se o vizinho já está na pilha é porque existe ciclo
			if(pilha_rec[*it]){
				cout << "Thread " << tid << " achou um ciclo\n";
				pthread_exit(NULL);
			}
				
			else if(!visitados[*it]){
				// se não está na pilha e não foi visitado, indica que achou
				achou_vizinho = true;
				break;
			}
		}

		if(!achou_vizinho){
			pilha_rec[pilha.top()] = false; // marca que saiu da pilha
			pilha.pop(); // remove da pilha
			if(pilha.empty()){
				cout << "Thread " << tid << " nao achou um ciclo\n";
				pthread_exit(NULL);
			}
			v = pilha.top();
		}
		else
			v = *it;
	}
}

int main(int argc, char *argv[]){
	int nVertices;
	int nArestas;
	int v1,v2;
	int rc;
	cin >> nArestas;
	cin >> nVertices;
	pthread_t threads[nVertices];
	int *ids[nVertices];  
	criarGrafo(nVertices);
	
	for (int i = 0; i < nArestas; i++){
		cin >> v1 >> v2;
		grafo.adj[v1].push_back(v2); 
	}

	for(int i = 0; i < nVertices; i++){
		ids[i] = new int;
		*ids[i] = i;
		rc = pthread_create(&threads[i], NULL, dfs, (void*) ids[i]);      
		if (rc){         
			printf("ERRO; codigo de retorno eh %d\n", rc);         
			exit(-1);      
		}
	}

	for (int i = 0; i < nVertices; i++)
		pthread_join(threads[i], NULL);
	
	pthread_exit(NULL);

	return 0;
}
