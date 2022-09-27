#include <omp.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <cmath>
using namespace std;
#define N 1024000
#define MAX_E 10

int main(int argc, char **argv){

    FILE *file = fopen(argv[1],"r");
    if(!file) printf("Error!\n");
    vector<vector<int> > adjlist(N);
    int max_round = atoi(argv[2]);
    double epsilon = abs(atof(argv[3]));
    float pr_old[N];
    float pr_new[N];
    float delta=0.0;
    //double pr_new;
    for(int i=0;i<N;i++) pr_old[i]=1.0;
    
    int from, to;

    while(!feof(file)){
        fscanf(file,"%d%d",&from,&to);
        adjlist[to].push_back(from);
    }

    fclose(file);
    int r=0;
    while(true){
    //for(int r=0;r<max_round;r++){
        #pragma omp parallel for
        for(int i=0;i<N;i++){
            pr_new[i] = 0.0;
            for(int j=0;j<adjlist[i].size();j++) pr_new[i] += pr_old[adjlist[i][j]]/adjlist[i].size();
        }
        #pragma omp barrier
        #pragma omp parallel for
        for(int i=0;i<N;i++){
            #pragma omp critical
            {
                delta = (delta>abs(pr_new[i]-pr_old[i]))?delta:abs(pr_new[i]-pr_old[i]);
            }
            pr_old[i] = pr_new[i];
            //pr_new[i] = 0.0;
        }
        r++;
        if(delta<epsilon|| r>=max_round) break; 
    }

    file = fopen(argv[4],"w");
    for(int i=0;i<N;i++) fprintf(file,"%f\n",pr_old[i]);
    return 0;
}