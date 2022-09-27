#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "string.h"
#include "MyMPI.h"
#define MPI_DTYPE MPI_INT
#define dtype int

int My_Allgather(const void *sendbuf, int sendcount, MPI_Datatype sendtype,
                                 void *recvbuf, int recvcount,
                                 MPI_Datatype recvtype, MPI_Comm comm){
    int id,p;
    MPI_Comm_rank(MPI_COMM_WORLD,&id);
    MPI_Comm_size(MPI_COMM_WORLD,&p);
    MPI_Status status;
    for(int i=0;i<p;i++){
        if(id==i) {
            for(int dest=0;dest<p;dest++){
                if(id==dest) continue;
                MPI_Send(sendbuf,sendcount,sendtype,dest,0,MPI_COMM_WORLD);
            }
        }else MPI_Recv(recvbuf+i*recvcount*get_size(sendtype),recvcount,MPI_DTYPE,i,0,MPI_COMM_WORLD,&status);
    }
    memcpy(recvbuf+id*recvcount*get_size(sendtype),sendbuf,recvcount*get_size(sendtype));
    return 0;
}

int main(int argc,char **argv){
    
    int id,p;
    dtype *solution;
    dtype *global_solution;
    double elapsed_time;

    MPI_Init(&argc,&argv);
    MPI_Comm_rank(MPI_COMM_WORLD,&id);
    MPI_Comm_size(MPI_COMM_WORLD,&p);
    
    int N = atoi(argv[1]);
    //srand(time(NULL));
    solution = (dtype *)malloc(N*sizeof(dtype));
    for(int i=0;i<N;i++) solution[i]=id*N+i;
    
    global_solution = (dtype *)malloc(N*p*sizeof(dtype));

    MPI_Barrier(MPI_COMM_WORLD);
    elapsed_time = -MPI_Wtime();
    MPI_Allgather(solution,N,MPI_DTYPE,global_solution,N,MPI_DTYPE,MPI_COMM_WORLD);
    //My_Allgather(solution,N,MPI_DTYPE,global_solution,N,MPI_DTYPE,MPI_COMM_WORLD);
    elapsed_time += MPI_Wtime();

    if(!id) {
        printf("elapsed_time=%f\n",elapsed_time);
        for(int i=0;i<p;i++){
            for(int j=0;j<N;j++) printf("%d ",global_solution[i*N+j]);
            printf("\n");
        }
        
    }
    
    MPI_Finalize();
    return 0;
}