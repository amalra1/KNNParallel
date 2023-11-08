// Pedro Amaral e Pedro William

// Estrutura de chave e valor para cada elemento da Heap
typedef struct
{
    float chave;
    int valor;
} par_t;

// #define parent(pos) ( pos/2 ) // SE nao usar posicao 0
#define parent(pos) ((pos - 1) / 2)

// Printa elementos da Heap
void drawHeapTree(par_t* heap, int size, int nLevels);

// Troca dois elementos
void swap(par_t *a, par_t *b);

// Função interna do processo de Heap
void maxHeapify(par_t* heap, int tam, int i);

// Função interna do processo de Heap
void heapifyUp(par_t* heap, int pos);

// Insere um elemento na Heap mantendo suas propriedades
void insert(par_t* heap, int *tam, float chave, int valor);

// Verifica se um vetor é uma heap
int isMaxHeap(par_t* heap, int tam);

// Substitui a cabeça da Heap por um valor e mantém as propriedades
void decreaseMax(par_t* heap, int tam, float chave, int valor);