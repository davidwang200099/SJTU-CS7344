#include "mpi.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

int My_Bcast(void *buffer, int count, MPI_Datatype datatype, int root, MPI_Comm comm){
    int id;
    int p;
    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    MPI_Comm_size(MPI_COMM_WORLD, &p);
    if(id==root){
        int i = 0;
        for(i=0; i<p; i++){
            if (i==id)continue;
            MPI_Send(buffer, count, datatype, i, 0, comm);
        }
        return 1;
    }
    MPI_Recv(buffer, count, datatype, root, 0, comm, MPI_STATUS_IGNORE);
    return 1;
}

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
    My_Bcast(a, len, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Barrier(MPI_COMM_WORLD);
    if(!id){
        mpi_barrier_time += MPI_Wtime();
        printf ("MPI_Bcast elapsed time: %f\n", mpi_barrier_time);
    }

    free(a);
    MPI_Finalize();
    return 0;
}