/*
  all right reserved 2019 calmwalter

-------------------------------------------------------------
  we define the file structure like this:
  /dev/disk_name/files or directory/..../files or directory
  /dev is the root
  /disk_name is the name of the disk
--------------------------------------------------------------
  and I construct the disk like this:
  +-------------------------------------------+
  | super block | block table | inode | block |
  +-------------------------------------------+

 */

#ifndef FS_H
#define FILESYSTEM_H

//type of the file
#define TYPE_FILE 0
#define TYPE_DIRECTORY 1

//validation
#define TRUE 1
#define FALSE 0

//define number of direct pointer point to the data
#define NUMBER_DIRECT_POINTER 5

//define the size, unit byte
//define inode size
#define SIZE_INODE 76
//define block size
#define SIZE_BLOCK 1024
//define superblock size
#define SIZE_SUPERBLOCK 16
//define table size of each unit
#define SIZE_TABLE_UNIT 4

//define error
#define DISK_NAME_EXIST_ERROR "ERROR: DISK NAME ALREADY EXISTS"

//store the main infomation of the disk
typedef struct superblock{
  int inode_size;//the size of the file system
  int inode_number;
  int block_size;
  int block_number;
}superblock;

//store the file infomation
typedef struct inode{
  int number;//the number of the inode, we use this to find the value of the file
  int parent_directory;//pointer point to the parent directory inode
  char name[30];//name of the file
  int type;//type of the file
  int size;//size of the data
  int valid;//if the block has been used
  int direct[NUMBER_DIRECT_POINTER];//pointer point to the data directly
  int indirect;//point to block where store the pointer point to the data
}inode;

//the disk that store the disk infomation:name,superblock,inode,and the next disk link
typedef struct disk{
  int number;
  char* disk_name;//the name of the disk
  superblock* sb;//the superblock infomation of the disk
  inode* inodes;//the inodes in the disk
  int* block_table;//record the validation of the block and 
}disk;

typedef struct filesystem
{
  //attributes of filesystem
  disk** disks;//dynamicly store the mounted disk
  int number_disk;//number of disk that is mounted
  int current_directory;//current directory number
  int current_disk;//the disk we current use
  int buffer_inode,buffer_disk;//buffer_inode point to the inode number,buffer_disk point to the disk number
  
  //operation to access disk
  int (*write)(char*,char*,struct filesystem*);//write to the file(file_path,content)
  int (*read)(char*,struct filesystem*);//read the file(file_path)
  int (*ls)(char*,struct filesystem*);//list the files and directories in current directory(file_path)
  int (*cp)(char*,struct filesystem*);//copy a file, put it to the buffer(file_path)
  int (*paste)(struct filesystem*);//paste the file in buffer to current directory
  int (*rm)(char*,struct filesystem*);//rm the file(file_path)
  int (*mv)(char*,char *,struct filesystem*);//move a file to another place(file_path,dest_file_path)
  int (*mkdir)(char*,struct filesystem*);//make a directory in current directory(directory_name)
  int (*find)(char*,struct filesystem*);//find the file or directory under current directory or child directory recursively(file or directory name)
  int (*cd)(char*,struct filesystem*);//go to the directory
  
  //operation to manage disk
  int (*mount)(char*,struct filesystem*);//mount the disk(disk_name)
  int (*unmount)(char*,struct filesystem*);//unmount the disk(disk_name)
  int (*create)(char*,int);//create the disk(disk_name,size)
  int (*format)(char*,struct filesystem*);//format the disk(disk_name)
  int (*delete)(char*,struct filesystem*);//delete the disk(disk_name)

  //init the file system
  int (*init)(struct filesystem*);//init the file system
}filesystem;

int write(char* file_path,char* content,filesystem* fs);
int read(char* file_path,filesystem* fs);
int ls(char* file_path,filesystem* fs);
int cp(char* file_path,filesystem* fs);
int paste(filesystem* fs);
int rm(char* file_path,filesystem* fs);
int mv(char* file_path, char* dest_file_path,filesystem* fs);
int mkdir(char* directory_name,filesystem* fs);
int find(char* name,filesystem* fs);
int cd(char* path,filesystem* fs);


int mount(char* disk_name,filesystem* fs);
int unmount(char* disk_name,filesystem* fs);
int create(char* disk_name,int size);
int format(char* disk_name,filesystem* fs);
int delete(char* disk_name,filesystem* fs);


int init(filesystem* fs);


#endif
