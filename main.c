#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"fs.h"
#include"main.h"

char cmd[500];
filesystem* fs;
int main(){
  printf("CALM FILE SYSTEM\nVERSION 1.0\nCalm Walter All Rights Reserved.\n");
  account* ac; 
  fs = (filesystem*)malloc(sizeof(filesystem));
  while(1){
    char name[30];
    char password[30];
    printf("Login:");
    scanf("%s", name);
    printf("Password:");
    scanf("%s",password);
    ac = verify(name, password);
    if(!ac){
      printf("NAME OR PASSWORD ERROR\n");
      continue;
    }
    //intialize file system
    init(fs);
    getchar();
    while(1){
      printf("%s@%s>",ac->name,get_path(fs->current_disk, fs->current_directory));
      int len = 0;
      while(1){
        char a;
        scanf("%c",&a);
        if(a=='\n'){
          cmd[len]='\0';
          break;
        }
        cmd[len++]=a;
      }
      if(!strcmp(cmd,"quit")){
	break;
      }
      command(cmd);
    }    
  }
  return 0;
}

void command(char* cmd){
  cmd_args* ca = __get_args(cmd);
  if(!ca){
    return;
  }
  //useradd
  if(!strcmp(*(ca->args),"useradd")){
    if (ca->len!=4){
      printf("ARGUMENTS ERROR\nuseradd <name> <password> <authority>\n");
      return;
    }
    useradd(ca->args[1],ca->args[2],atoi(ca->args[3]));
    return;
  }
  //mount
  if(!strcmp(*(ca->args),"mount")){
    if (ca->len!=2){
      printf("ARGUMENTS ERROR\nmount <disk name>\n");
      return;
    }
    mount(ca->args[1], fs);
    return;
  }
  //unmount
  if(!strcmp(*(ca->args),"unmount")){
    if (ca->len!=2){
      printf("ARGUMENTS ERROR\nunmount <disk name>\n");
      return;
    }
    unmount(ca->args[1], fs);
    return;
  }
  //format
  if(!strcmp(*(ca->args),"format")){
    if (ca->len!=2){
      printf("ARGUMENTS ERROR\nformat <disk name>\n");
      return;
    }
    format(ca->args[1], fs);
    return;
  }
  //delete
  if(!strcmp(*(ca->args),"delete")){
    if (ca->len!=2){
      printf("ARGUMENTS ERROR\ndelete <disk name>\n");
      return;
    }
    delete(ca->args[1], fs);
    return;
  }
  
  //create
  if(!strcmp(*(ca->args),"create")){
    if (ca->len!=3){
      printf("ARGUMENTS ERROR\ncreate <disk name> <size>\n");
      return;
    }
    create(ca->args[1],atoi(ca->args[2]));
    return;
  }
  //list_disks
  if(!strcmp(*(ca->args),"ldsk")){
    if (ca->len!=1){
      printf("ARGUMENTS ERROR\nldsk\n");
      return;
    }
    list_disks(fs);
    return;
  }
  
  //cd
  if(!strcmp(*(ca->args),"cd")){
    if (ca->len!=2){
      printf("ARGUMENTS ERROR\ncd <path>\n");
      return;
    }
    cd(ca->args[1],fs);
    return;
  }
  //find
  if(!strcmp(*(ca->args),"find")){
    if (ca->len!=2){
      printf("ARGUMENTS ERROR\nfind <file/directory name>\n");
      return;
    }
    find(ca->args[1], fs->current_directory, fs->current_disk, fs);
    return;
  }
  //mkdir
  if(!strcmp(*(ca->args),"mkdir")){
    if (ca->len!=2){
      printf("ARGUMENTS ERROR\nmkdir <directory name>\n");
      return;
    }
    mkdir(ca->args[1], fs);
    return;
  }
  //write
  if(!strcmp(*(ca->args),"write")){
    if (ca->len!=3){
      printf("ARGUMENTS ERROR\nwrite <file name> <content>\n");
      return;
    }
    write(ca->args[1], ca->args[2], fs);
    return;
  }
  //read
  if(!strcmp(*(ca->args),"read")){
    if (ca->len!=2){
      printf("ARGUMENTS ERROR\nread <file name>\n");
      return;
    }
    read(ca->args[1], fs);
    return;
  }
  
  //ls
  if(!strcmp(*(ca->args),"ls")){
    if (ca->len!=1){
      printf("ARGUMENTS ERROR\nls\n");
      return;
    }
    ls(fs);
    return;
  }
  //cp
  if(!strcmp(*(ca->args),"cp")){
    if (ca->len!=2){
      printf("ARGUMENTS ERROR\ncp <file path>\n");
      return;
    }
    cp(ca->args[1],fs);
    return;
  }
  //paste
  if(!strcmp(*(ca->args),"paste")){
    if (ca->len!=1){
      printf("ARGUMENTS ERROR\npaste\n");
      return;
    }
    paste(fs);
    return;
  }
  //rm
  if(!strcmp(*(ca->args),"rm")){
    if (ca->len!=2){
      printf("ARGUMENTS ERROR\nrm <file path>\n");
      return;
    }
    rm(ca->args[1],fs);
    return;
  }
  //mv
  if(!strcmp(*(ca->args),"mv")){
    if (ca->len!=3){
      printf("ARGUMENTS ERROR\nmv <file path> <dest file path>\n");
      return;
    }
    mv(ca->args[1],ca->args[2],fs);
    return;
  }  



}


cmd_args* __get_args(char* cmd){
  int len = strlen(cmd);
  if(len==0){return NULL;}
  cmd_args* ca= (cmd_args*)malloc(sizeof(cmd_args));
  ca->args = (char**)malloc(sizeof(char*));
  ca->len = 0;
  int markl=0;
  int i=0;
  while(cmd[i]=='\t' || cmd[i]==' ' || cmd[i]=='\n'){
    i++;
    markl++;
    if(i==len){
      free(ca);
      return NULL;
    }
  }
  int sign=1;
  char** ptr;
  for(;i<len;i++){
    if(cmd[i]=='\t' || cmd[i]==' ' || cmd[i]=='\n'){
      if(!sign){
	continue;
      }
      while(!(ptr = (char**)realloc(ca->args, sizeof(char*)*(ca->len+1))));
      ca->args = ptr;
      while(!(*(ca->args+ca->len)=(char*)malloc(sizeof(char)*(i-markl+1))));
      char* arg = *(ca->args+ca->len);
      *arg = '\0';
      strncpy(arg, cmd+markl, i-markl);
      arg[i-markl]='\0';
      ca->len++;
      sign = 0;
      
    }else{
      if(sign==0){
        markl=i;
      }
      sign = 1;
    }
  }
  
  if(cmd[len-1]!='\t' && cmd[len-1]!=' ' && cmd[len-1]!='\n'){  
    while(!(ptr = (char**)realloc(ca->args, sizeof(char*)*(ca->len+1))));
    ca->args = ptr;
    while(!(*(ca->args+ca->len)=(char*)malloc(sizeof(char)*(i-markl+1))));
    **(ca->args+ca->len) = '\0';
    strncpy(*(ca->args+ca->len), cmd+markl, i-markl);
    (*(ca->args+ca->len))[i-markl]='\0';
    ca->len++;
  }
  return ca;
}
