#include <stdio.h>
#include <math.h>
#include <mpi.h>
#include <string.h>
#include "MyMPI.h"


typedef int dtype;
#define mpitype MPI_INT
int main(int argc, char *argv[]){
    dtype **a;          /* The first factor, a matrix */
    dtype *b;           /* The second factor, a vector */
    dtype *local_c;     /* The local product, a vector */
    dtype *global_c;    /* The global product, a vector */
    dtype *storage;  /* This process's portion of 'a' */
    int m, n;       /* Rows and Columns in the matrix */
    int nprime;     /* Elements in vector */
    int id;
    int p;
    int i, j;
    int low_value, block_size;
    double elapsed_time;

    MPI_Init(&argc, &argv);
    MPI_Barrier(MPI_COMM_WORLD);
    elapsed_time = -MPI_Wtime();
    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    MPI_Comm_size(MPI_COMM_WORLD, &p);
    if (argc != 3){
        if (!id) printf ("Command line argument lost.\n", argv[0]);
        MPI_Finalize(); exit (1);
    }
    read_col_striped_matrix(argv[1], (void ***)&a, (void **)&storage, mpitype, &m, &n, MPI_COMM_WORLD);
    if(!id)printf("Matrix:\n");
    print_col_striped_matrix((void **)a, mpitype, m, n, MPI_COMM_WORLD);
    read_replicated_vector(argv[2], (void **)&b, mpitype, &nprime, MPI_COMM_WORLD);
    if(!id)printf("Vector:\n");
    print_replicated_vector(b, mpitype, nprime, MPI_COMM_WORLD);
    low_value = BLOCK_LOW(id, p, n);
    block_size = BLOCK_SIZE(id, p, n);
    local_c = (dtype*) malloc(n*sizeof(dtype));
    for(i = 0; i<m; i++){
        local_c[i] = 0;
        for(j = 0; j<block_size; j++){
            local_c[i]+= a[i][j]*b[low_value+j];
        }
    }
    MPI_Allreduce(local_c, global_c, m, mpitype, MPI_SUM, MPI_COMM_WORLD);
    if(!id)printf("Result:\n");
    print_replicated_vector(global_c, mpitype, m, MPI_COMM_WORLD);
    elapsed_time+=MPI_Wtime();
    if(!id){
        printf("Total elapsed time: %f\n", elapsed_time);
    }
    fflush(stdout);
    MPI_Finalize();
    return 0;


}