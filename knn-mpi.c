#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "mpi.h"
#include "heap.h"

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
            int a = rand() % 20;
            int b = rand() % 20; 
        
            // Junta esses dois e forma um numero para a coordenada
            float elem = a * 100.0 + b;

            // Inserir o valor v na posição p
            C[i][j] = elem;
        }
    }
}

// Calcula os k vizinhos mais próximos de cada ponto de Q com os pontos de P e armazena em R
void calculaDistancias(float** Q, long int nq, float** P, long int npp, long int d, long int k, float** R)
{
    float dist;
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
            for (int m = 0; m < d; m++)
                dist = dist + (Q[i][m]*Q[i][m] - P[l][m]*P[l][m]);
            
            // Vai enchendo a heap
            for (int x = 0; x < k; x++)
            {
                if (tamHeap < k)
                    insert(heap, &tamHeap, dist, l);
                else
                    decreaseMax(heap, tamHeap, dist, l);
            }
        }

        // Copia os valores da heap pro R
        for (int l = 0; l < k; k++)
            R[i][l] = heap[l];
        
        // Zera a heap
        for (int l = 0; l < k; k++)
            heap[l] = 0;
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

    float **R;
    R = malloc(nq * sizeof(float*));
    for (long int i = 0; i < nq; i++)
        R[i] = malloc(k * sizeof(float));

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
            printf("[%.0f] ", R[i][j]);
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