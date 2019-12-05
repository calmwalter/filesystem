#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include"fs.h"

//int write(char* file_path,char* content,filesystem* fs){}
//int read(char* file_path,filesystem* fs){}

void ls(filesystem* fs){
  //find direct pointer
  disk* di = fs->current_disk;
  inode* in = di->inodes+fs->current_directory;
  inode* aim;
  for(int i=0;i<NUMBER_DIRECT_POINTER;i++){
    if(*(in->direct+i)==-1){
      return;
    }
    aim = di->inodes+(*(in->direct+i));
    printf("%s\t%d\t%d\n",aim->name,aim->type,aim->size);
  }
  
  //find indirect pointer
  if(in->indirect!=-1){
    FILE* fp = fopen(di->disk_name,"rb");
    long block_offset = SIZE_SUPERBLOCK+
      SIZE_TABLE_UNIT*(di->sb->block_number)+
      SIZE_INODE*(di->sb->inode_number);
  
    long offset = block_offset+SIZE_BLOCK*(in->indirect);
  
    fseek(fp,offset,SEEK_CUR);
    int in_num = 0;
    while(in_num!=-1){
      fread(&in_num, sizeof(int), 1, fp);
      if(in_num == -2){
	fread(&in_num, sizeof(int), 1, fp);
	offset = block_offset+SIZE_BLOCK*(in_num-1);
	fread(&in_num, offset, 1, fp);
	if(in_num==-1){
	  return;
	}
      }
      aim = di->inodes+in_num;
      printf("%s\t%d\t%d\n",aim->name,aim->type,aim->size);
    }
  }
}

