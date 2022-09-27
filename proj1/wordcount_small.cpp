#include <mpi.h>
#include <cstdio>
#include <cstdlib>
#include <dirent.h>
#include <unordered_map>
#include <map>
#include <cstring>
using namespace std;
#define MAX_WORD_LEN 256
#define MAX_LINE_LEN 1024
#define MAX_NAME_LEN 256
typedef struct 
{
    char word[MAX_WORD_LEN];
    int cnt;
} PAIR;

typedef enum {READY_MSG, NUMKEY_MSG, WORDDICT_MSG,NAMELEN_MSG, FILENAME_MSG} msg_t;

void countWord(unordered_map<string,int> &map, char *buffer, FILE *file=NULL){
    int len=strlen(buffer);
    for(int i=0;i<len;i++) if(isupper(buffer[i])) buffer[i] = buffer[i] - 'A' + 'a';
    for(int i=0;i<len;i++) if(!islower(buffer[i])) buffer[i] = 0;
    int i=0;
    int id;
    MPI_Comm_rank(MPI_COMM_WORLD,&id);
    while(i<len){
        while(buffer[i]==0) i++;
        if(i>=len) break;
        //if(id==1) printf("In %d, buffer[%d..]:%s\n",id,i,buffer+i);
        map[string(buffer+i)]++;
        //printf("%s\n",buffer+i);
        //if(file) fprintf(file,"%s\n",buffer+i);
        while(islower(buffer[i])) i++;
    }
    //printf("Exit!\n");
}

