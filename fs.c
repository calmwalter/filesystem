#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include"fs.h"


int write(char* file_name,char* content,filesystem* fs){
  //if at the root directory, we return FALSE
  if(!fs->current_disk && fs->current_directory==-1){
    printf("CAN'T WRITE FILE AT THE ROOT DIRECTORY\n");
    return FALSE;
  }
  if(!__check_permission(fs->current_disk->inodes+fs->current_directory,fs)){
    printf("%s\n",PERMISSION_DENIED_ERROR);
    return FALSE;
  }
  //find if the file already exist
  int aim = __find_aim(fs->current_directory, file_name, fs->current_disk);
  if(aim!=-1){
    printf("%s\n",NAME_ALREADY_EXIST_ERROR);
    return FALSE;
  }

  //calculate the size of the content
  int content_len = strlen(content);

  //define variables
  disk* di = fs->current_disk;
  int cur_dir = fs->current_directory;
  inode* inodes = di->inodes;

  //find free inode
  inode* ptr;
  int in_num = -1;
  for(int i=0;i<di->sb->inode_number;i++){
    ptr = inodes+i;
    if(ptr->valid==FALSE){
      in_num=i;
      break;
    }
  }
  if(in_num==-1){
    printf("%s\n",NO_ENOUGH_INDOE_SPACE_ERROR);
    return FALSE;
  }



  //add inode for the file
  ptr->valid=TRUE;
  *(ptr->name)='\0';
  strcpy(ptr->name, file_name);
  ptr->type = TYPE_FILE;
  ptr->parent_directory=cur_dir;
  *(ptr->owner)='\0';
  strcpy(ptr->owner,fs->user->name);

  ptr->size = content_len*sizeof(char);//plus one is the \0 end content symbol
  __update_size(ptr->size,cur_dir,di);//update size infomation

  //add to parent node
  __add_inode_pointer(in_num, di, cur_dir);

  ptr->direct[0]=-1;
  ptr->indirect=-1;

  __write_inode_to_disk(di,in_num);

  //find free block
  int number_need_block = (content_len*sizeof(char))/SIZE_BLOCK+
    ((content_len*sizeof(char))%SIZE_BLOCK==0? 0:1);
  int tmp[number_need_block];
  int j=0;
  for(int i=0;i<di->sb->block_number;i++){
    if(di->block_table[i]==FALSE){
      tmp[j++]=i;
      if(j==number_need_block){
	break;
      }
    }
  }
  if(j<number_need_block){
    printf("%s\n",NO_ENOGH_BLOCK_SPEACE_ERROR);
    return FALSE;
  }

  //write file data to the disk
  int block_offset = SIZE_SUPERBLOCK+
    SIZE_TABLE_UNIT*di->sb->block_number+
    SIZE_INODE*di->sb->inode_number;
  FILE* fp = fopen(di->disk_name,"rb+");
  for(int i=0;i<number_need_block;i++){
    int block_number=tmp[i];
    //update block table
    di->block_table[block_number]=TRUE;
    __write_table_to_disk(di, block_number, TRUE);
    //write to inode
    __add_inode_pointer(block_number, di, in_num);
    //write data to file
    int offset = block_offset+SIZE_BLOCK*block_number;
    fseek(fp, offset, SEEK_SET);
    int size = (i==number_need_block-1 ? ptr->size%SIZE_BLOCK:SIZE_BLOCK);
    fwrite(content+i*SIZE_BLOCK, size, 1, fp);
  }
  fclose(fp);
  return TRUE;
}

