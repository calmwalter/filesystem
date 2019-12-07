#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include"fs.h"


account* verify(char* name,char* password){
  char path[200]="./data/";
  strcat(path, name);
  printf("%s\n",path);
  FILE* fp=fopen(path,"rb");
  if(!fp){
    return NULL;
  }
  account* ac=(account*)malloc(sizeof(account));
  fread(ac, sizeof(struct account), 1, fp);
  if(!strcmp(name,ac->name) && !strcmp(password,ac->password)){
    fclose(fp);
    return ac;
  }
  fclose(fp);
  free(ac);
  return NULL;
}

int useradd(char* name,char* password, int authority){
  char path[200]="./data/";
  strcat(path, name);
  printf("%s\n",path);
  FILE* fp=fopen(path,"rb");
  if(!fp){
    fp=fopen(path,"wb");
    account ac;
    ac.authority=authority;
    strcpy(ac.name, name);
    strcpy(ac.password,password);

    while(!fwrite(&ac, sizeof(struct account), 1,fp));

    fclose(fp);
    return TRUE;
  }
  fclose(fp);
  return FALSE;  
}
