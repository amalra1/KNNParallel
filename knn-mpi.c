#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "mpi.h"
// #include "heap.h"

typedef struct
{
    float chave;
    int valor;
} par_t;

#define parent(pos) ((pos - 1) / 2)

void drawHeapTree(par_t* heap, int size, int nLevels)
{
    int offset = 0;
    int space = (int)pow(2, nLevels - 1);

    int nElements = 1;
    for (int level = 0; level < nLevels; level++)
    {
        // Printa todos os elementos desse nível
        for (int i = offset; i < size && i < (offset + nElements); i++)
            printf("[chave: %.0f, índice: %d]", heap[i].chave, heap[i].valor);
        printf("\n");

        offset += nElements;
        space = nElements - 1;
        nElements *= 2;
    }
}

// Troca dois elementos
void swap(par_t *a, par_t *b)
{
    par_t temp = *a;
    *a = *b;
    *b = temp;
}

// Função interna do processo de Heap
void maxHeapify(par_t* heap, int tam, int i)
{
    while (1)
    {
        int largest = i;
        int left = 2 * i + 1;
        int right = 2 * i + 2;

        if (left < tam && heap[left].chave > heap[largest].chave)
            largest = left;

        if (right < tam && heap[right].chave > heap[largest].chave)
            largest = right;

        if (largest != i)
        {
            swap(&heap[i], &heap[largest]); // Using the swap function
            i = largest;
        }
        else
            break;
    }
}

// Função interna do processo de Heap
void heapifyUp(par_t* heap, int pos)
{
    float chave = heap[pos].chave;
    float valor = heap[pos].valor;

    while (pos > 0 && chave > heap[parent(pos)].chave)
    {
        heap[pos].chave = heap[parent(pos)].chave;
        heap[pos].valor = heap[parent(pos)].valor;
        pos = parent(pos);
    }

    heap[pos].chave = chave;
    heap[pos].valor = valor;
}

// Insere um elemento na Heap mantendo suas propriedades
void insert(par_t* heap, int *tam, float chave, int valor)
{
    *tam += 1;
    int last = *tam - 1;

    heap[last].chave = chave;
    heap[last].valor = valor;

    heapifyUp(heap, last);
}

// Verifica se um vetor é uma heap
int isMaxHeap(par_t* heap, int tam)
{
    for (int i = 1; i < tam; i++)
        if (heap[i].chave <= heap[parent(i)].chave)
            continue;
        else
        {
            printf("\nbroke at [%d]=%f\n", i, heap[i].chave);
            printf("father at [%d]=%f\n", parent(i), heap[parent(i)].chave);
            return 0;
        }
    return 1;
}

// Substitui a cabeça da Heap por um valor e mantém as propriedades
void decreaseMax(par_t* heap, int tam, float chave, int valor)
{
    // Heap vazia
    if (tam == 0)
        return;

    if (heap[0].chave > chave)
    {
        heap[0].chave = chave;
        heap[0].valor = valor;

        maxHeapify(heap, tam, 0);
    }
}

// Funcao do professor
void verificaKNN(float *Q, int nq, float *P, int n, int D, int k, int *R)
{
    // Note que R tem nq linhas por k colunas, para qualquer tamanho de k (colunas)
    // entao é linearizado para acesso como um VETOR
    printf( " ------------ VERIFICA KNN --------------- ");
    for(int linha = 0; linha < nq; linha++)
    {
        printf( "knn[%d]: ", linha);

        for(int coluna = 0; coluna < k; coluna++)
            printf( "%d ", R[ linha*k+coluna ]);
        printf( "\n" );
    }
}

// Geração de números aleatórios
void geraConjuntoDeDados(float **C, int nc, int D)
{
    for(int i = 0; i < nc; i++)
    {
        for(int j = 0; j < D; j++)
        {
            // Gera dois aleatorios 
            // int a = rand() % 20;
            // int b = rand() % 20; 
        
            // Junta esses dois e forma um numero para a coordenada
            // float elem = a * 100.0 + b;

            float elem = rand() % 10;

            // Inserir o valor v na posição p
            C[i][j] = elem;
        }
    }
}