void manager(char *dirname){
    DIR *streamp = opendir(dirname);
    if(!streamp) {
        printf("Open dir %s error!",dirname);
        MPI_Finalize();
    }
    struct dirent *dep;
    MPI_Status status;
    int src,tag;
    int terminated = 0;
    int buffer;
    int p;
    int lines=0;
    int len;
    MPI_Comm_size(MPI_COMM_WORLD,&p);
    int target_id = 1;
    PAIR **pair = (PAIR **)malloc((p-1)*sizeof(void *));
    int *dict_size = (int *)malloc((p-1)*sizeof(int));
    memset(dict_size,0,(p-1)*sizeof(int));
    
    do{
        //printf("Receiving msg from workers...\n");
        MPI_Recv(&buffer,1,MPI_INT,MPI_ANY_SOURCE,MPI_ANY_TAG,MPI_COMM_WORLD,&status);
        src = status.MPI_SOURCE;
        tag = status.MPI_TAG;

        if(tag==NUMKEY_MSG){
            dict_size[src-1] = buffer;
            pair[src-1] = (PAIR *)malloc(buffer*sizeof(PAIR));
            //printf("There are %d words in process %d\n",buffer,src);
            terminated++;
            //MPI_Recv(pair[src-1],buffer*sizeof(PAIR),MPI_CHAR,src,WORDDICT_MSG,MPI_COMM_WORLD,&status);
            //printf("Word dictionary from %d received...\n",src);
        }
        //if(tag==WORDDICT_MSG) MPI_Recv(pair[src-1],buffer*sizeof(PAIR),MPI_CHAR,src,WORDDICT_MSG,MPI_COMM_WORLD,&status);
        if(tag==READY_MSG){
            do{
                dep = readdir(streamp);
            }while(dep && dep->d_name[0]=='.');

            if(dep){
                len=strlen(dep->d_name);
                MPI_Send(&len,1,MPI_INT,src,NAMELEN_MSG,MPI_COMM_WORLD);
                MPI_Send(dep->d_name,strlen(dep->d_name),MPI_CHAR,src,FILENAME_MSG,MPI_COMM_WORLD);
            }else{
                len=0;
                MPI_Send(&len,1,MPI_INT,src,NAMELEN_MSG,MPI_COMM_WORLD);
                
            }
        }
    }while(terminated<p-1);
    MPI_Bcast(&buffer,1,MPI_INT,0,MPI_COMM_WORLD);
    for(int i=0;i<p-1;i++){
        //printf("Receiving %d*%d=%d bytes from Process %d...\n",dict_size[i],sizeof(PAIR),dict_size[i]*sizeof(PAIR),i+1);
        if(dict_size[i])
            MPI_Recv(pair[i],dict_size[i]*sizeof(PAIR),MPI_CHAR,i+1,WORDDICT_MSG,MPI_COMM_WORLD,&status);
    }
    //printf("Receive word dict finished.\n");
    map<string,int> map;
    for(int i=0;i<p-1;i++){
        for(int j=0;j<dict_size[i];j++){
            //cout<<string(pair[i][j].word)<<endl;
            map[string(pair[i][j].word)]+=pair[i][j].cnt;
        }
    }
    for(auto i=map.begin();i!=map.end();i++){
        printf("%s:%d\n",i->first.data(),i->second);
    }
}
void worker(char *dirname){
    char filename[MAX_NAME_LEN]={0};
    char fullname[MAX_LINE_LEN]={0};
    char buffer[MAX_LINE_LEN]={0};
    int len_dirname = strlen(dirname);
    int namelen;
    
    
    MPI_Status status;
    int id;
    unordered_map<string,int> map;
    MPI_Comm_rank(MPI_COMM_WORLD,&id);
    //FILE *fptr = open(,)
    MPI_Send(&id,1,MPI_INT,0,READY_MSG,MPI_COMM_WORLD);
    int file_cnt=0;
    while(1){
        MPI_Recv(&namelen,1,MPI_INT,0,NAMELEN_MSG,MPI_COMM_WORLD,&status);
        if(namelen==0) break;
        file_cnt++;
        MPI_Recv(filename,namelen,MPI_CHAR,0,FILENAME_MSG,MPI_COMM_WORLD,&status);
        strcpy(fullname,dirname);
        if(fullname[len_dirname-1]!='/') {fullname[len_dirname]='/';strcpy(fullname+len_dirname+1,filename);}
        else{strcpy(fullname+len_dirname,filename);}
        FILE *file = fopen(fullname,"r");

        //if(!file) printf("Open [%s] failed!\n",fullname);
        //else printf("Process %d opens [%s]...\n",id,fullname);
        //printf("Process %d is working...\n",id);
        while(!feof(file)){
            fgets(buffer,MAX_LINE_LEN,file);
            if(strlen(buffer)==0) continue;
            countWord(map,buffer);
            //fscanf(file,"%s",buffer);
            //if(buffer[0]==0) continue;
            //map[string(buffer)]++;
        }
        fclose(file);
        MPI_Send(&id,1,MPI_INT,0,READY_MSG,MPI_COMM_WORLD);
    }
    //printf("Process %d analized %d files.\n",id,file_cnt);
    int num_keys = map.size();

    MPI_Send(&num_keys,1,MPI_INT,0,NUMKEY_MSG,MPI_COMM_WORLD);
    PAIR *pairs = (PAIR *)malloc(num_keys*sizeof(PAIR));
    int i=0;
    for(auto iter=map.begin();iter!=map.end();iter++,i++){
        strcpy(pairs[i].word,iter->first.data());
        pairs[i].cnt = iter->second;
    }
    int tmp;
    MPI_Bcast(&tmp,1,MPI_INT,0,MPI_COMM_WORLD);
    //printf("Process %d sending dictionary:%dx%d=%d bytes...\n",id,num_keys,sizeof(PAIR),num_keys*sizeof(PAIR));
    MPI_Send(pairs,num_keys*sizeof(PAIR),MPI_CHAR,0,WORDDICT_MSG,MPI_COMM_WORLD);
    //printf("%s finished...\n",filename);
}



int main(int argc, char **argv){
    int id,p;

    MPI_Init(&argc,&argv);
    MPI_Comm_size(MPI_COMM_WORLD,&p);
    MPI_Comm_rank(MPI_COMM_WORLD,&id);

    if(!id){
        manager(argv[1]);
    }else{
        worker(argv[1]);
    }

    MPI_Finalize();
    return 0;
}