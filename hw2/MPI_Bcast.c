#include <stdio.h>
#include <math.h>
#include <mpi.h>
#include <string.h>

int main(int argc, char **argv){

    int* a;
    int id;
    int p;
    int len = 0;
    double mpi_barrier_time;
    

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    MPI_Comm_size(MPI_COMM_WORLD, &p);
    if (argc != 2){
        if (!id) printf ("Command line argument lost.\n");
        MPI_Finalize(); exit (1);
    }
    len = atoi(argv[1]);
    a =(int*)calloc(len, sizeof(int));

    MPI_Barrier(MPI_COMM_WORLD);
    mpi_barrier_time = -MPI_Wtime();
    MPI_Bcast(a, len, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Barrier(MPI_COMM_WORLD);
    if(!id){
        mpi_barrier_time += MPI_Wtime();
        printf ("MPI_Bcast elapsed time: %f\n", mpi_barrier_time);
    }

    free(a);
    MPI_Finalize();
    return 0;
}