// Calcula os k vizinhos mais próximos de cada ponto de Q com os pontos de P e armazena em R
void calculaDistancias(float** Q, long int nq, float** P, long int npp, long int d, long int k, par_t** R)
{
    float dist = 0.0;
    par_t* heap = malloc(k * sizeof(par_t));
    int tamHeap = 0;

    // Para cada ponto de Q -> Qi
    for (int i = 0; i < nq; i++)
    {
        // Para cada ponto de P -> Pl
        for (int l = 0; l < npp; l++)
        {
            // Para cada dimensão dos dois pontos
            // Vai calculando a distância entre os dois
            for (int m = 0; m < d; m++){
                dist = dist + (Q[i][m] - P[l][m])*(Q[i][m] - P[l][m]);
            }
            
            // Vai enchendo a heap
            if (tamHeap < k)
                insert(heap, &tamHeap, dist, l);
            else
                decreaseMax(heap, tamHeap, dist, l);
            dist = 0.0;
            
        }

        // Copia os valores da heap pro R
        for (int l = 0; l < k; l++){
            R[i][l] = heap[l];
            heap[l].chave = 0;
            heap[l].valor = 0;
        }
        
        // // Zera a heap
        // for (int l = 0; l < k; k++){
        //     heap[l].chave = 0;
        //     heap[l].valor = 0;
        // }
        tamHeap = 0;
    }

    free(heap);
}

int main(int argc, char* argv[])
{
    // Tratando a entrada
    if (argc != 5) 
    {
        printf("Forma de execução: mpirun <Número de processos> %s <Número de pontos em Q> <Número de pontos em P> <Número de dimensoes dos pontos> <Tamanho de cada conjunto de vizinho>\n" , argv[0]); 
        exit(EXIT_FAILURE);
    } 

    long int nq, npp, d, k;
    nq = atoi(argv[1]);
    npp = atoi(argv[2]);
    d = atoi(argv[3]);
    k = atoi(argv[4]);
    
    // Cria e aloca matrizes
    float **Q;
    Q = malloc(nq * sizeof(float*));
    for (long int i = 0; i < nq; i++)
        Q[i] = malloc(d * sizeof(float)); 

    float **P;
    P = malloc(npp * sizeof(float*));
    for (long int i = 0; i < npp; i++)
        P[i] = malloc(d * sizeof(float));

    par_t **R;
    R = malloc(nq * sizeof(par_t*));
    for (long int i = 0; i < nq; i++)
        R[i] = malloc(k * sizeof(par_t));

    // Randomiza a SEED
    srand(time(NULL));

    // Preenche matrizes
    geraConjuntoDeDados(Q, nq, d);
    geraConjuntoDeDados(P, npp, d);

    // NO DECREASE MAX A CHAVE É A DISTANCIA CALCULADA ENTRE O Qi e Pi
    // O VALOR É O ÍNDICE DO Pi
    // A SAÍDA TEM QUE IMPRIMIR OS ÍNDICES DA HEAP FINAL --> VALORES

    calculaDistancias(Q, nq, P, npp, d, k, R);

    // PRINTS DE TESTE
    for (int i = 0; i < nq; i++)
    {
        for (int j = 0; j < d; j++)
            printf("[%.0f] ", Q[i][j]);
        printf("\n");
    }
    printf("\n");

    for (int i = 0; i < npp; i++)
    {
        for (int j = 0; j < d; j++)
            printf("[%.0f] ", P[i][j]);
        printf("\n");
    }

    for (int i = 0; i < nq; i++)
    {
        for (int j = 0; j < k; j++)
            printf("[%.0f] ", R[i][j].chave);
        printf("\n");
    }
    
    // Libera memória
    for (long int i = 0; i < npp; i++)
       free(P[i]);
    free(P);
    for (long int i = 0; i < nq; i++)
       free(Q[i]);
    free(Q);
    for (long int i = 0; i < nq; i++)
       free(R[i]);
    free(R);

    return 0;
}