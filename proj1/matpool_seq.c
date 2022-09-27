#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
int K=4;
#define dtype int

int max_pooling(dtype **a, int ks, int m, int n){
    int max = 0x80000000;
    for(int i=0;i<ks;i++){
        for(int j=0;j<ks;j++) 
            max = (max>a[i+m][j+n])?max:a[i+m][j+n];
    }
    return max;
}

int main(int argc, char **argv){
    int kernel[K][K];
    srand(time(NULL));


    FILE *file = fopen(argv[1],"rb");

    int m,n;
    fread(&m,sizeof(int),1,file);
    fread(&n,sizeof(int),1,file);
    

    int *storage = malloc(m*m*sizeof(int));
    fread(storage,sizeof(int),m*m,file);
    int **a = malloc(m*sizeof(void *));
    for(int i=0;i<m;i++) a[i] = storage + i*m;
    fclose(file);
    
    //printf("Read matrix finished...\n");
    // file = fopen(argv[2],"rb");
    // int ks;
    // fread(&ks,sizeof(int),1,file);
    // fread(&(kernel[0][0]),sizeof(int),K*K,file);
    // fclose(file);

    //printf("Write kernel finished.\n");
    int *storage_c = malloc((m-K+1)*(m-K+1)*sizeof(int));
    memset(storage_c,0,sizeof(int)*(m-K+1)*(m-K+1));
    int **c = malloc((m-K+1)*sizeof(void *));
    for(int i=0;i<m-K+1;i++) c[i] = storage_c + (m-K+1)*i;

    printf("Read finished. Calculating...\n");
    for(int i=0;i<m-K+1;i++){
        for(int j=0;j<m-K+1;j++){
            c[i][j] = max_pooling(a,K,i,j);
            // for(int x=0;x<K;x++){
            //     for(int y=0;y<K;y++){
            //         c[i][j] += kernel[x][y] * a[i+x][j+y];
            //     }
            // }
        }
    }
    printf("Printing results to %s...\n",argv[2]);
    file = fopen(argv[2],"w");
    for(int i=0;i<m-K+1;i++){
        for(int j=0;j<m-K+1;j++) fprintf(file,"%6d ",c[i][j]);
        fprintf(file,"\n");
    }
    fprintf(file,"\n");
    fclose(file);
    printf("Print finished...\n");
    return 0;
}