#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
int M  = 1024;
int N  = 1024;

int main(int argc, char **argv){
    int m,n;
    FILE* pFile; 
    
    pFile = fopen(argv[1],"rb");
    fread(&M,sizeof(int),1,pFile);
    fread(&N,sizeof(int),1,pFile);
    int *storage_a = malloc(M*M*sizeof(int));
    fread(storage_a,sizeof(int),M*M,pFile);
    fclose(pFile);

    pFile = fopen(argv[2],"rb");
    fread(&M,sizeof(int),1,pFile);
    fread(&N,sizeof(int),1,pFile);
    int *storage_b = malloc(M*M*sizeof(int));
    fread(storage_b,sizeof(int),M*M,pFile);
    fclose(pFile);

    printf("Read finished. Calculating...\n");

    int **a = malloc(M*sizeof(void *));
    int **b = malloc(M*sizeof(void *));
    for(int i=0;i<M;i++) a[i] = storage_a + i*M;
    for(int i=0;i<M;i++) b[i] = storage_b + i*M;

    int **C = malloc(M*sizeof(void *));
    for(int i=0;i<M;i++) {C[i] = (int *)malloc(M*sizeof(int));memset(C[i],0,M*sizeof(int));}

    for(int i=0;i<M;i++){
        for(int j=0;j<M;j++){
            for(int k=0;k<M;k++) C[i][j] += a[i][k] * b[k][j];
            //printf("C[%d][%d]=%d\n",i,j,C[i][j]);
        }
    }
    
    printf("Printing results to %s...\n",argv[3]);
    pFile = fopen(argv[3],"w");
    for(int i=0;i<M;i++){
        for(int j=0;j<M;j++) fprintf(pFile,"%6d ",C[i][j]);
        fprintf(pFile,"\n");
    }
    fprintf(pFile,"\n");
    printf("Print finished...\n");
    return 0;
}