//can read file, directory, disk, only file return content, others return information
int read(char* file_name,filesystem* fs){
  //check if it's root directory
  if(!fs->current_disk && fs->current_directory==-1){
    printf("%s\n",FILE_NOT_FOUND_ERROR);
    return FALSE;
  }
  int aim = __find_aim(fs->current_directory, file_name, fs->current_disk);
  if(aim==-1){
    printf("%s\n",FILE_NOT_FOUND_ERROR);
    return FALSE;
  }
  inode* in = fs->current_disk->inodes+aim;
  if(in->type==TYPE_DIRECTORY){
    printf("%s\n",FILE_NOT_FOUND_ERROR);
    return FALSE;
  }
  if(!__check_permission(in,fs)){
    printf("%s\n",PERMISSION_DENIED_ERROR);
    return FALSE;
  }

  FILE* fp = fopen(fs->current_disk->disk_name,"rb");
  int block_offset = SIZE_SUPERBLOCK+
    SIZE_TABLE_UNIT*fs->current_disk->sb->block_number+
    SIZE_INODE*fs->current_disk->sb->inode_number;
  int size = in->size;
  char* buffer = (char*)malloc(size+1);
  buffer[0]='\0';
  int block_number = size/SIZE_BLOCK+(size%SIZE_BLOCK==0? 0:1);
  //number of block smaller than NUMBER_DIRECT_POINTER
  if(block_number<=NUMBER_DIRECT_POINTER){
    for(int i=0;i<block_number;i++){
      int block_pos = in->direct[i];
      int offset = block_offset+block_pos*SIZE_BLOCK;
      int read_size = (i==block_number-1 ? size%SIZE_BLOCK:SIZE_BLOCK);
      fseek(fp, offset, SEEK_SET);
      fread(buffer+SIZE_BLOCK*i, read_size, 1, fp);
    }
    fclose(fp);
    buffer[size] = '\0';
    printf("%s\n",buffer);

    return TRUE;
  }

  //read direct pointer
  for(int i=0;i<NUMBER_DIRECT_POINTER;i++){
    int block_pos = in->direct[i];
    int offset = block_offset+block_pos*SIZE_BLOCK;
    int read_size = (i==block_number-1 ? size%SIZE_BLOCK:SIZE_BLOCK);
    fseek(fp, offset, SEEK_SET);
    fread(buffer+SIZE_BLOCK*i, read_size, 1, fp);
  }

  //read indirect pointer
  for(int i=0;i<block_number-NUMBER_DIRECT_POINTER;i++){
    int block_pos=-1;
    int indirect_offset = block_offset + (in->indirect)*SIZE_BLOCK+i*sizeof(int);
    fseek(fp, indirect_offset, SEEK_SET);
    fread(&block_pos, sizeof(int), 1, fp);
    int read_size =
      (i==block_number-NUMBER_DIRECT_POINTER-1 ? size%SIZE_BLOCK:SIZE_BLOCK);

    int offset = block_offset+block_pos*SIZE_BLOCK;
    fseek(fp, offset, SEEK_SET);
    fread(buffer+SIZE_BLOCK*(i+5), read_size, 1, fp);
  }
  fclose(fp);
  buffer[size]='\0';
  printf("%s\n",buffer);
  return TRUE;
}

void ls(filesystem* fs){
  if (!fs->current_disk && fs->current_directory==-1){
    return ;
  }
  
  printf("\n%-30s%-15s%-15s%-30s\n","NAME","TYPE","SIZE","OWNER");
  for (int i = 0; i < 90; i++)
    {
      printf("-");
    }
  printf("\n");

  //find direct pointer
  disk* di = fs->current_disk;
  inode* in = di->inodes+fs->current_directory;
  inode* aim;
  for(int i=0;i<NUMBER_DIRECT_POINTER;i++){
    if(*(in->direct+i)==-1){
      return;
    }
    aim = di->inodes+(*(in->direct+i));
    printf("%-30s%-15d%-15d%-30s\n",aim->name,aim->type,aim->size,aim->owner);
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
        offset = block_offset+SIZE_BLOCK*(in_num-1);
        fread(&in_num, offset, 1, fp);

      }
      if(in_num==-3){
        continue;
      }
      if(in_num==-1){
	return;
      }
      aim = di->inodes+in_num;
      printf("%-30s%-15d%-15d%-30s\n",aim->name,aim->type,aim->size,aim->owner);
    }
  }
  printf("\n");
}



int cp(char* path,filesystem* fs){
  //first get list
  path_list* pl = __get_path_list(path);

  //find the aim
  position* pos = __search_position(pl, fs);
  if(!pos){
    return FALSE;
  }
  fs->buffer_disk = pos->di;
  fs->buffer_inode = pos->position;
  return TRUE;
}

int paste(filesystem* fs){}



int rm(char* path,filesystem* fs){
  //first get list
  path_list* pl = __get_path_list(path);

  //find the aim
  position* pos = __search_position(pl, fs);
  if(!pos){
    return FALSE;
  }
  disk* di = pos->di;
  int aim = pos->position;
  if (!__check_permission(di->inodes+aim,fs)){
    printf("%s\n",PERMISSION_DENIED_ERROR);
    return FALSE;
  }
  
  //set invalid
  (di->inodes+aim)->valid = FALSE;
  //delete the parent pointer
  int par_aim = (di->inodes+aim)->parent_directory;
  __set_inode_pointer(aim, -3, par_aim, di);

  //write to file
  __write_inode_to_disk(di, aim);
  __write_inode_to_disk(di, (di->inodes+aim)->parent_directory);
  return TRUE;
}


