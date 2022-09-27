#include "mpi.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
int isPrime(int n){
    int h=sqrt(n);
    for(int i=2;i<=h;i++) if(n%i==0) return 0;
    return 1;
}

int main(int argc, char **argv){
    char * global_solutions;
    int i=-1;
    int id=-1;
    int p=-1;
    int n=-1;
    int *seed;
    int ii=0;
    char * solutions;
    int global_count=0;

    MPI_Init(&argc,&argv);
    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Comm_rank(MPI_COMM_WORLD,&id);
    MPI_Comm_size(MPI_COMM_WORLD,&p);

    n = atoi(argv[1]);
    int sqrt_n = (int)(sqrt((double)(n)));
    if(argc != 2){
        printf("Command line argument lost.\n");
        MPI_Finalize();
        exit(-1);
    }

    if(p>(int)(sqrt((double)(n)))){
        if(!id) printf("Too many processors allocated.\n");
        MPI_Finalize();
        exit(-1);
    }

    seed = (int *)malloc((int)(sqrt((double)(n))));
    solutions = (char *)malloc(n+1);
    global_solutions = (char *)malloc(n+1);

    memset(solutions,0,n+1);
    memset(global_solutions,0,n+1);
    

    for(int i=2;i<=sqrt_n;i++){
        if(isPrime(i)) seed[ii++] = i;
    }

    for(int i=id;i<ii;i+=p){
        int base = seed[i];
        for(int j=base<<1;j<=n;j+=base) solutions[j]=1;
    }

    MPI_Reduce(solutions,global_solutions,n+1,MPI_CHAR,MPI_LOR,0,MPI_COMM_WORLD);

    if(!id){
        for(int i=2;i<=n;i++) if(!global_solutions[i]) global_count++;
        printf("There are %d primes less than or equal to %d\n",global_count,n);
    }

    MPI_Finalize();
    return 0;
    
}