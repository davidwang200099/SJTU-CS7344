#include <bits/stdc++.h>
using namespace std;
#define N 1024000
#define MAX_E 10

int main(){
    FILE *file = fopen("graph.txt","w");

    //int degree[N]={0}
    vector<vector<int> > adjlist(N);
    

    for(int i=0;i<N;i++){
        int d=(rand()%MAX_E)+1;
        if(true){
            for(int j=0;j<d;j++){
                int to = rand()%N;
                //auto iter = find(adjlist[i].begin(),adjlist[i].end(),to);
                if(i!=to&&find(adjlist[i].begin(),adjlist[i].end(),to)==adjlist[i].end()){
                    //degree[i]++;
                    adjlist[i].push_back(to);
                }
            }
        }
        //if(i%1000==0) printf("Finished 1000 nodes...\n");
    }

    for(int i=0;i<N;i++){
        for(int j=0;j<adjlist[i].size();j++) fprintf(file,"%d %d\n",i,adjlist[i][j]);
    }
    fclose(file);
    return 0;
}