#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "mpi.h"
#include "chrono.h"
#include <pthread.h>

typedef struct
{
    float chave;
    int valor;
} par_t;

long int nq, npp, d, k, limite, nt;
int nproc, processId;
float *Q, *P;
par_t *R;
float *heapChave, *QLocal, *heapChaveLocal;
int *heapValor, *heapValorLocal;

par_t** heaps;
int* tamHeaps;

pthread_t threads[10];

#define parent(pos) ((pos - 1) / 2)

chronometer_t chrono;

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
void verificaKNN(float *Q, int nq, float *P, int n, int D, int k, par_t *R)
{
    // Note que R tem nq linhas por k colunas, para qualquer tamanho de k (colunas)
    // entao é linearizado para acesso como um VETOR
    printf( " ------------ VERIFICA KNN --------------- \n");
    for(int linha = 0; linha < nq; linha++)
    {
        printf( "knn[%d]: ", linha);

        for(int coluna = 0; coluna < k; coluna++)
            printf( "%f ", R[(linha*k) + coluna].chave);
        printf( "\n" );
    }
}

// Geração de números aleatórios
void geraConjuntoDeDados(float *C, int nc, int D)
{
    for(int i = 0; i < nc; i++)
    {
        for(int j = 0; j < D; j++)
        {
            // Gera dois aleatorios 
            int a = rand();
            int b = rand(); 
        
            // Junta esses dois e forma um numero para a coordenada
            float elem = a * 100.0 + b;

            // Inserir o valor v na posição p
            C[(i*D) + j] = elem;
        }
    }
}

