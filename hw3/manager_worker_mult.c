#include <stdio.h>
#include <mpi.h>
#include <string.h>
#include "MyMPI.h"
typedef int dtype;
#define mpitype MPI_INT
#define SIMPLE_MSG 0
#define MATRIX_MSG 1
#define VECTOR_MSG 2

void read_whole_matrix(char  *s,void ***subs,void **storage,MPI_Datatype dtype,int *m,int *n){
    void **lptr;
    void *rptr;
    int i;
    int datum_size =get_size(dtype);
    FILE *infileptr = fopen (s, "r");
    if (infileptr == NULL) *m = 0;
    else {
        fread (m, sizeof(int), 1, infileptr);
        fread (n, sizeof(int), 1, infileptr);
    }
    if (!(*m)) MPI_Abort (MPI_COMM_WORLD, OPEN_FILE_ERROR);
    *storage = (void *) malloc(*m * *n * datum_size);
        
    *subs = (void **) malloc(*m * PTR_SIZE);

    lptr = &(*subs[0]);
    rptr = *storage;
    for (i = 0; i < *m; i++) {
        *(lptr++)= (void *) rptr;
        rptr += *n * datum_size;
    }

    fread (*storage, datum_size, *m * *n, infileptr);
    fclose (infileptr);
}

void read_whole_vector(
    char *s,
    void **v,
    MPI_Datatype dtype,
    int *n)
{
    
    int i;
    FILE *infileptr = fopen(s, "r");
    if(infileptr==NULL) *n = 0;
    else fread(n, sizeof(int), 1, infileptr);
    int datum_size = get_size(dtype);

    if(!*n) {
        printf("Error: Cannot open vector file\n");
        fflush(stdout);
        MPI_Finalize();
        exit(-1);
    }
    *v = malloc(*n*datum_size);
    fread(*v, datum_size, *n, infileptr);
    fclose(infileptr);
}

void manager(int argc, char *argv[], int id, int p){
    dtype **a;
    dtype *b;
    dtype *c;
    dtype *storage;
    int i, j;
    int m, n;
    int nn; /* Elements in vector-b*/
    MPI_Request pending;
    MPI_Status status;
    int datum_size = get_size(mpitype);
    read_whole_matrix(argv[1], (void***)&a, (void**)&storage, mpitype, &m, &n);
    printf("Matrix:\n");
    print_submatrix((void**)a, mpitype, m, n);
    read_whole_vector(argv[2], (void**)&b, mpitype, &nn);
    printf("Vector:\n");
    print_subvector((void*)b, mpitype, nn);
    putchar('\n');

    for(i = 1; i<p; i++){
        int rows_and_n[2] = {BLOCK_SIZE(i-1, p-1, m), n};
        MPI_Isend(rows_and_n, 2, MPI_INT, i, SIMPLE_MSG, MPI_COMM_WORLD, &pending);
    }
    MPI_Wait(&pending, &status);

    for (i = 1; i<p; i++){
        MPI_Isend(b, nn, mpitype, i, VECTOR_MSG, MPI_COMM_WORLD, &pending);
    }
    MPI_Wait(&pending, &status);

    for (i = 1; i<p; i++){
        int low_value = BLOCK_LOW(i-1, p-1, m);
        int size = BLOCK_SIZE(i-1, p-1, m);
        MPI_Isend(a[low_value], size*n, mpitype, i, MATRIX_MSG, MPI_COMM_WORLD, &pending);
    }
    MPI_Wait(&pending, &status);

    c = (dtype*)malloc(n*datum_size);
    for (i = 1; i<p; i++){
        MPI_Irecv(c+BLOCK_LOW(i-1, p-1, m), BLOCK_SIZE(i-1, p-1, m), mpitype, i, VECTOR_MSG, MPI_COMM_WORLD, &pending);
    }
    MPI_Wait(&pending, &status);
    printf("Result:\n");
    print_subvector((void*)c, mpitype, m);
    putchar('\n');
}

void worker(){
    dtype **subs;
    dtype *storage;
    dtype *b;
    dtype *c_block;
    int rows, n;
    int i, j;
    void **lptr;
    void *rptr;
    MPI_Request pending;
    MPI_Status status;
    int datum_size = get_size(mpitype);
    int *rows_and_n = (int*)malloc(2*sizeof(int));

    MPI_Irecv(rows_and_n, 2, MPI_INT, 0, SIMPLE_MSG, MPI_COMM_WORLD, &pending);
    MPI_Wait(&pending, &status);
    rows = rows_and_n[0];
    n = rows_and_n[1];

    b = (dtype*)malloc(n*datum_size);
    storage = (dtype*)malloc(rows*n*datum_size);
    subs = (dtype**)malloc(rows*sizeof(dtype*));
    c_block = (dtype*)malloc(rows*datum_size);

    MPI_Irecv(b, n, mpitype, 0, VECTOR_MSG, MPI_COMM_WORLD, &pending);
    MPI_Wait(&pending, &status);

    MPI_Irecv(storage, rows*n, mpitype, 0, MATRIX_MSG, MPI_COMM_WORLD, &pending);
    MPI_Wait(&pending, &status);
    

    lptr = (void**)subs;
    rptr = (void*)storage;
    for(i = 0; i<rows; i++){
        *(lptr++) = rptr;
        rptr += n*datum_size;
    }

    for(i = 0; i<rows; i++){
        c_block[i] = 0;
        for(j = 0; j<n; j++){
            c_block[i]+= subs[i][j]*b[j];
        }
    }
    MPI_Isend(c_block, rows, mpitype, 0, VECTOR_MSG, MPI_COMM_WORLD, &pending);
}

int main(int argc, char **argv){
    int id;
    int p;
    MPI_Comm worker_comm;
    void manager(int, char **, int, int);
    void worker();

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    MPI_Comm_size(MPI_COMM_WORLD, &p);
    if (argc!=3){
        if(!id){
            printf("Command line argument lost.\n");
        }
        MPI_Finalize();
        return 0;
    }
    if (p<2){
        printf("Program needs at least two processes.\n");
        MPI_Finalize();
    }
    if(!id){
        MPI_Comm_split(MPI_COMM_WORLD,MPI_UNDEFINED,id,&worker_comm);
        manager(argc, argv, id, p);
    }else {
        MPI_Comm_split(MPI_COMM_WORLD,0,id,&worker_comm);
        worker();
    }
    MPI_Finalize();
    return 0;
}

