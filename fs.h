#ifndef FS_H
#define FS_H

int disk_size = 1024;//the size of the disk,unit kb
int block_size = 4;//the size of the block,unit kb

//type of the file
#define TYPE_FILE 0
#define TYPE_DIRECTORY 1

//validation
#define TRUE 1
#define FALSE 0

//define number of direct pointer point to the data
#define NUMBER_DIRECT_POINTER


//store the main infomation of the disk
typedef struct superblock{
  int magic_number;//the number of the disk
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
  int direct[5];//pointer point to the data directly
  int indirect;//point to block where store the pointer point to the data
}inode;

typedef struct fs
{
  int current_directory;//current directory number
  int buffer;//buffer point to the inode
  int (*write)(char*,char*);//write to the file(file_path,content)
  int (*read)(char*);//read the file(file_path)
  int (*ls)(char*);//list the files and directories in current directory(file_path)
  int (*cp)(char*);//copy a file, put it to the buffer(file_path)
  int (*paste)(char*);//paste the file in current buffer
  int (*rm)(char*);//rm the file(file_path)
  int (*mv)(char*,char *);//move a file to another place(file_path,dest_file_path)
  int (*mkdir)(char*);//make a directory in current directory(directory_name)
  int (*find)(char*);//find the file or directory under current directory or child directory recursively(file or directory name)
  int (*init)();//init the file system, load the root directory
  
};



#endif
