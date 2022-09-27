#include "mpi.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
int isPrime(int n){
    int h=sqrt(n);
    for(int i=2;i<=h;i++) if(n%i==0) return 0;
    return 1;
}

int process(int start, int end){
    printf("From %d to %d\n",start,end);
    int sum=0;
    if(start%2==0) start++;
    if(end%2==0) end--;
    int flag=0;
    for(int i=start;i<=end;i+=2){
        if(isPrime(i)){
            if(flag==0) flag=1;
            else{
                sum++;
            }
        }else{
            flag=0;
        }
    }
    return sum;
}

int main(int argc, char **argv){
    int global_solutions;
    int i=-1;
    int id=-1;
    int p=-1;
    int solutions=0;
    
    
    MPI_Init(&argc,&argv);
    MPI_Comm_rank(MPI_COMM_WORLD,&id);
    MPI_Comm_size(MPI_COMM_WORLD,&p);
    
    int l=(1000000-3)/p;

    int start=3;
    int end=start+l;
    if(end%2==0) end-=1;
    for(int i=0;i<id;i++) {
        start = end;
        end += l;
        if(end%2==0) end-=1;
        if(i==p-1) end=999999;
    }

    solutions+=process(start,end);
    MPI_Reduce(&solutions,&global_solutions,1,MPI_INT,MPI_SUM,0,MPI_COMM_WORLD);
    MPI_Finalize();
    if(id==0) printf("global solution is %d\n",global_solutions);
    return 0;
}