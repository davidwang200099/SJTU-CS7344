#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "MyMPI.h"
#define MPI_DTYPE MPI_INT
#define dtype int

int main(int argc,char **argv){
    int id,id1,id_r,id_c,p,p_cart;
    dtype **a=0,**b=0,**c=0;
    dtype *storage_a,*storage_b,*storage_c;
    int m1,n1,m2,n2,n;
    
    MPI_Status status;
    
    MPI_Init(&argc,&argv);
    MPI_Comm_size(MPI_COMM_WORLD,&p);
    MPI_Comm_rank(MPI_COMM_WORLD,&id);

    
    int *size = malloc(2*sizeof(int));
    size[0] = size[1] =0;
    MPI_Dims_create(p,2,size);

    MPI_Comm cart_comm;

    int periodic[2]={0};
    MPI_Cart_create(MPI_COMM_WORLD,2,size,periodic,1,&cart_comm);
    MPI_Comm_rank(cart_comm,&id1);

    int dest_coord[2];
    MPI_Cart_coords(cart_comm,id,2,dest_coord);

    MPI_Comm row_comm,col_comm;
    MPI_Comm_split(cart_comm,dest_coord[0],dest_coord[1],&row_comm);
    MPI_Comm_split(cart_comm,dest_coord[1],dest_coord[0],&col_comm);

    MPI_Comm_rank(row_comm,&id_r);
    MPI_Comm_rank(col_comm,&id_c);
    MPI_Comm_size(row_comm,&p_cart);

    //printf("id %d, id_r %d, id_c %d\n",id,id_r,id_c);

    read_row_striped_matrix(argv[1],&a,&storage_a,MPI_DTYPE,&m1,&n1,col_comm);
    read_col_striped_matrix(argv[2],&b,&storage_b,MPI_DTYPE,&m2,&n2,row_comm);
    
    
    n = n1;
    int n_group = sqrt(p);
    int n_part = n/n_group;
    
    storage_c = malloc(n_part*n_part*sizeof(dtype));
    memset(storage_c,0,n_part*n_part*sizeof(dtype));
    c = malloc(n_part*PTR_SIZE);
    for(int i=0;i<n_part;i++) c[i] = storage_c+i*n_part;
    
    for(int i=0;i<n_part;i++){
        for(int j=0;j<n_part;j++){
            for(int k=0;k<n;k++) c[i][j] += a[i][k]*b[k][j];
        }
    }

    MPI_Barrier(MPI_COMM_WORLD);
    print_checkerboard_matrix(c,MPI_DTYPE,m1,n1,cart_comm);

    MPI_Finalize();
    return 0;
}