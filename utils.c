/*
Calm Walter All Rights Reserved 2019

put utils in this file

*/
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include"fs.h"

void __update_size(int size, int pos, disk* di){
  inode* in = di->inodes + pos;
  in->size+=size;
  __write_inode_to_disk(di, pos);
  if(in->parent_directory!=-1){
    __update_size(size, in->parent_directory, di);
  }
}


int __find_aim(int cur_dir,char* aim_name,disk* di){
  //find direct pointer
  inode* in = di->inodes+cur_dir;
  inode* aim;
  for(int i=0;i<NUMBER_DIRECT_POINTER;i++){
    if(*(in->direct+i)==-1){
      return -1;
    }
    aim = di->inodes+(*(in->direct+i));
    if(!strcmp(aim->name,aim_name)){
      return *(in->direct+i);
    }
  }
  //find indirect pointer
  if(in->indirect!=-1){
    FILE* fp = fopen(di->disk_name,"rb");
    long block_offset = SIZE_SUPERBLOCK+
      SIZE_TABLE_UNIT*(di->sb->block_number)+
      SIZE_INODE*(di->sb->inode_number);

    long offset = block_offset+SIZE_BLOCK*(in->indirect);

    fseek(fp,offset,SEEK_SET);
    int in_num = -1;
    while(1){
      fread(&in_num, sizeof(int), 1, fp);

      if(in_num == -2){
        fread(&in_num, sizeof(int), 1, fp);
        offset = block_offset+SIZE_BLOCK*(in_num);
        fseek(fp, offset, SEEK_SET);
        fread(&in_num, sizeof(int), 1, fp);

      }
      if(in_num==-3){
        continue;
      }
      if(in_num==-1){
	return -1;
      }
      aim = di->inodes+in_num;
      if(!strcmp(aim->name,aim_name)){
	return in_num;
      }
    }
  }
  return -1;
}