//TODO achieve multi disk operation
int mv(char* file_path, char* dest_file_path,filesystem* fs){
  //get file path list
  path_list* pl_aim = __get_path_list(file_path);
  //get aim inode
  position* pos_aim = __search_position(pl_aim, fs);
  if(!pos_aim){
    printf("%s\n",PATH_NOT_FOUND_ERROR);
    return FALSE;
  }
  if (!__check_permission(pos_aim->di->inodes+pos_aim->position,fs)){
    printf("%s\n",PERMISSION_DENIED_ERROR);
    return FALSE;
  }
  //get dest path list
  path_list* pl_dest = __get_path_list(dest_file_path);
  //get dest directory inode
  position* pos_dest = __search_position(pl_dest, fs);
  if(!pos_dest || (pos_dest->di->inodes+pos_dest->position)->type == TYPE_FILE){
    printf("%s\n",PATH_NOT_FOUND_ERROR);
    return FALSE;
  }

  //update the inode
  inode* in_aim = pos_aim->di->inodes+pos_aim->position;

  //set the parent pointer to be -3
  __set_inode_pointer(pos_aim->position, BLOCK_POSITION_NULL, in_aim->parent_directory, pos_aim->di);
  //change the parent pointer to the new one
  in_aim->parent_directory = pos_dest->position;
  //add the inode pointer to the new parent directory
  __add_inode_pointer(pos_aim->position, pos_dest->di, pos_dest->position);

  //update three inodes to disk
  __write_inode_to_disk(pos_aim->di, pos_aim->position);
  __write_inode_to_disk(pos_aim->di, in_aim->parent_directory);
  __write_inode_to_disk(pos_dest->di, pos_dest->position);
  return TRUE;
}

int mkdir(char* directory_name,filesystem* fs){
  if (!fs->current_disk && fs->current_directory==-1)
    {
      printf("CAN'T CREATE FILE OR DIRECTORY IN ROOT DIRECTORY\n");
      return FALSE;
    }
  // if(!__check_permission(fs->current_disk->inodes+fs->current_directory,fs)){
  //   printf("%s\n",PERMISSION_DENIED_ERROR);
  //   return FALSE;
  // }
  //check if the name already exist
  disk* cur_di = fs->current_disk;
  int cur_dir = fs->current_directory;
  inode* in = cur_di->inodes;
  inode* cur_in = in+cur_dir;
  inode* aim;
  int i=0;

  //->check direct file
  while(cur_in->direct[i]!=-1 && i<NUMBER_DIRECT_POINTER){
    if(cur_in->direct[i]==-3){
      continue;
    }
    aim = in+cur_in->direct[i];
    if(!strcmp(aim->name,directory_name)){
      printf("%s\n",NAME_ALREADY_EXIST_ERROR);
      return FALSE;
    }
    i++;
  }

  if(cur_in->direct[i]!=-1 && cur_in->indirect!=-1){
    //->check indirect file
    i = 0;
    //-->read disk get aim pointer
    long block_offset = SIZE_SUPERBLOCK+
      SIZE_TABLE_UNIT*(cur_di->sb->block_number)+
      SIZE_INODE*(cur_di->sb->inode_number);

    long offset = block_offset+SIZE_BLOCK*(cur_in->indirect);
    FILE* fp = fopen(cur_di->disk_name,"rb");
    fseek(fp, offset,SEEK_SET);
    int aim_dir=-1;
    while(1){
      fread(&aim_dir, sizeof(int), 1, fp);
      if(aim_dir == NEXT_BLOCK){
        aim_dir = fread(&aim_dir, sizeof(int), 1, fp);
        fclose(fp);
        offset = block_offset+SIZE_BLOCK*(aim_dir);
        fp = fopen(cur_di->disk_name,"rb");
        fseek(fp, offset, SEEK_SET);
        aim_dir = fread(&aim_dir, sizeof(int), 1, fp);

      }
      if(aim_dir == END_BLOCK){
        break;
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
      strcpy(new_in->owner,fs->user->name);
      new_in->valid=TRUE;
      __add_inode_pointer(i,cur_di,cur_dir);//add to parent inode pointer

      //write to disk
      __write_inode_to_disk(cur_di, cur_dir);
      __write_inode_to_disk(cur_di, i);
      return TRUE;
    }
  }

  printf("%s\n",NO_ENOUGH_INDOE_SPACE_ERROR);
  return FALSE;
}



//find all file or directory under current directory use recursive descent
void find(char* name,int dir,disk* di, filesystem* fs){
  if(fs->current_directory==dir){
    printf("\n%-30s%-15s%-15s%-50s\n","NAME","TYPE","SIZE","PATH");
    for (int i = 0; i < 110; i++)
      {
	printf("-");
      }
    printf("\n");
  }
  if(!di && dir==-1){
    disk* pt=fs->disks;
    while (pt)
      {
	find(name,0,pt,fs);
	pt=pt->next_disk;
      }
    return;
  }
  //find direct pointer
  inode* in = di->inodes+dir;
  inode* aim;
  for(int i=0;i<NUMBER_DIRECT_POINTER;i++){
    if(*(in->direct+i)==-1){
      return;
    }
    if(*(in->direct+i)==-3){
      continue;
    }
    aim = di->inodes+(*(in->direct+i));
    if(!strcmp(aim->name,name)){
      //TODO: find file path
      char* file_path = __get_path(di,*(in->direct+i));
      printf("%-30s%-15d%-15d%-50s\n",aim->name,aim->type,aim->size,file_path);
      free(file_path);
    }
    if(aim->type==TYPE_DIRECTORY){
      find(name,*(in->direct+i),di,fs);
    }
  }
  if(in->indirect==-1){return;}
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
      fseek(fp,offset,SEEK_SET);
      fread(&in_num, sizeof(int), 1, fp);
    }
    if(in_num == -3){
      continue;
    }
    if(in_num == -1){
      return;
    }
    aim = di->inodes+in_num;
    if(!strcmp(aim->name,name)){
      char* file_path = __get_path(di,in_num);
      printf("%-30s%-15d%-15d%-50s\n",aim->name,aim->type,aim->size,file_path);
      free(file_path);
    }
    if(aim->type==TYPE_DIRECTORY){
      find(name,in_num,di,fs);
    }
  }
  return;
}





