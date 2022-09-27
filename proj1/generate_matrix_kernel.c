#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
int M  = 1024;
int N  = 1024;
int K = 4;

int main(){
    int r, c;
    int cnt=0;
    FILE* pFile;  
    srand(time(NULL));
    pFile = fopen("matrix_a" , "wb");
    //printf("Generation completed!\n");
    fwrite(&M, sizeof(int), 1, pFile);
    fwrite(&N, sizeof(int), 1, pFile);
    for (r= 0; r<M; r++){
        for(c = 0; c<N; c++){
            int elem = rand()%1000;
            if(!elem) cnt++;
            fwrite(&elem, sizeof(int), 1, pFile);
        }
    }
    fclose(pFile);

    pFile = fopen("matrix_b" , "wb");
    //printf("Generation completed!\n");
    fwrite(&M, sizeof(int), 1, pFile);
    fwrite(&N, sizeof(int), 1, pFile);
    for (r= 0; r<M; r++){
        for(c = 0; c<N; c++){
            int elem = rand()%1000;
            if(elem==0) cnt++;
            fwrite(&elem, sizeof(int), 1, pFile);
        }
    }
    fclose(pFile);

    int kernel[K][K];
    for(int i=0;i<K;i++) for(int j=0;j<K;j++) kernel[i][j] = rand()%1000;
    pFile = fopen("kernel","wb");
    fwrite(&K,sizeof(int),1,pFile);
    fwrite(&(kernel[0][0]),sizeof(int),K*K,pFile);
    fclose(pFile);

    //printf("Write finished...\n");
    // pFile = fopen("matrix_a","r");
    // fread(&M,sizeof(int),1,pFile);
    // fread(&N,sizeof(int),1,pFile);
    // int *storage_a = malloc(M*M*sizeof(int));
    // fread(storage_a,sizeof(int),M*M,pFile);
    // fclose(pFile);

    // pFile = fopen("matrix_b","r");
    // fread(&M,sizeof(int),1,pFile);
    // fread(&N,sizeof(int),1,pFile);
    // int *storage_b = malloc(M*M*sizeof(int));
    // fread(storage_b,sizeof(int),M*M,pFile);
    // fclose(pFile);

    // int **a = malloc(M*sizeof(void *));
    // int **b = malloc(M*sizeof(void *));
    // for(int i=0;i<M;i++) a[i] = storage_a + i*M;
    // for(int i=0;i<M;i++) b[i] = storage_b + i*M;

    // //printf("There are %d 0's in a and b...\n",cnt);
    // //printf("Allocating C...\n");
    // int **C = malloc(M*sizeof(void *));
    // for(int i=0;i<M;i++) {C[i] = (int *)malloc(M*sizeof(int));memset(C[i],0,M*sizeof(int));}

    // for(int i=0;i<M;i++){
    //     for(int j=0;j<M;j++){
    //         for(int k=0;k<M;k++) C[i][j] += a[i][k] * b[k][j];
    //         //printf("C[%d][%d]=%d\n",i,j,C[i][j]);
    //     }
    // }
    
    // pFile = fopen("results_seq.txt","w");
    // for(int i=0;i<M;i++){
    //     for(int j=0;j<M;j++) fprintf(pFile,"%6d ",C[i][j]);
    //     fprintf(pFile,"\n");
    // }
    // fprintf(pFile,"\n");
    return 0;
}