path_list* __get_path_list(char* path){
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

position* __search_position(path_list* pl, filesystem* fs){

  char** dl = pl->list;
  int dl_len = pl->len;
  //find the aim
  int cur_dir=fs->current_directory;
  disk* di = fs->current_disk;
  int i=0;
  //judge if start from the root directory
  if(!strcmp(*dl,"/")){
    if (dl_len==1){
      position* pos;
      while(!(pos = (position*)malloc(sizeof(struct position))));
      pos->di = NULL;
      pos->position = -1;
      return pos;
    }

    di = __find_disk(*(dl+1),fs);
    if(!di){
      //printf("%s\n",PATH_NOT_FOUND_ERROR);
      return NULL;
    }

    if (dl_len==2){
      position* pos;
      while(!(pos = (position*)malloc(sizeof(struct position))));
      pos->di = di;
      pos->position = 0;
      return pos;
    }

    cur_dir = 0;
    i=2;
  }else if(fs->current_disk==NULL && fs->current_directory==-1){
    di = __find_disk(*dl,fs);
    if(!di){
      //printf("%s\n",PATH_NOT_FOUND_ERROR);
      return NULL;
    }
    if (dl_len==1)
      {
	position* pos;
	while(!(pos = (position*)malloc(sizeof(struct position))));
	pos->di = di;
	pos->position = 0;
	return pos;
      }
    cur_dir = 0;
    i=1;
  }

  for(;i<dl_len-1;i++){
    cur_dir = __find_dir(cur_dir,*(dl+i),di);//if find return the directory, else return -1
    if(cur_dir==-1){
      //printf("%s\n",PATH_NOT_FOUND_ERROR);
      return NULL;
    }
  }
  int aim = __find_aim(cur_dir, *(dl+dl_len-1), di);
  if(aim==-1){
    //printf("%s\n",PATH_NOT_FOUND_ERROR);
    return NULL;
  }
  position* pos;
  while(!(pos = (position*)malloc(sizeof(struct position))));
  pos->di = di;
  pos->position = aim;
  return pos;
}



void __write_inode_to_disk(disk* di, int position){
  long inode_offset = SIZE_SUPERBLOCK+
    SIZE_TABLE_UNIT*(di->sb->block_number);
  long offset = inode_offset+SIZE_INODE*(position);
  FILE* fp = fopen(di->disk_name,"rb+");
  fseek(fp, offset, SEEK_SET);
  fwrite(di->inodes+position, sizeof(struct inode), 1, fp);
  fclose(fp);
}

void __set_inode_pointer(int cur_value, int set_value,int position, disk* di){
  //direct pointer
  int* pt;
  for(int i=0;i<NUMBER_DIRECT_POINTER;i++){
    pt = (di->inodes+position)->direct+i;
    if(*pt==-1){
      return;
    }
    if(*pt==cur_value){
      *pt = set_value;
      return;
    }
  }
  //indirect pointer
  if((di->inodes+position)->indirect!=-1){
    long block_offset = SIZE_SUPERBLOCK+
      SIZE_TABLE_UNIT*(di->sb->block_number)+
      SIZE_INODE*(di->sb->inode_number);
    long offset = block_offset+SIZE_BLOCK*((di->inodes+position)->indirect);
    FILE* fp = fopen(di->disk_name,"rb+");
    fseek(fp, offset, SEEK_SET);

    int in_num = -1;
    while(1){
      fread(&in_num, sizeof(int), 1, fp);

      if(in_num == -2){
        fread(&in_num, sizeof(int), 1, fp);
        offset = block_offset+SIZE_BLOCK*(in_num);
        fseek(fp, offset, SEEK_SET);
        fread(&in_num, sizeof(int), 1, fp);
      }
      if(in_num==-3){
        continue;
      }
      if(in_num==-1){
	return;
      }
      if(in_num==cur_value){
	fseek(fp, -sizeof(int), SEEK_CUR);
	fwrite(&set_value, sizeof(int), 1, fp);
	fclose(fp);
	return;
      }
    }
  }

}

void __write_table_to_disk(disk* di, int pos, int valid){
  FILE* fp = fopen(di->disk_name,"rb+");
  long offset = SIZE_SUPERBLOCK+SIZE_TABLE_UNIT*pos;
  fseek(fp, offset, SEEK_SET);
  int sc = fwrite(&valid, sizeof(int), 1, fp);

  fclose(fp);
}


int __add_inode_pointer(int value, disk* di, int pos){
  inode* in = di->inodes+pos;
  //find direct
  for(int i=0;i<NUMBER_DIRECT_POINTER;i++){
    if(*(in->direct+i)==BLOCK_POSITION_NULL ){
      *(in->direct+i) = value;
      __write_inode_to_disk(di, pos);
      return TRUE;
    }
    if (*(in->direct+i)==END_BLOCK)
      {
	if(i!=NUMBER_DIRECT_POINTER-1){
	  *(in->direct+i+1)=-1;
	}
	*(in->direct+i) = value;
	__write_inode_to_disk(di, pos);
	return TRUE;
      }

  }
  //find indirect
  if(in->indirect!=-1){
    //find a position
    FILE* fp = fopen(di->disk_name,"rb+");
    long block_offset = SIZE_SUPERBLOCK+
      SIZE_TABLE_UNIT*(di->sb->block_number)+
      SIZE_INODE*(di->sb->inode_number);
    long offset = block_offset + SIZE_BLOCK*(in->indirect);
    fseek(fp, offset, SEEK_SET);
    int in_num=-1;
    while(1){
      fread(&in_num, sizeof(int), 1, fp);
      if(in_num==-2){
	fread(&in_num, sizeof(int), 1, fp);
        offset = block_offset+in_num*SIZE_BLOCK;
        fseek(fp, offset, SEEK_SET);
        fread(&in_num, sizeof(int), 1, fp);
      }
      if(in_num==END_BLOCK){
        //create new position
        //at the end of the block
        if(ftell(fp)==offset+SIZE_BLOCK){
          int pre_num;
          fseek(fp,ftell(fp)-sizeof(int)*2,SEEK_SET);
          fread(&pre_num, sizeof(int), 1, fp);
          fseek(fp,ftell(fp)-sizeof(int),SEEK_SET);
          int tmp=-2;
          fwrite(&tmp, sizeof(int), 1, fp);
          //we have another file operate so we need close the file stream
          long offset_tmp = ftell(fp);
          fclose(fp);
          //find NULL block
          for(int i=0;i<di->sb->block_number;i++){
            if(*(di->block_table+i)==FALSE){
              *(di->block_table+i)=TRUE;
              __write_table_to_disk(di,i,TRUE);
              tmp = i;
              break;
            }
          }

          // if not find
          if(tmp==-2){
            printf("%s\n",NO_ENOGH_BLOCK_SPEACE_ERROR);
            return FALSE;
          }
          //reopen file stream and recover the offset
          fp = fopen(di->disk_name,"rb+");
          fseek(fp,offset_tmp,SEEK_SET);
          //write next block number to the end of the block
          fwrite(&tmp, sizeof(int), 1, fp);
          //goto the next block and write prenum and new num to it
          offset = block_offset + SIZE_BLOCK*tmp;
          fseek(fp,offset,SEEK_SET);
          fwrite(&pre_num, sizeof(int), 1, fp);
          fwrite(&value, sizeof(int), 1, fp);
          //write END_BLOCK to the file
          tmp = -1;
          fwrite(&tmp, sizeof(int), 1, fp);
          fclose(fp);
          return TRUE;
        }
	else{
	  fseek(fp, ftell(fp)-sizeof(int), SEEK_SET);
	  fwrite(&value, sizeof(int), 1, fp);
	  int tmp = -1;
	  fwrite(&tmp,sizeof(int),1,fp);
	  fclose(fp);
	  return TRUE;
	}
      }
      if(in_num==BLOCK_POSITION_NULL){
        //write to it
        fseek(fp, ftell(fp)-sizeof(int), SEEK_SET);
        fwrite(&value, sizeof(int), 1, fp);
        fclose(fp);
        return TRUE;
      }
    }
  }else{
    //select a empty block to write
    //find NULL block
    int tmp = -2;
    for(int i=0;i<di->sb->block_number;i++){
      if(*(di->block_table+i)==FALSE){
        *(di->block_table+i)=TRUE;
        (di->inodes+pos)->indirect = i;
        __write_table_to_disk(di,i,TRUE);
        tmp = i;
        break;
      }
    }
    // if not find
    if(tmp==-2){
      printf("%s\n",NO_ENOGH_BLOCK_SPEACE_ERROR);
      return FALSE;
    }
    FILE* fp = fopen(di->disk_name,"rb+");
    long block_offset = SIZE_SUPERBLOCK+
      SIZE_TABLE_UNIT*(di->sb->block_number)+
      SIZE_INODE*(di->sb->inode_number);
    long offset = block_offset + SIZE_BLOCK*(tmp);
    fseek(fp, offset, SEEK_SET);
    fwrite(&value, sizeof(int), 1, fp);
    tmp=-1;
    fwrite(&tmp,sizeof(tmp),1,fp);//end of the block
    fclose(fp);
  }
  //write to disk
  __write_inode_to_disk(di, pos);
  return TRUE;
}



char* __get_path(disk* di, int pos){
  if(!di && pos==-1){
    return "/";
  }
  inode* in = di->inodes+pos;
  if(in->parent_directory==-1){
    int len = sizeof(char)+strlen(in->name);
    char* path = (char*)malloc(sizeof(char)*(len+1));
    path[0]='\0';
    strcat(path, "/");
    strcat(path, in->name);
    return path;
  }
  char* path = __get_path(di, in->parent_directory);
  int len = strlen(path)+strlen(in->name)+sizeof(char);
  char* new_path = (char*)malloc(sizeof(char)*(len+1));
  new_path[0]='\0';
  strcat(new_path, path);
  strcat(new_path, "/");
  strcat(new_path, in->name);
  free(path);
  return new_path;

}


disk* __find_disk(char* disk_name,filesystem* fs){
  disk* di = fs->disks;
  while(di){
    if(!strcmp(di->disk_name, disk_name)){
      return di;
    }
    di = di->next_disk;
  }
  return NULL;
}



int __find_dir(int cur_dir,char* aim_dir_name,disk* di){
  //find direct pointer
  inode* in = di->inodes+cur_dir;
  inode* aim;
  for(int i=0;i<NUMBER_DIRECT_POINTER;i++){
    if(*(in->direct+i)==-1){
      return -1;
    }
    aim = di->inodes+(*(in->direct+i));
    if(aim->name==aim_dir_name && aim->type==TYPE_DIRECTORY){
      return *(in->direct+i);
    }
  }
  //find indirect pointer
  FILE* fp = fopen(di->disk_name,"rb");
  long block_offset = SIZE_SUPERBLOCK+
    SIZE_TABLE_UNIT*(di->sb->block_number)+
    SIZE_INODE*(di->sb->inode_number);

  long offset = block_offset+SIZE_BLOCK*(in->indirect);

  fseek(fp,offset,SEEK_SET);
  int in_num = -1;
  while(1){
    fread(&in_num, sizeof(int), 1, fp);

    if(in_num == -2){
      fread(&in_num, sizeof(int), 1, fp);
      offset = block_offset+SIZE_BLOCK*in_num;
      fseek(fp, offset, SEEK_SET);
      fread(&in_num, offset, 1, fp);

    }
    if(in_num==-3){
      continue;
    }
    if(in_num==-1){
      return -1;
    }
    aim = di->inodes+in_num;
    if(aim->name==aim_dir_name && aim->type==TYPE_DIRECTORY){
      return in_num;
    }
  }
  return -1;
}


void __free_disk(disk* dp){
  //free disk memory
  free(dp->disk_name);
  free(dp->block_table);
  free(dp->sb);
  free(dp->inodes);
  free(dp);
}

int __check_permission(inode* in,filesystem* fs){

  // only the administrator and permit user and can operate file can be accessed
  if(!((fs->user->authority==ADMINISTRATOR)||(!strcmp(in->owner,fs->user->name)))){
    return FALSE;
  }
  return TRUE;
}