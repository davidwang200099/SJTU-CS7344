#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "MyMPI.h"
#define MPI_DTYPE MPI_INT
#define dtype int

#define K 4
#define N 8

int max_pooling(dtype **a, int ks, int m, int n){
    int max = 0x80000000;
    for(int i=0;i<ks;i++){
        for(int j=0;j<ks;j++) 
            max = (max>a[i+m][j+n])?max:a[i+m][j+n];
    }
    return max;
}

int main(int argc,char **argv){
    int id,p,id1,id_c,id_r,p_group;
    int m,n;
    dtype **a,**c;
    dtype kernel[K][K];
    dtype *storage_a,*storage_c;
    MPI_Comm cart_comm;

    MPI_Init(&argc,&argv);
    MPI_Comm_size(MPI_COMM_WORLD,&p);

    int size[2]={0};
    MPI_Dims_create(p,2,size);

    
    int periodic[2]={0};
    MPI_Cart_create(MPI_COMM_WORLD,2,size,periodic,1,&cart_comm);
    MPI_Comm_rank(cart_comm,&id1);

    int dest_coord[2];
    MPI_Cart_coords(cart_comm,id1,2,dest_coord);

    MPI_Comm row_comm,col_comm;
    MPI_Comm_split(cart_comm,dest_coord[0],dest_coord[1],&row_comm);
    MPI_Comm_split(cart_comm,dest_coord[1],dest_coord[0],&col_comm);

    MPI_Comm_rank(row_comm,&id_r);
    MPI_Comm_rank(col_comm,&id_c);
    MPI_Comm_size(row_comm,&p_group);

    if(!id1){
        FILE *file = fopen(argv[1],"rb"); /*matrix*/
        fread(&m,sizeof(int),1,file);
        fread(&n,sizeof(int),1,file);

        if(m!=n) {printf("Matrix must be square!\n");MPI_Finalize();}
        storage_a  = malloc(m*m*sizeof(dtype));
        fread(storage_a,sizeof(int),m*m,file);
        fclose(file);

        file = fopen(argv[2],"rb");
        int kk;
        fread(&kk,sizeof(int),1,file);
        fread(&(kernel[0][0]),sizeof(dtype),K*K,file);
    }
    MPI_Bcast(&m,1,MPI_INT,0,cart_comm);
    MPI_Bcast(&(kernel[0][0]),K*K,MPI_DTYPE,0,cart_comm);
    if(id1) storage_a  = malloc(m*m*sizeof(dtype));
    MPI_Bcast(storage_a,m*m,MPI_DTYPE,0,cart_comm);

    a = malloc(m*PTR_SIZE);
    for(int i=0;i<m;i++) a[i] = storage_a + i*m;

    int mc = BLOCK_SIZE(id_c,p_group,m+1-K);
    int nc = BLOCK_SIZE(id_r,p_group,m+1-K);

    storage_c = malloc(mc*nc*sizeof(dtype));
    memset(storage_c,0,mc*nc*sizeof(dtype));
    c = malloc(mc*PTR_SIZE);
    for(int i=0;i<mc;i++) c[i] = storage_c + i*nc;

    int row_start = BLOCK_LOW(id_c,p_group,m+1-K);
    //int row_end = BLOCK_HIGH(id_c,p_group,m+1-K);
    int col_start = BLOCK_LOW(id_r,p_group,m+1-K);
    //int col_end = BLOCK_HIGH(id_c,p_group,m+1-K);

    // if(1) {
    //     printf("id %d is printing matrix......\n",id1);
    //     for(int i=0;i<m;i++){
    //         for(int j=0;j<m;j++) printf("\t%d",a[i][j]);
    //         printf("\n");
    //     }
    // }
    
    //printf("Calculating......\n");
    //printf("id=%d, row_start=(%d*%d)/%d=%d, col_start=(%d*%d)/%d=%d\n",id1,id_c,m+1-K,p_group,row_start,id_r,m+1-K,p_group,col_start);
    for(int i=0;i<mc;i++){
        for(int j=0;j<nc;j++){
            //c[i][j] = max_pooling(a,K,row_start+i,col_start+j);
            for(int x=0;x<K;x++){
                for(int y=0;y<K;y++){
                    //if(id1==2)
                    //printf("id=%d, c[%d][%d]+=kernel[%d][%d]*a[%d][%d](%d)\n",id1,i,j,x,y,row_start+i+x,col_start+j+y,a[row_start+i+x][col_start+j+y]);
                    c[i][j] += kernel[x][y]*a[row_start+i+x][col_start+j+y];
                }
            }
        }
    }
    
    MPI_Barrier(cart_comm);
    print_checkerboard_matrix(c,MPI_DTYPE,m+1-K,m+1-K,cart_comm);

    MPI_Finalize();
    return 0;
}