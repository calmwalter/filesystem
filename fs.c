#include<stdio.h>
#include"fs.h"

int write(char* file_path,char* content,filesystem* fs){}
int read(char* file_path,filesystem* fs){}
int ls(char* file_path,filesystem* fs){}
int cp(char* file_path,filesystem* fs){}
int paste(filesystem* fs){}
int rm(char* file_path,filesystem* fs){}
int mv(char* file_path, char* dest_file_path,filesystem* fs){}
int mkdir(char* directory_name,filesystem* fs){}
int find(char* name,filesystem* fs){}
int cd(char* path,filesystem* fs){}


int mount(char* disk_name,filesystem* fs){}
int unmount(char* disk_name,filesystem* fs){}
int create(char* disk_name,int size,filesystem* fs){}
int format(char* disk_name,filesystem* fs){}
int delete(char* disk_name,filesystem* fs){}


int init(filesystem* fs){
  //init the inner function of filesystem
  fs->write = write;
  fs->read = read;
  fs->ls = ls;
  fs->cp = cp;
  fs->paste = paste;
  fs->rm = rm;
  fs->mv = mv;
  fs->mkdir = mkdir;
  fs->find = find;
  fs->cd = cd;

  fs->mount = mount;
  fs->unmount = unmount;
  fs->create = create;
  fs->format = format;
  fs->delete = delete;
  
  fs->init = init;

  //init attributes
  fs->disk_head=NULL;
  fs->current_directory=-1;//means the directory is /dev
  fs->current_disk
}

