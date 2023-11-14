#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <pthread.h>
#include "chrono.h"
#include "heap.h"

#define THREAD_MIN 1
#define THREAD_MAX 8
#define MAX_HEAP_SIZE (1024 * 1024)

int n, k, nThreads, tamOutput;
float *input;
par_t *output;
par_t **heaps;
int* tamHeaps;

// Geração de números aleatórios
void geraNaleatorios(float v[], int n)
{
    for(int i = 0; i < n; i++)
    {
        // Gera dois aleatorios entre 0 e RAND_MAX
	    int a = rand();
	    int b = rand(); 
	
        // Junta esses dois e forma um numero para o vetor v
	    float elem = a * 100.0 + b;

        // inserir o valor v na posição p
	    v[i] = elem;
    }
}

// Função principal
void *acharKMenores(void *arg) 
{
    int threadNum = (int)arg;
    int indexPrimElem = threadNum * (n/nThreads);  // Cada thread vai ter n/nthreads elementos
    int indexUltElem;                              // A multiplicação é pra ver em qual dos blocos ela está

    // Checa se é a última thread
    if (threadNum == nThreads - 1)
        indexUltElem = n - 1;
    else
    {
        if (((threadNum + 1) * (n/nThreads)) < n)
            indexUltElem = (threadNum + 1) * (n/nThreads) - 1;
        else
            indexUltElem = n - 1;
    }

    // Vai inserindo na heap 
    for (int i = indexPrimElem; i <= indexUltElem; i++)
    {
        if (tamHeaps[threadNum] < k)
            insert(heaps[threadNum], &tamHeaps[threadNum], input[i], i);
        else
            decreaseMax(heaps[threadNum], k, input[i], i);          
    }

    pthread_exit(NULL);
    return NULL;
}

// Necessário pra usar o qsort
int compare(const void *a, const void *b) 
{
    const par_t *parA = (const par_t *)a;
    const par_t *parB = (const par_t *)b;

    if (parA->chave < parB->chave) return -1;
    if (parA->chave > parB->chave) return 1;
    
    // Retorna 0 se os elementos são iguais
    return 0;
}

// Verifica se a saída realmente tem os K menores
void verifyOutput(const float *Input, const par_t *Output, int nTotalElmts, int k)
{
    par_t* I = malloc(nTotalElmts * sizeof(par_t));
    int ok = 1;

    for (int i = 0; i < nTotalElmts; i++)
    {
        I[i].chave = Input[i];
        I[i].valor = i;
    }

    // Ordena I
    qsort(I, nTotalElmts, sizeof(par_t), compare);

    // Ordena Output -- Pra ficar mais rápido a verificação
    qsort((void*) Output, k, sizeof(par_t), compare);

    for (int i = 0; i < k; i++)
    {
        if (I[i].valor != Output[i].valor)
        {
            // Caso de elementos iguais em ordem oposta
            // Podia resolver isso usando um alg de ordenacao estavel
            if (I[i].chave != Output[i].chave)
            {
                ok--;
                break;
            }
        }   
    }
    
    if(ok)
       printf("\nOutput set verified correctly.\n");
    else
       printf("\nOutput set DID NOT compute correctly!!!\n");

    free(I);   
}

int main(int argc, char* argv[])
{                        
    double timeSeconds;
    int num;

    // Tratando a entrada
    if (argc != 4) 
    {
        printf("Forma de execução: %s <Quantidade de números aleatórios> <Quantidade de K menores> <Quantidade de threads>\n" , argv[0]); 
        exit(EXIT_FAILURE);
    } 
    else 
    {
        nThreads = atoi(argv[3]);

        if (nThreads < THREAD_MIN || nThreads > THREAD_MAX) 
        {
            printf("Forma de execução: %s <Quantidade de números aleatórios> <Quantidade de K menores> <Quantidade de threads>\n" , argv[0]); 
            printf("<Quantidade de threads> tem que ser entre %d e %d\n", THREAD_MIN, THREAD_MAX);
            exit(EXIT_FAILURE);
        }  

        n = atoi(argv[1]);
        k = atoi(argv[2]);
        
        if (k < 0 || k > n || k > MAX_HEAP_SIZE) 
        {
            printf("Forma de execução: %s <Quantidade de números aleatórios> <Quantidade de K menores> <Quantidade de threads>\n" , argv[0]); 
            printf("<Quantidade de K menores> tem que ser entre 1 e <Quantidade de números aleatórios>, e não pode ser maior que MAX_HEAP_SIZE: %d\n", MAX_HEAP_SIZE);
            exit(EXIT_FAILURE);
        }       
    }

    // Aloca espaços
    pthread_t threads[nThreads];
    input = malloc(n * sizeof(par_t));
    output = malloc(k * sizeof(par_t));
    heaps = malloc(nThreads * sizeof(par_t *));
    tamHeaps = malloc(nThreads * sizeof(int));

    // Randomiza a SEED
    srand(time(NULL));

    // Cria o vetor v
    geraNaleatorios(input, n);

    // Aloca espaço para cada heap
    for (int i = 0; i < nThreads; i++)
    {
        tamHeaps[i] = 0;
        heaps[i] = malloc(k * sizeof(par_t));
    }

    // Começa o cronômetro
    chronometer_t time;
    chrono_reset(&time);
    chrono_start(&time);
    
    // Cria as threads
    num = 0;
    for (int i = 0; i < nThreads; i++)
    {
        // Testa se criou as threads
        if (pthread_create(&threads[i], NULL, acharKMenores, (void*)num) != 0)
        {
            perror("Erro ao criar threads");
            exit(EXIT_FAILURE);
        }

        num++; 
    }   

    // Aguarda as threads terminarem a execução
    for (int i = 0; i < nThreads; i++)
    {
        // Testa se chegaram ao fim
        if (pthread_join(threads[i], NULL) != 0)
        {
            perror("Erro ao aguardar as threads\n");
            exit(EXIT_FAILURE);
        }
    }

    // Junta todas as heaps em uma só, montando a solução final
    tamOutput = 0;
    for (int i = 0; i < nThreads; i++)
    {
        for (int j = 0; j < k; j++)
        {
            if (tamOutput < k)
                insert(output, &tamOutput, heaps[i][j].chave, heaps[i][j].valor);
            else
                decreaseMax(output, tamOutput, heaps[i][j].chave, heaps[i][j].valor);
        }
    }

    // Para o cronometro
    chrono_stop(&time);

    // Printando tempo e MOPS
    chrono_reportTime(&time, "time: ");
    timeSeconds = (double) chrono_gettotal(&time) / ((double)1000 * 1000 * 1000); // NanoSeconds para Seconds
    printf("\nO algoritmo demorou: %lf ms\n", timeSeconds);
    printf("E a vazão foi de: %lf MOPS", (n/timeSeconds));  // MOPS

    // Verifica saída
    verifyOutput(input, output, n, k);

    // Da free em tudo
    for (int i = 0; i < nThreads; i++)
        free(heaps[i]);
    free(heaps);
    free(tamHeaps);

    free(input);
    free(output);

    return 0;
}