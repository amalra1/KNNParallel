#include <stdio.h>
#include <math.h>
#include "heap.h"

// Pedro Amaral e Pedro Willian



// Printa elementos da Heap
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