int cd(char* path,filesystem* fs){
  //first get list
  path_list* pl = __get_path_list(path);

  //find the directory
  position* pos = __search_position(pl, fs);
  if (!pos){
    printf("%s\n",PATH_NOT_FOUND_ERROR);
    return FALSE;
  }

  if(!pos->di && pos->position!=-1 && (pos->di->inodes+pos->position)->type != TYPE_DIRECTORY){
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
  while(!(di->disk_name=(char*)malloc(sizeof(char)*(len+1))));
  strcpy(di->disk_name, disk_name);

  di->sb = sb;
  di->inodes = in;
  di->block_table = bt;
  di->next_disk = fs->disks;
  fs->disks = di;

  fs->number_disk++;
  printf("mount disk %s success\nused: %d/%d bytes\n",disk_name,di->inodes->size,sb->size);
  return TRUE;
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
      __free_disk(dp);
      return TRUE;
    }
    dpt = dp;
    dp = dp->next_disk;
  }
  printf("%s\n",DISK_NAME_NOT_FOUND_ERROR);
  return FALSE;
}


//TODO: rewrite this part
int create(char* disk_name,int size,filesystem* fs){
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
  sb.size = size;
  // sb.inode_size = 0;
  sb.inode_number = inode_number;
  // sb.block_size = 0;
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
  //first initial the disk size
  fp = fopen(disk_name,"wb");
  fseek(fp,size-1,SEEK_SET);
  fputc(32, fp);
  fclose(fp);
  //then write to the disk
  fp = fopen(disk_name,"rb+");
  fwrite(&sb,sizeof(sb),1,fp);
  int valid = FALSE;
  for(int i=0;i<block_number;i++){
    fwrite(&valid, sizeof(valid), 1, fp);
  }


  node.type = TYPE_DIRECTORY;//first node is the root directory
  node.valid = TRUE;//first directory must be valid when the disk is created
  node.direct[0]=-1;
  node.indirect=-1;
  node.parent_directory=-1;
  node.size=0;
  strcpy(node.name,disk_name);

  fwrite(&node,sizeof(node),1,fp);

  node.valid = FALSE;
  for(int i=0;i<inode_number-1;i++){
    fwrite(&node,sizeof(node),1,fp);
  }
  fseek(fp,block_number*SIZE_BLOCK-1,SEEK_CUR);
  fputc(32, fp);

  fclose(fp);
  // long total_size = SIZE_SUPERBLOCK+
  //   SIZE_INODE*inode_number+
  //   block_number*(SIZE_BLOCK+SIZE_TABLE_UNIT);

  printf("create disk success\ntotal size: %ldB = %.1lfKB = %.1lfMB\n",
	 size,
	 (double)size/1024,
	 (double)size/1024/1024
	 );

  return TRUE;

}


int format(char* disk_name,filesystem* fs){
  if(fs->user->authority!=ADMINISTRATOR){
    printf("%s\n",PERMISSION_DENIED_ERROR);
    return FALSE;
  }
  disk* di = __find_disk(disk_name, fs);
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
  if(!create(disk_name,size,fs)){
    return FALSE;
  }
  printf("format disk %s success\n",disk_name);
  return TRUE;
}


int delete(char* disk_name,filesystem* fs){
  if(__find_disk(disk_name, fs)){
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
