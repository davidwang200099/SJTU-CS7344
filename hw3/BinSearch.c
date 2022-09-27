#include <stdio.h>
#include <stdlib.h>
#include <values.h>
#include <mpi.h>
#include "MyMPI.h"
#define MIN(a,b)  ((a)<(b)?(a):(b))

int main(int argc, char *argv[]){
    int n;
    int id, p;
    int i, j, k;
    float* proba;
    int **root;
    float **cost;
    float *storage_cost;
    int *storage_root;
    float **aux_cost;
    int **aux_root;
    double elapsed_time;

    void alloc_matrix(void ***, int, int, int);
    void print_root (int **, int, int);

    MPI_Init(&argc, &argv);
    
    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    MPI_Comm_size(MPI_COMM_WORLD, &p);
    
    if(!id){
        scanf("%d", &n);
    }
    MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
    proba = (float*)malloc(n*sizeof(float));
    if(!id){
        for(i = 0; i<n; i++)scanf("%f", &proba[i]);
    }
    MPI_Bcast(proba, n, MPI_FLOAT, 0, MPI_COMM_WORLD);
    elapsed_time = -MPI_Wtime();

    alloc_matrix((void ***)&cost, n+1, n+1, sizeof(float));
    alloc_matrix((void ***)&root, n+1, n+1, sizeof(int));

    
    storage_cost = (float*)malloc(((n+1)*(n+2)/2)*sizeof(float));
    storage_root = (int*)malloc(((n+1)*(n+2)/2)*sizeof(int));
    aux_cost = (float**)malloc((n+1)*sizeof(float*));
    aux_root = (int**)malloc((n+1)*sizeof(int*));
    for(i = 0; i<n+1; i++){
        storage_root[i] = i;
        storage_cost[i] = 0.0;
    }

    float **tmp_ac = aux_cost;
    float *tmp_sc = storage_cost;
    int **tmp_ar = aux_root;
    int *tmp_sr = storage_root;
    for(i = 0; i<n+1; i++){
        *(tmp_ac++) =  tmp_sc;
        *(tmp_ar++) = tmp_sr;
        tmp_sc += n+1-i;
        tmp_sr += n+1-i;
    }


    int iter = 1;
    while(iter<=n){
        int newn = n+1-iter;
        int newp = MIN(newn, p);
        if(id+1<=newn){
            int start = BLOCK_LOW(id, newp, newn);
            int block_size = BLOCK_SIZE(id, newp, newn);
            for (i = 0; i<block_size; i++){
                int I = iter;
                int J = start+i;
                float rcost = 0.0;
                float bestcost = MAXFLOAT;
                int bestroot;
                for(j = 0; j<iter; j++){
                    rcost = aux_cost[j][J] + aux_cost[I-1-j][J+1+j];
                    for(k = J; k<I+J; k++)rcost += proba[k];
                    if(rcost<bestcost){
                        bestcost = rcost;
                        bestroot = j+J;
                    }
                }
                aux_cost[I][J] = bestcost;
                aux_root[I][J] = bestroot;
            }

        }
        for(int i = 0; i<newp; i++){
            MPI_Bcast(aux_cost[iter]+BLOCK_LOW(i, newp, newn), BLOCK_SIZE(i, newp, newn), MPI_FLOAT, i, MPI_COMM_WORLD);
            MPI_Bcast(aux_root[iter]+BLOCK_LOW(i, newp, newn), BLOCK_SIZE(i, newp, newn), MPI_INT, i, MPI_COMM_WORLD);
        }
        iter++;
    }

    if(!id){
        int q = 0;
        for(k= 0; k<n+1; k++){
            for(i = 0; i<n+1-k; i++){
                j = i+k;
                root[i][j] = storage_root[q];
                cost[i][j] = storage_cost[q];
                q++;
            }
        }

        print_root(root, 0, n-1);
        elapsed_time+=MPI_Wtime();
        printf("Total elapsed time: %10.6f\n", elapsed_time);
    }
    fflush(stdout);
    MPI_Finalize();
    return 0;
}

void print_root(int **root, int low, int high){
    printf("Root of tree spanning %d-%d is %d\n",
    low, high, root[low][high+1]);

    if (low<root[low][high+1]-1)
        print_root(root, low, root[low][high+1]-1);
    
    if (root[low][high+1]<high-1)
        print_root(root, root[low][high+1]+1, high);
}

void alloc_matrix (void ***a, int m, int n, int size){
    int i;
    void *storage;
    storage = (void *)malloc(m*n*size);
    *a = (void**) malloc(m*sizeof(void *));
    for (i = 0; i<m; i++){
        (*a)[i] = storage + i*n*size;
    }
}