int find_aim(int cur_dir,char* aim_name,disk* di){
  //find direct pointer
  inode* in = di->inodes+cur_dir;
  inode* aim;
  for(int i=0;i<NUMBER_DIRECT_POINTER;i++){
    if(*(in->direct+i)==-1){
      return -1;
    }
    aim = di->inodes+(*(in->direct+i));
    if(aim->name==aim_name){
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
    int in_num = 0;
    while(in_num!=-1){
      fread(&in_num, sizeof(int), 1, fp);
      if(in_num == -2){
	fread(&in_num, sizeof(int), 1, fp);
	offset = block_offset+SIZE_BLOCK*(in_num);
	fseek(fp, offset, SEEK_SET);
	fread(&in_num, sizeof(int), 1, fp);
	if(in_num==-1){
	  return -1;
	}
      }
      aim = di->inodes+in_num;
      if(aim->name==aim_name){
	return in_num;
      }
    }
  }
  return -1;
}

path_list* get_path_list(char* path){
  //first get list
  char** dl=NULL;//directory list from path
  int dl_len=0;//len of dl
  int len = strlen(path);//len of path
  int markl=-1;
  int i=0;
  if(path[0]=='/'){
    markl++;
    i++;
  }
  char** pt=NULL;
  for(;i<len;i++){
    if(path[i]=='/'){
      while(!(pt=(char**)realloc(dl, sizeof(char*)*(dl_len+1))));
      dl = pt;
      while(!(*(dl+dl_len) = (char*)malloc(sizeof(char)*(i-markl-1))));
      strncpy(*(dl+dl_len), path+markl+1, i-markl-1);
      markl = i;
      dl_len++;
    }
  }
  if(path[len-1]!='/'){
    while(!(pt=(char**)realloc(dl, sizeof(char*)*(dl_len+1))));
    dl = pt;
    while(!(*(dl+dl_len) = (char*)malloc(sizeof(char)*(i-markl-1))));
    strncpy(*(dl+dl_len), path+markl+1, i-markl-1);
    dl_len++;
  }
  path_list* pl=(path_list*)malloc(sizeof(struct path_list));
  pl->len=dl_len;
  pl->list = dl;
  return pl;
}

position* search_position(path_list* pl, filesystem* fs){
  char** dl = pl->list;
  int dl_len = pl->len;
  
  //find the aim
  int cur_dir=fs->current_directory;
  disk* di = fs->current_disk;
  int i=0;
  //judge if start from the root directory
  if(!strcmp(*dl,"dev")){
    //TODO write find disk function
    di = find_disk(*(dl+1),fs);
    if(!di){
      printf("%s\n",PATH_NOT_FOUND_ERROR);
      return NULL;
    }
    cur_dir = 0;
    i=2;
  }
  
  for(;i<dl_len-1;i++){
    //TODO: find next directory (current_directory,aim_dir_name,disk)
    cur_dir = find_dir(cur_dir,*(dl+i),di);//if find return the directory, else return -1
    if(cur_dir==-1){
      printf("%s\n",PATH_NOT_FOUND_ERROR);
      return NULL;
    }
  }
  int aim = find_aim(cur_dir, *(dl+dl_len-1), di);
  if(aim==-1){
    printf("%s\n",PATH_NOT_FOUND_ERROR);
    return NULL;
  }
  position* pos;
  while(!(pos = (position*)malloc(sizeof(struct position))));
  pos->di = di;
  pos->position = aim;
  return pos;
}

int cp(char* path,filesystem* fs){
  //first get list
  path_list* pl = get_path_list(path);
  
  //find the aim
  position* pos = search_position(pl, fs);
  if(!pos){
    return FALSE;
  }
  fs->buffer_disk = pos->di;
  fs->buffer_inode = pos->position;
  return TRUE;
}

int paste(filesystem* fs){}

void write_inode_to_disk(disk* di, int position){
  long inode_offset = SIZE_SUPERBLOCK+
    SIZE_TABLE_UNIT*(di->sb->block_number);
  long offset = inode_offset+SIZE_INODE*(position);
  FILE* fp = fopen(di->disk_name,"rb+");
  fseek(fp, offset, SEEK_SET);
  fwrite(di->inodes+position, sizeof(struct inode), 1, fp);
  fclose(fp);
}
void set_inode_pointer(int cur_value, int set_value,int position, disk* di){
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

    int in_num = 0;
    while(in_num!=-1){
      fread(&in_num, sizeof(int), 1, fp);
      if(in_num == -2){
	fread(&in_num, sizeof(int), 1, fp);
	offset = block_offset+SIZE_BLOCK*(in_num);
	fseek(fp, offset, SEEK_SET);
	fread(&in_num, sizeof(int), 1, fp);
	if(in_num==-1){
	  return;
	}
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
int rm(char* path,filesystem* fs){
  //first get list
  path_list* pl = get_path_list(path);
  
  //find the aim
  position* pos = search_position(pl, fs);
  if(!pos){
    return FALSE;
  }
  disk* di = pos->di;
  int aim = pos->position;
  //set invalid
  (di->inodes+aim)->valid = FALSE;
  //delete the parent pointer
  int par_aim = (di->inodes+aim)->parent_directory;
  set_inode_pointer(aim, -3, par_aim, di);
  
  //write to file
  write_inode_to_disk(di, aim);
  write_inode_to_disk(di, (di->inodes+aim)->parent_directory);
  return TRUE;
}

void write_table_to_disk(disk* di, int pos, int valid){
  FILE* fp = fopen(di->disk_name,"rb+");
  long offset = SIZE_SUPERBLOCK+SIZE_TABLE_UNIT*pos;
  fseek(fp, offset, SEEK_SET);
  fwrite(&valid, sizeof(int), 1, fp);
  fclose(fp);
}
int add_inode_pointer(int value, disk* di, int pos){
  inode* in = di->inodes+pos;
  //find direct
  for(int i=0;i<NUMBER_DIRECT_POINTER;i++){
    if(*(in->direct+i)==BLOCK_POSITION_NULL || *(in->direct+i)==-1){
      *(in->direct+i) = value;
      write_inode_to_disk(di, pos);
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
    int in_num=0;
    while(in_num!=-1){
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
	  //find NULL block
	  for(int i=0;i<di->sb->block_number;i++){
	    if(*(di->block_table+i)==FALSE){
	      *(di->block_table+i)=TRUE;
	      write_table_to_disk(di,i,TRUE);
	      tmp = i;
	      break;
	    }
	  }
	  // if not find
	  if(tmp==-2){
	    printf("%s\n",NO_ENOGH_BLOCK_SPEACE_ERROR);
	    return FALSE;
	  }
	  
	  fwrite(&tmp, sizeof(int), 1, fp);
          offset = block_offset + SIZE_BLOCK*tmp;
	  fseek(fp,offset,SEEK_SET);
	  fwrite(&pre_num, sizeof(int), 1, fp);
	  fwrite(&value, sizeof(int), 1, fp);
	  tmp = -1;
	  fwrite(&tmp, sizeof(int), 1, fp);
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
	write_table_to_disk(di,i,TRUE);
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
  }
  //write to disk
  write_inode_to_disk(di, pos);
  return TRUE;
}

//TODO achieve multi disk operation
int mv(char* file_path, char* dest_file_path,filesystem* fs){
  //get file path list
  path_list* pl_aim = get_path_list(file_path);
  //get aim inode
  position* pos_aim = search_position(pl_aim, fs);
  if(!pos_aim){
    printf("%s\n",PATH_NOT_FOUND_ERROR);
    return FALSE;
  }
  //get dest path list
  path_list* pl_dest = get_path_list(dest_file_path);
  //get dest directory inode
  position* pos_dest = search_position(pl_dest, fs);
  if(!pos_dest || (pos_dest->di->inodes+pos_dest->position)->type == TYPE_FILE){
    printf("%s\n",PATH_NOT_FOUND_ERROR);
    return FALSE;
  }
  
  //update the inode
  inode* in_aim = pos_aim->di->inodes+pos_aim->position;
  
  //set the parent pointer to be -3
  set_inode_pointer(pos_aim->position, BLOCK_POSITION_NULL, in_aim->parent_directory, pos_aim->di);
  //change the parent pointer to the new one 
  in_aim->parent_directory = pos_dest->position;
  //add the inode pointer to the new parent directory
  add_inode_pointer(pos_aim->position, pos_dest->di, pos_dest->position);
  
  //update three inodes to disk
  write_inode_to_disk(pos_aim->di, pos_aim->position);
  write_inode_to_disk(pos_aim->di, in_aim->parent_directory);
  write_inode_to_disk(pos_dest->di, pos_dest->position);
  return TRUE;
}
  
int mkdir(char* directory_name,filesystem* fs){
  //check if the name already exist
  disk* cur_di = fs->current_disk;
  int cur_dir = fs->current_directory;
  inode* in = cur_di->inodes;
  inode* cur_in = in+cur_dir;
  inode* aim;
  int i=0;

  //->check direct file
  while(cur_in->direct[i]!=-1){
    aim = in+cur_in->direct[i];
    if(!strcmp(aim->name,directory_name)){
      printf("%s\n",NAME_ALREADY_EXIST_ERROR);
      return FALSE;
    }        
    i++;
  }
  if(cur_in->indirect!=-1){
    //->check indirect file
    i = 0;
    //-->read disk get aim pointer
    long block_offset = SIZE_SUPERBLOCK+
      SIZE_TABLE_UNIT*(cur_di->sb->block_number)+
      SIZE_INODE*(cur_di->sb->inode_number);
  
    long offset = block_offset+SIZE_BLOCK*(cur_in->indirect);
    FILE* fp = fopen(cur_di->disk_name,"rb");
    fseek(fp, offset,SEEK_SET);
    int aim_dir;
    while((aim_dir = fread(&aim_dir, sizeof(int), 1, fp))!=END_BLOCK){
      if(aim_dir == NEXT_BLOCK){
	aim_dir = fread(&aim_dir, sizeof(int), 1, fp);
	fclose(fp);
	offset = block_offset+SIZE_BLOCK*(aim_dir);
	fp = fopen(cur_di->disk_name,"rb");
	fseek(fp, offset, SEEK_SET);
	aim_dir = fread(&aim_dir, sizeof(int), 1, fp);
	if(aim_dir == END_BLOCK){
	  break;
	}
      }
      if(aim_dir == BLOCK_POSITION_NULL){
	continue;
      }
      aim = in+aim_dir;
      if(!strcmp(aim->name,directory_name)){
	printf("%s\n",NAME_ALREADY_EXIST_ERROR);
	fclose(fp);
	return FALSE;
      }
    }
    fclose(fp);
    
  }
  //write to free inode
  int in_num = cur_di->sb->inode_number;
  
  for(int i=0;i<in_num;i++){
    if((in+i)->valid==FALSE){
      inode* new_in = in+i;
      new_in->direct[0] = -1;
      new_in->indirect = -1;
      
      strcpy(new_in->name,directory_name);
      new_in->parent_directory = cur_dir;
      new_in->size=0;
      new_in->valid=TRUE;
      add_inode_pointer(i,cur_di,cur_dir);//add to parent inode pointer

      //write to disk
      write_inode_to_disk(cur_di, cur_dir);
      write_inode_to_disk(cur_di, i);
      return TRUE;
    }
  }
  
  printf("%s\n",NO_ENOUGH_INDOE_SPACE_ERROR);
  return FALSE;
}
char* get_path(disk* di, int pos){
  inode* in = di->inodes+pos;
  if(in->parent_directory==0){
    int len = sizeof(char)*3+strlen(di->disk_name)+strlen(in->name)+strlen("dev");
    char* path = (char*)malloc(sizeof(char)*len);
    strcat(path, "/");
    strcat(path, "dev");
    strcat(path, "/");
    strcat(path, di->disk_name);
    strcat(path, "/");
    strcat(path, in->name);
    return path;
  }
  char* path = get_path(di, in->parent_directory);
  int len = strlen(path)+strlen(in->name)+sizeof(char);
  char* new_path = (char*)malloc(sizeof(char)*len);
  strcat(new_path, path);
  strcat(new_path, "/");
  strcat(new_path, in->name);
  free(path);
  return new_path;
  
}
//find all file or directory under current directory use recursive descent
void find(char* name,int cur_dir,filesystem* fs){
  //find direct pointer
  disk* di = fs->current_disk;
  inode* in = di->inodes+cur_dir;
  inode* aim;
  for(int i=0;i<NUMBER_DIRECT_POINTER;i++){
    if(*(in->direct+i)==-1){
      return;
    }
    if(*(in->direct+i)==-3){
      continue;
    }
    aim = di->inodes+(*(in->direct+i));
    if(aim->name==name){
      //TODO: find file path
      char* file_path = get_path(di,*(in->direct+i));
      printf("%s\t%d\t%d\t%s\n",aim->name,aim->type,aim->size,file_path);
      free(file_path);
    }
    if(aim->type==TYPE_DIRECTORY){
      find(name,*(in->direct+i),fs);
    }
  }
  //find indirect pointer
  FILE* fp = fopen(di->disk_name,"rb");
  long block_offset = SIZE_SUPERBLOCK+
    SIZE_TABLE_UNIT*(di->sb->block_number)+
    SIZE_INODE*(di->sb->inode_number);
  
  long offset = block_offset+SIZE_BLOCK*(in->indirect);
  fseek(fp,offset,SEEK_SET);
  
  int in_num = 0;
  while(in_num!=-1){
    fread(&in_num, sizeof(int), 1, fp);
    if(in_num == -3){
      continue;
    }
    if(in_num == -2){
      fread(&in_num, sizeof(int), 1, fp);
      offset = block_offset+SIZE_BLOCK*in_num;
      fseek(fp,offset,SEEK_SET);
      fread(&in_num, sizeof(int), 1, fp);
      if(in_num==-1){
	return;
      }
    }
    aim = di->inodes+in_num;
    if(aim->name==name){
      char* file_path = get_path(di,in_num);
      printf("%s\t%d\t%d\t%s\n",aim->name,aim->type,aim->size,file_path);
      free(file_path);
    }
    if(aim->type==TYPE_DIRECTORY){
      find(name,in_num,fs);
    }
  }
  return;
}

disk* find_disk(char* disk_name,filesystem* fs){
  disk* di = fs->disks;
  while(di){
    if(!strcmp(di->disk_name, disk_name)){
      return di;
    }
    di = di->next_disk;
  }
  return NULL;
}

int find_dir(int cur_dir,char* aim_dir_name,disk* di){
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
  int in_num = 0;
  while(in_num!=-1){
    fread(&in_num, sizeof(int), 1, fp);
    if(in_num==-3){
      continue;
    }
    if(in_num == -2){
      fread(&in_num, sizeof(int), 1, fp);
      offset = block_offset+SIZE_BLOCK*in_num;
      fseek(fp, offset, SEEK_SET);
      fread(&in_num, offset, 1, fp);
      if(in_num==-1){
	return -1;
      }
    }
    aim = di->inodes+in_num;
    if(aim->name==aim_dir_name && aim->type==TYPE_DIRECTORY){
      return in_num;
    }
  }
  return -1;
}


int cd(char* path,filesystem* fs){
  //first get list
  path_list* pl = get_path_list(path);

  //find the directory
  position* pos = search_position(pl, fs);
  if((pos->di->inodes+pos->position)->type != TYPE_DIRECTORY){
    printf("%s\n",PATH_NOT_FOUND_ERROR);
    return FALSE;
  }
  fs->current_disk = pos->di;
  fs->current_directory = pos->position;
  return TRUE;
}


int mount(char* disk_name,filesystem* fs){
  /*
    mount the disk to the file system
  */
  //check if the disk exists
  FILE *fp;
  fp = fopen(disk_name,"r");
  if(!fp){
    printf("%s\n",DISK_NOT_EXIST_ERROR);
    return FALSE;
  }
  fclose(fp);
  //check if the disk already mounted
  
  disk* dp = fs->disks;
  while(dp){
    if(!strcmp(dp->disk_name,disk_name)){
      printf("%s\n",DISK_ALREADY_MOUNT_ERROR);
      return FALSE;
    }
    dp = dp->next_disk;
  }

  //file pointer to read data from the disk
  
  //allocate super block memory
  superblock* sb;
  while(!(sb=(struct superblock*)malloc(sizeof(struct superblock))));
  //read super block from disk
  fp = fopen(disk_name,"rb");
  if(!fread(sb, sizeof(struct superblock), 1, fp)){
    printf("%s\n",DISK_SUPERBLOCK_READ_FAIL_ERROR);
    free(sb);
    fclose(fp);
    return FALSE;
  }
  
  //allocate block table memory
  int* bt;
  while(!(bt = (int*)malloc(sizeof(int)*sb->block_number)));
  if((int)fread(bt, sizeof(int),sb->block_number,fp)!=sb->block_number){
    printf("%s\n",DISK_BLOCK_TABLE_READ_FAIL_ERROR);
    free(sb);free(bt);
    fclose(fp);
    return FALSE;
  }
  
  //allocate inodes memory
  inode* in;
  while(!(in=(inode*)malloc(sizeof(struct inode)*sb->inode_number)));
  if((int)fread(in, sizeof(struct inode), sb->inode_number, fp)!=sb->inode_number){
    printf("%s\n",DISK_INODE_READ_FAIL_ERROR);
    free(sb);free(bt);free(in);
    fclose(fp);
    return FALSE;
  }

  fclose(fp);
  
  //allocate disk memory
  disk* di;
  while(!(di=(disk*)malloc(sizeof(struct disk))));
  
  //initialize disk and put it to the file system
  int len = strlen(disk_name);
  while(!(di->disk_name=(char*)malloc(sizeof(char)*len)));
  strcpy(di->disk_name, disk_name);

  di->sb = sb;
  di->inodes = in;
  di->block_table = bt;
  di->next_disk = fs->disks;
  fs->disks = di;
  
  fs->number_disk++;
  printf("mount disk %s success\nused: %d/%d bytes\n",disk_name,sb->block_size,sb->block_number*SIZE_BLOCK);
  return TRUE;
}
void free_disk(disk* dp){
  //free disk memory
  free(dp->disk_name);
  free(dp->block_table);
  free(dp->sb);
  free(dp->inodes);
  free(dp);
}


int unmount(char* disk_name,filesystem* fs){
  //free all dynamic memory
  if(!fs->disks){
    printf("%s\n",NO_DISK_MOUNT_ERROR);
    return FALSE;
  }
  disk* dp = fs->disks;
  disk* dpt=NULL;
  while(dp){
    if(!strcmp(dp->disk_name,disk_name)){
      if(!dpt){
	      fs->disks=dp->next_disk;
      }else{
	      dpt->next_disk = dp->next_disk;
      }
      free_disk(dp);
      return TRUE;
    }
    dpt = dp;
    dp = dp->next_disk;
  }
  printf("%s\n",DISK_NAME_NOT_FOUND_ERROR);
  return FALSE;
}


//TODO: rewrite this part
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
  //printf("inode number:%d\tblock number:%d\n",inode_number,block_number);

  //initialize the superblock
  superblock sb;
  sb.inode_size = 0;
  sb.inode_number = inode_number;
  sb.block_size = 0;
  sb.block_number = block_number;

  //initialize the inode
  inode node;
  
  //write superblock,block table,inode,block to the file
  FILE *fp;
  fp = fopen(disk_name,"r");
  if(fp){
    printf("%s\n",DISK_NAME_EXIST_ERROR);
    fclose(fp);
    return FALSE;
  }
  fp = fopen(disk_name,"wb");
  fwrite(&sb,sizeof(sb),1,fp);
  int valid = FALSE;
  for(int i=0;i<block_number;i++){
    fwrite(&valid, sizeof(valid), 1, fp);
  }
  
  
  node.type = TYPE_DIRECTORY;//first node is the root directory
  node.valid = TRUE;//first directory must be valid when the disk is created
  
  fwrite(&node,sizeof(node),1,fp);
  
  node.valid = FALSE;
  for(int i=0;i<inode_number-1;i++){
    fwrite(&node,sizeof(node),1,fp);
  }
  fseek(fp,block_number*SIZE_BLOCK-1,SEEK_CUR);
  fputc(32, fp);
  
  fclose(fp);
  long total_size = SIZE_SUPERBLOCK+
    SIZE_INODE*inode_number+
    block_number*(SIZE_BLOCK+SIZE_TABLE_UNIT);
  
  printf("create disk success\ntotal size: %ldbytes = %.1lfKB = %.1lfMB\n",
	 total_size,
	 (double)total_size/1024,
	 (double)total_size/1024/1024
	 );
  
  return TRUE;
  
}


int format(char* disk_name,filesystem* fs){
  disk* di = find_disk(disk_name, fs);
  if(!di){
    printf("%s\n",DISK_NOT_MOUNT_ERROR);
    return FALSE;
  }
  int size = di->sb->block_number*SIZE_BLOCK+
    di->sb->inode_number*(SIZE_INODE+SIZE_TABLE_UNIT)+
    SIZE_SUPERBLOCK;
  if(!delete(disk_name, fs)){
    return FALSE;
  }
  if(!create(disk_name,size)){
    return FALSE;
  }
  printf("format disk %s success\n",disk_name);
  return TRUE;
}


int delete(char* disk_name,filesystem* fs){
  if(find_disk(disk_name, fs)){
    unmount(disk_name, fs);
  }
  
  int rc = remove(disk_name);
  if(rc){
    printf("%s\n",DELETE_DISK_FAIL_ERROR);
    return FALSE;
  }
  printf("delete disk %s success\n",disk_name);
  return TRUE;
}

int init(filesystem* fs){
    //init attributes
    fs->disks=NULL;
    fs->number_disk = 0;//the number of disk
    fs->current_directory=-1;//means the directory is /dev
    fs->current_disk=NULL;//this means we are now at the root node
    fs->buffer_inode=-1;//the pasted file's inode
    fs->buffer_disk=NULL;//the pasted file's disk

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
    fs->list_disks = list_disks;

    fs->init = init;
    return TRUE;
  }

void list_disks(filesystem* fs){
  disk* di = fs->disks;
  while(di){
    printf("%s\t",di->disk_name);
    di = di->next_disk;
  }
  printf("\n");

}
