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

int create(char* disk_name,int size){
  /*
    the unit is bytes.
    we have the derivation:

    ->inode_size = size/100;
    ->inode_number = inode_size/SIZE_INODE
    --->inode_number = size/100/SIZE_INODE

    ->block_size + block_table_size = size - superblock_size - inode_number * SIZE_INODE
    --->SIZE_BLOCK*block_number + SIZE_TABLE_UNIT*block_number 
            = size - superblock_size - inode_number * SIZE_INODE
    --->block_number = (size - superblock_size - inode_number * SIZE_INODE)/(SIZE_BLOCK+SIZE_TABLE_UNIT) 
  */
  int inode_number = size/100/SIZE_INODE;
  int block_number = (size-SIZE_SUPERBLOCK-inode_number*SIZE_INODE)/(SIZE_BLOCK+SIZE_TABLE_UNIT);
 
  //initialize the superblock
  superblock sb;
  sb.inode_size = inode_number*SIZE_INODE;
  sb.inode_number = inode_number;
  sb.block_size = block_number*SIZE_BLOCK;
  sb.block_number = block_number;

  //initialize the inode
  inode node;
  node.valid = FALSE;
  
  //write superblock,block table,inode,block to the file
  FILE *fp;
  if(!fopen(disk_name,"r")){
    printf("%s",DISK_NAME_EXIST_ERROR);
    return FALSE;
  }
  fp = fopen(disk_name,"wb");
  fwrite(&sb,sizeof sb,1,fp);
  fseek(fp,block_number*8,SEEK_CUR);
  for(int i=0;i<inode_number;i++){
    fwrite(&node,sizeof node,1,fp);
  }
  fseek(fp,block_number*SIZE_BLOCK,SEEK_CUR);
  fclose(fp);
  printf("create disk success\ntotal size: %d bytes\n",
	 SIZE_SUPERBLOCK+
	 SIZE_INODE*inode_number+
	 block_number*(SIZE_BLOCK+SIZE_TABLE_UNIT));
  
  return TRUE;
  
}
int format(char* disk_name,filesystem* fs){}
int delete(char* disk_name,filesystem* fs){}


int init(filesystem* fs){
  //init attributes
  fs->disks=NULL;
  fs->number_disk = 0;//the number of disk
  fs->current_directory=-1;//means the directory is /dev
  fs->current_disk=-1;//this means we are now at the root node
  fs->buffer_inode=-1;//the pasted file's inode
  fs->buffer_disk=-1;//the pasted file's disk


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
}


