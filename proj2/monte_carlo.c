#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
int main(int argc, char **argv){
    int count=0;
    int i;
    double x,y;
    int samples=atoi(argv[1]);

    
    #pragma omp parallel num_threads(8)
    {
        //printf("hello, world from thread %d!\n",omp_get_thread_num());
        srand(omp_get_thread_num()+time(NULL));
        #pragma omp for private(i,x,y) reduction(+:count)
        for(i=0;i<samples;i++){
            //printf("hello, thread %d!\n",omp_get_thread_num());
            x=(double)rand()/RAND_MAX;
            y=(double)rand()/RAND_MAX;
            if(x*x+y*y<=1) count++;
        }
        
    }
    double pi = 4*(double) count/(samples);
    printf("After sampling %d times, pi=%f\n",samples,pi);
    return 0;
}