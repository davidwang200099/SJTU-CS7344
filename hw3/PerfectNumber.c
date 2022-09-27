#include <stdio.h>
#include <math.h>
#include <mpi.h>
#include <string.h>
#include <stdlib.h>
int isPrime(long long num){
    long long i=2;
    if (num<2)return 0;
    while(i<=(long long)sqrt((long double)(num))){
        if ((num%i)==0){
            return 0;
        }
        i++;
    }
    return 1;
}

void manager(int p, int num_wanted){
    int count = 0;
    int i;
    long long seed = 4;
    long long perfect_num;
    long long stop_signal = 0;
    MPI_Request pending;
    MPI_Status status;
    long long *perfect = (long long*) malloc(num_wanted*sizeof(long long));
    while(count!=num_wanted){
        for(i = 1; i<p; i++){
            MPI_Isend(&seed, 1, MPI_LONG_LONG_INT, i, 0, MPI_COMM_WORLD, &pending);
            seed*=2;
        }
        MPI_Wait(&pending, &status);

        for(i = 1; i<p; i++){
            MPI_Recv(&perfect_num, 1, MPI_LONG_LONG_INT, i, 0, MPI_COMM_WORLD, &status);
            /* if the result is not a valid perfect number or we have already get enough perfect numbers, just ignore */
            if(perfect_num!=0&&count!=num_wanted){
                perfect[count++] = perfect_num;
            }
        }
    }
    for (i = 1; i<p; i++){
        MPI_Isend(&stop_signal, 1, MPI_LONG_LONG_INT, i, 0, MPI_COMM_WORLD, &pending);
    }
    printf("The first %d perfect numbers are:\n", num_wanted);
    for (i = 0; i<num_wanted; i++){
        printf("%lld ", perfect[i]);
    }
    putchar('\n');
    MPI_Wait(&pending, &status);
}

void worker(int root){
    long long seed;
    long long perfect_num;
    MPI_Request pending;
    MPI_Status status;
    while(1){
        MPI_Irecv(&seed, 1, MPI_LONG_LONG_INT, root, 0, MPI_COMM_WORLD, &pending);
        MPI_Wait(&pending, &status);
        if(seed!=0){
            if(isPrime(seed-1)){
                perfect_num = (seed-1)*(seed/2);
            }else perfect_num = 0;

            MPI_Send(&perfect_num, 1, MPI_LONG_LONG_INT, root, 0, MPI_COMM_WORLD);
        }else{
            break;
        }

    }
}

void seqSolution(int num_wanted){
    int count = 0;
    long long seed = 4;
    int i;
    long long *perfect = (long long*) malloc(num_wanted*sizeof(long long));
    int isPrime(long long);
    while (count!=num_wanted){
        if(isPrime(seed-1)){
            perfect[count++] = (seed-1)*(seed/2);
        }
        seed*=2;
    }
    printf("The first %d perfect numbers are:\n", num_wanted);
    for(i = 0; i<num_wanted; i++){
        printf("%lld ", perfect[i]);
    }
    putchar('\n');
}

int main(int argc, char *argv[]){
    int id;
    int p;
    int i, j;
    double elapsed_time;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    MPI_Comm_size(MPI_COMM_WORLD, &p);

    if(argc<2) {
        printf("Command line argument lost.\n");
        MPI_Finalize();
        return 0;
    }
    int num_wanted = atoi(argv[1]);

    if(p==1){
         seqSolution(num_wanted);
    }else{
        if(!id){
            manager(p, num_wanted);
        }else {
            worker(0);
        }
    }
    MPI_Finalize();
    return 0;
}