void calculaDistanciasSeq(float* Q, long int nq, float* P, long int npp, long int d, long int k, par_t* R, int iniQ)
{
    float dist = 0.0;
    par_t* heap = malloc(k * sizeof(par_t));
    int tamHeap = 0;

    // Para cada ponto de Q -> Qi
    for (int i = iniQ; i < nq; i++)
    {
        // Para cada ponto de P -> Pl
        for (int l = 0; l < npp; l++)
        {
            // Para cada dimensão dos dois pontos
            // Vai calculando a distância entre os dois
            for (int m = 0; m < d; m++){
                dist = dist + (Q[(i * d) + m] - P[(l * d) + m])*(Q[(i * d) + m] - P[(l * d) + m]);
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
            R[(i * k) + l] = heap[l];
            heap[l].chave = 0;
            heap[l].valor = 0;
        }
 
        tamHeap = 0;
    }

    free(heap);
}

int min(int a, int b)
{
    if (a < b)
        return a;
    else
        return b;
}

// Calcula distancias mas em threads
void* calculaDistanciasThreads(void *args)
{
    float dist = 0.0;
    
    int threadId = *(int *)args;
    int nElem = limite/nt;
    int prim = threadId * nElem;
    int ult = min((threadId + 1) * nElem, limite) - 1;
    if (threadId == nt - 1)
        ult = limite - 1;

    //printf("processId = %d, threadId = %d, nElms = %d, prim %d, ult %d\n\n", processId, threadId, nElem, prim, ult);
    // Para cada ponto de Q -> Qi
    for (int i = prim; i <= ult; i++)
    {
        // Para cada ponto de P -> Pl
        for (int l = 0; l < npp; l++)
        {
            // Para cada dimensão dos dois pontos
            // Vai calculando a distância entre os dois
            for (int m = 0; m < d; m++){
                dist = dist + (QLocal[(i * d) + m] - P[(l * d) + m])*(QLocal[(i * d) + m] - P[(l * d) + m]);
            }
            
            // Vai enchendo a heap
            if (tamHeaps[i] < k)
                insert(heaps[i], &tamHeaps[i], dist, l);
            else
                decreaseMax(heaps[i], tamHeaps[i], dist, l);
            dist = 0.0;
            
        }

        //printf("Thread[%d] = ", threadId);
        // for (int l = 0; l < k; l++){
        //     printf("%.0f ", heaps[i][l].chave);
        // }
        // printf("\n");

        // Copia os valores da heap pro R
        for (int l = 0; l < k; l++){
            heapChave[(i * k) + l] = heaps[i][l].chave;
            heapValor[(i * k) + l] = heaps[i][l].valor;
            heaps[i][l].chave = 0;
            heaps[i][l].valor = 0;
        }

        tamHeaps[i] = 0;
    }
}


// // Calcula os k vizinhos mais próximos de cada ponto de Q com os pontos de P e armazena em R
// void calculaDistancias(float* Q, long int nq, float* P, long int npp, long int d, long int k, par_t* R, int limite, MPI_Comm comm, int processId)
// {
//     float dist = 0.0;
    
//     // MPI_Scatter(Q, d * limite, MPI_FLOAT, QLocal, d * limite, MPI_FLOAT, 0, comm);
//     // MPI_Bcast(P, npp * d, MPI_FLOAT, 0, comm);

//     // Para cada ponto de Q -> Qi
//     for (int i = 0; i < limite; i++)
//     {
//         // Para cada ponto de P -> Pl
//         for (int l = 0; l < npp; l++)
//         {
//             // Para cada dimensão dos dois pontos
//             // Vai calculando a distância entre os dois
//             for (int m = 0; m < d; m++){
//                 dist = dist + (QLocal[(i * d) + m] - P[(l * d) + m])*(QLocal[(i * d) + m] - P[(l * d) + m]);
//             }
            
//             // Vai enchendo a heap
//             if (tamHeaps[i] < k)
//                 insert(heaps[i], &tamHeaps[i], dist, l);
//             else
//                 decreaseMax(heaps[i], tamHeaps[i], dist, l);
//             dist = 0.0;
            
//         }

//         // Copia os valores da heap pro R
//         for (int l = 0; l < k; l++){
//             heapChave[(i * k) + l] = heaps[i][l].chave;
//             heapValor[(i * k) + l] = heaps[i][l].valor;
//             heaps[i][l].chave = 0;
//             heaps[i][l].valor = 0;
//         }

//         tamHeaps[i] = 0;
//     }

//     free(heap);

//     MPI_Gather(heapChave, k * limite, MPI_FLOAT, heapChaveLocal, k * limite, MPI_FLOAT, 0, comm);
//     MPI_Gather(heapValor, k * limite, MPI_INT, heapValorLocal, k * limite, MPI_INT, 0, comm);

//     if(processId == 0){
//         for(int i = 0; i < nq; i++){
//             for(int j = 0; j < k; j++){
//                 R[(i*k) + j].chave = heapChaveLocal[(i * k) + j];
//                 R[(i*k) + j].valor = heapValorLocal[(i * k) + j];
//             }
//         }
//     }
// }

int main(int argc, char* argv[])
{
    // Tratando a entrada
    if (argc != 6) 
    {
        printf("Forma de execução: mpirun <Número de processos> %s <Número de pontos em Q> <Número de pontos em P> <Número de dimensoes dos pontos> <Tamanho de cada conjunto de vizinho> <Número de threads>\n" , argv[0]); 
        exit(EXIT_FAILURE);
    } 

    // Atribuição das variáveis dos parâmetros
    nq = atoi(argv[1]);
    npp = atoi(argv[2]);
    d = atoi(argv[3]);
    k = atoi(argv[4]);
    nt = atoi(argv[5]);

    // Inicialização do MPI
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &nproc);
    MPI_Comm_rank(MPI_COMM_WORLD, &processId);

    // Define um limite para cada processo em Q
    limite = (int) nq / nproc;
    
    // Cria e aloca matrizes
    Q = malloc(nq * d * sizeof(float*));
    P = malloc(npp * d * sizeof(float*));
    R = malloc(nq * k * sizeof(par_t*));

    // Alocando as matrizes para o Scatter
    heapChave = malloc((k * limite) * sizeof(float));
    heapValor = malloc (k * limite * sizeof(int));
    QLocal = malloc(d * limite * sizeof(float));
    heapChaveLocal = malloc(k * nq * sizeof(float));
    heapValorLocal = malloc(k * nq * sizeof(int));

    // Aloca as heaps
    heaps = malloc(limite * sizeof(par_t *));
    tamHeaps = malloc(limite * sizeof(int));
    for (int i = 0; i < limite; i++)
    {
        tamHeaps[i] = 0;
        heaps[i] = malloc(k * sizeof(par_t));
    }

    // Randomiza a SEED
    srand(time(NULL));

    // Preenche matrizes e começa a contagem de tempo
    if(processId == 0)
    {
        geraConjuntoDeDados(Q, nq, d);
        geraConjuntoDeDados(P, npp, d);
        chrono_reset(&chrono);
        chrono_start(&chrono);
    }

    // Divide a matriz Q e manda para os nodos
    MPI_Scatter(Q, d * limite, MPI_FLOAT, QLocal, d * limite, MPI_FLOAT, 0, MPI_COMM_WORLD);

    // Manda a matriz P inteira para cada nodo
    MPI_Bcast(P, npp * d, MPI_FLOAT, 0, MPI_COMM_WORLD);

    // Cria vetor com id das threads
    int *threadIds = malloc(nt * sizeof(int));

    // Thread 0 = sequencial
    threadIds[0] = 0;
    calculaDistanciasThreads(threadIds);

    // Começa as threads se passado um número > 1
    for (int i = 1; i < nt; i++)
    {
        threadIds[i] = i;
        pthread_create(&threads[i], NULL, &calculaDistanciasThreads, threadIds + i);
    }

    // Termina as threads
    for (int i = 1; i < nt; i++)
        pthread_join(threads[i], NULL);

    MPI_Gather(heapChave, limite * k, MPI_FLOAT, heapChaveLocal, limite * k, MPI_FLOAT, 0, MPI_COMM_WORLD);
    MPI_Gather(heapValor, limite * k, MPI_INT, heapValorLocal, limite * k, MPI_INT, 0, MPI_COMM_WORLD);

    if(processId == 0){
        for(int i = 0; i < nq; i++){
            for(int j = 0; j < k; j++){
                R[(i*k) + j].chave = heapChaveLocal[(i * k) + j];
                R[(i*k) + j].valor = heapValorLocal[(i * k) + j];
            }
        }
    }

    // Para os processos restantes se sobrar algo da divisão do Q entre os processos
    if (nq % nproc)
        calculaDistanciasSeq(Q, nq, P, npp, d, k, R, nq - (nq % nproc));

    // Impressão do tempo e realização dos testes
    if(processId == 0)
    {
        chrono_stop(&chrono);
        double total_time_in_seconds = (double)chrono_gettotal(&chrono) / ((double)1000 * 1000 * 1000);
        printf("total_time_in_seconds: %lf s\n", total_time_in_seconds);
        double MBPS = (((double) nq * npp * d) / ((double)total_time_in_seconds*1000*1000));
        printf("Throughput: %lf MB/s\n", MBPS);
        //verificaKNN(Q, nq, P, npp, d, k, R); // DESCOMENTAR NA ENTREGA
    }

    // PRINTS DE TESTE
    // if(processId == 0)
    //{
    //     for (int i = 0; i < nq; i++)
    //     {
    //         for (int j = 0; j < d; j++)
    //             printf("[%.0f] ", Q[(i*d) + j]);
    //         printf("\n");
    //     }
    //     printf("\n");

    //     for (int i = 0; i < npp; i++)
    //     {
    //         for (int j = 0; j < d; j++)
    //             printf("[%.0f] ", P[(i*d) + j]);
    //         printf("\n");
    //     }
    //     printf("\n");

    //     for (int i = 0; i < nq; i++)
    //     {
    //         for (int j = 0; j < k; j++)
    //             printf("[%.0f] ", R[(i*k) + j].chave);
    //         printf("\n");
    //     }
    // }
    
    // Libera memória
    free(P);
    free(Q);
    free(R);
    for (int i = 0; i < limite; i++)
        free(heaps[i]);
    free(heaps);
    free(tamHeaps);

    free(QLocal);
    free(heapChave);
    free(heapValor);
    free(heapChaveLocal);
    free(heapValorLocal);

    // Finaliza o MPI
    MPI_Finalize();

    return 0;
}