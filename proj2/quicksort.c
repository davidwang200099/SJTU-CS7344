#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define N 1000000

void quicksort(int *arr, int start, int end){
    
    if(start>=end-1) return;
    //printf("sort arr[%d:%d]...\n",start,end);
    int i=start,j=end-1,tmp;

    int pivot_rank = start+rand()%(end-start);

    tmp = arr[pivot_rank];arr[pivot_rank]=arr[start];arr[start]=tmp;

    int pivot = arr[start];
    while(i<j){
        while(arr[i]<=pivot&&i<end) i++;
        while(arr[j]>pivot) j--;
        if(i<j) {tmp=arr[i];arr[i]=arr[j];arr[j]=tmp;}
    }
    arr[start] = arr[j];
    arr[j] = pivot;

    #pragma omp parallel sections
    {
        #pragma omp section
        {
            quicksort(arr,start,j);
        }
        #pragma omp section
        {
            quicksort(arr,j+1,end);
        }
    }
    //#pragma omp task default(none) firstprivate(arr,start,end)
    //#pragma omp task default(none) firstprivate(arr,start,end)
}

int main(){
    int *arr = malloc(N*sizeof(int));
    
    FILE *file = fopen("random.txt","r");
    for(int i=0;i<N;i++) fscanf(file,"%d",arr+i);
    //double elapsed_time = -MPI_Wtime();
    clock_t start = clock();
    quicksort(arr,0,N);
    clock_t end = clock();
    //elapsed_time += MPI_Wtime();
    for(int i=0;i<N-1;i++) if(arr[i]>arr[i+1]) printf("Error!\n");
    //printf("\n");
    printf("Elapsed time: %lf\n",(double)(end-start)/CLOCKS_PER_SEC);
    free(arr);
    return 0;
}