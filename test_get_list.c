#include<stdio.h>
#include<string.h>
#include<stdlib.h>

typedef struct path_list{
  char** list;
  int len;
}path_list;

path_list* get_path_list(char* path){
  //first get list
  char** dl=(char**)malloc(sizeof(char*));//directory list from path
  int dl_len=0;//len of dl
  int len = strlen(path);//len of path
  int markl=-1;
  int i=0;
  char** pt=NULL;
  if(path[0]=='/'){
    while(!(pt=(char**)realloc(dl, sizeof(char*)*(dl_len+1))));
    dl = pt;
    while(!(*(dl+dl_len) = (char*)malloc(sizeof(char)*(i-markl+1))));
    strncpy(*(dl+dl_len), path+markl+1, i-markl);
    *(*(dl+dl_len)+i-markl)='\0';
    dl_len++;
    markl++;
    i++;
  }
  
  for(;i<len;i++){
    if(path[i]=='/'){
      while(!(pt=(char**)realloc(dl, sizeof(char*)*(dl_len+1))));
      dl = pt;
      while(!(*(dl+dl_len) = (char*)malloc(sizeof(char)*(i-markl))));
      strncpy(*(dl+dl_len), path+markl+1, i-markl-1);
      *(*(dl+dl_len)+i-markl-1)='\0';
      markl = i;
      dl_len++;
    }
  }
  if(path[len-1]!='/'){
    while(!(pt=(char**)realloc(dl, sizeof(char*)*(dl_len+1))));
    dl = pt;
    while(!(*(dl+dl_len) = (char*)malloc(sizeof(char)*(i-markl))));
    strncpy(*(dl+dl_len), path+markl+1, i-markl-1);
    *(*(dl+dl_len)+i-markl-1)='\0';
    //printf("%s\t%s\t%d\t%d\n",path,*(dl+dl_len),strlen(*(dl+dl_len)),i-markl-1);
    dl_len++;
  }
  path_list* pl=(path_list*)malloc(sizeof(struct path_list));
  pl->len=dl_len;
  pl->list = dl;
  return pl;
}

int main(){
  char path[100] = "/hello";
  /*
  //first get list
  char** dl;//directory list from path
  int dl_len=0;//len of dl
  int len = strlen(path);//len of path
  int markl=0;
  int i=0;
  if(path[0]=='/'){
    //markl++;
    i++;
  }
  
  for(;i<len;i++){
    if(path[i]=='/'){
      char** pt;
      while(!(pt=(char**)realloc(dl, sizeof(char*)*(dl_len+1))));
      dl = pt;
      while(!(*(dl+dl_len) = (char*)malloc(sizeof(char)*(i-markl-1))));
      strncpy(*(dl+dl_len), path+markl+1, i-markl-1);
      markl = i;
      dl_len++;
    }
  }
  if(path[len-1]!='/'){
    char** pt;
    while(!(pt=(char**)realloc(dl, sizeof(char*)*(dl_len+1))));
    dl = pt;
    while(!(*(dl+dl_len) = (char*)malloc(sizeof(char)*(i-markl-1))));
    strncpy(*(dl+dl_len), path+markl+1, i-markl-1);
    dl_len++;
  }
  */

  char** dl;
  int dl_len;
  path_list* pl = get_path_list(path);
  dl = pl->list;
  dl_len = pl->len;
  for(int i=0;i<dl_len;i++){
    printf("%s\n",*(dl+i));
    
  }


  
}
