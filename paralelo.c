#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"

#define MAX_ROWS 100
#define MAX_COLS 100

int main(int argc, char *argv[]) {
    int rank, size;
    int nl, nc, nls;
    long int matrix[MAX_ROWS][MAX_COLS];
    int i, j;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (argc < 4) {
        printf("Uso: mpirun -np <número_de_processos> ./nome_do_programa <número_de_linhas> <número_de_colunas> <número_de_linhas_por_processo>\n");
        MPI_Finalize();
        return 1;
    }

    nl = atoi(argv[1]);
    nc = atoi(argv[2]);
    nls = atoi(argv[3]);

    if (rank == 0) {
        for (i = 0; i < nl; i++) {
            for (j = 0; j < nc; j++) {
                matrix[i][j] = i * nc + j + 1; // Preenchendo a matriz com valores diferentes de zero
            }
        }
    }

    long int* sendbuf = NULL;

    if (rank == 0) {
        sendbuf = (long int*)malloc(nl * nc * sizeof(long int));
        int index = 0;
        for (i = 0; i < nl; i++) {
            for (j = 0; j < nc; j++) {
                sendbuf[index++] = matrix[i][j];
            }
        }
    }

    long int* recvbuf = (long int*)malloc(nls * nc * sizeof(long int));


    MPI_Scatter(sendbuf, nls * nc, MPI_LONG, recvbuf, nls * nc, MPI_LONG, 0, MPI_COMM_WORLD);

    printf("Processo %d:\n", rank);
    for (i = 0; i < nls; i++) {
        printf("Linha %d: ", i);
        for (j = 0; j < nc; j++) {
            printf("%ld ", recvbuf[i * nc + j]);
        }
        printf("\n");
    }

    if (rank == 0) {
        free(sendbuf);
    }
    free(recvbuf);

    MPI_Finalize();
    return 0;
}