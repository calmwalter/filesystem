/*
  all right reserved 2019 calmwalter

-------------------------------------------------------------
  we define the file structure like this:
  /disk_name/files or directory/..../files or directory
  first / is the root
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
#define SIZE_INODE 72
//define block size
#define SIZE_BLOCK 1024
//define superblock size
#define SIZE_SUPERBLOCK 12
//define table size of each unit
#define SIZE_TABLE_UNIT 4

//define value operation for directory file pointer table
#define END_BLOCK -1
#define NEXT_BLOCK -2
#define BLOCK_POSITION_NULL -3

//define error
#define DISK_NAME_EXIST_ERROR "ERROR: DISK NAME ALREADY EXISTS"
#define DISK_ALREADY_MOUNT_ERROR "ERROR: DISK ALREADY MOUNTED"
#define DISK_NOT_EXIST_ERROR "ERROR: DISK NOT EXISTS"
#define MEMORY_ALLOCATION_FAIL_ERROR "ERROR: MEMORY ALLOCATION FAILED"
#define DISK_SUPERBLOCK_READ_FAIL_ERROR "ERROR: READ DISK SUPERBLOCK FAILED"
#define DISK_BLOCK_TABLE_READ_FAIL_ERROR "ERROR: READ DISK BLOCK TABLE FAILED"
#define DISK_INODE_READ_FAIL_ERROR "ERROR: READ DISk INODE FAILED"
#define DISK_NOT_MOUNT_ERROR "ERROR: DISK NOT MOUNTED"
#define NO_DISK_MOUNT_ERROR "ERROR: NO DISK MOUNTED"
#define DISK_NAME_NOT_FOUND_ERROR "ERROR: DISK NAME NOT FOUND"
#define DELETE_DISK_FAIL_ERROR "ERROR: DELETE DISK FAILED"
#define PATH_NOT_FOUND_ERROR "ERROR: PATH NOT FOUND"
#define FILE_NOT_FOUND_ERROR "ERROR: FILE NAME NOT FOUND"
#define NAME_ALREADY_EXIST_ERROR "ERROR: NAME ALREADY EXISTS"
#define NO_ENOUGH_INDOE_SPACE_ERROR "ERROR: NO ENOUGH INDOE SPACE"
#define NO_ENOGH_BLOCK_SPEACE_ERROR "ERROR: NO ENOUGH BLOCK SPACE"

//define authority
#define ADMINISTRATOR 0
#define USER 1

//define account
typedef struct account{
  char name[30];
  char password[30];
  int authority;
}account;
account* verify(char* name,char* password);
int useradd(char* name,char* password, int authority);


//store the main infomation of the disk
typedef struct superblock{
  int size;
  // int inode_size;//total used inode size
  int inode_number;//number of inodes
  // int block_size;//total used block size
  int block_number;//number of blocks
}superblock;

//store the file infomation
typedef struct inode{
  //int number;//the number of the inode, we use this to find the value of the file
  int parent_directory;//pointer point to the parent directory inode
  char name[30];//name of the file
  int type;//type of the file
  int size;//size of the data
  int valid;//if the block has been used
  int direct[NUMBER_DIRECT_POINTER];//pointer point to the data(file) or inode(directory) directly
  int indirect;//point to block where store the pointer point to the data or inode, end with -1, end with -2, read one more for next block.
}inode;

//the disk that store the disk infomation:name,superblock,inode,and the next disk link
typedef struct disk{
  char* disk_name;//the name of the disk
  superblock* sb;//the superblock infomation of the disk
  inode* inodes;//the inodes in the disk
  int* block_table;//record the validation of the block and
  struct disk* next_disk;// next linked disk
}disk;

typedef struct filesystem
{
  //attributes of filesystem
  disk* disks;//dynamicly store the mounted disk using linked list
  int number_disk;//number of disk that is mounted
  int current_directory;//current directory number
  disk* current_disk;//the disk we current use
  int buffer_inode;disk* buffer_disk;//buffer_inode point to the inode number,buffer_disk point to the disk number
  account* user;//the user that loaded
  
  //operation to access disk
  int (*write)(char*,char*,struct filesystem*);//write to the file(file_path,content)
  int (*read)(char*,struct filesystem*);//read the file(file_path)
  void (*ls)(struct filesystem*);//list the files and directories in current directory(file_path)
  int (*cp)(char*,struct filesystem*);//copy a file, put it to the buffer(file_path)
  int (*paste)(struct filesystem*);//paste the file in buffer to current directory
  int (*rm)(char*,struct filesystem*);//rm the file(file_path)
  int (*mv)(char*,char *,struct filesystem*);//move a file to another place(file_path,dest_file_path)
  int (*mkdir)(char*,struct filesystem*);//make a directory in current directory(directory_name)
  void (*find)(char*,int,disk*,struct filesystem*);//find the file or directory under current directory or child directory recursively(file or directory name)
  int (*cd)(char*,struct filesystem*);//go to the directory
  
  //operation to manage disk
  int (*mount)(char*,struct filesystem*);//mount the disk(disk_name)
  int (*unmount)(char*,struct filesystem*);//unmount the disk(disk_name)
  int (*create)(char*,int);//create the disk(disk_name,size)
  int (*format)(char*,struct filesystem*);//format the disk(disk_name)
  int (*delete)(char*,struct filesystem*);//delete the disk(disk_name)
  void (*list_disks)(struct filesystem*);//list disks
  //init the file system
  int (*init)(struct filesystem*);//init the file system
}filesystem;

int write(char* file_path,char* content,filesystem* fs);
int read(char* file_path,filesystem* fs);
void ls(filesystem* fs);
int cp(char* file_path,filesystem* fs);
int paste(filesystem* fs);
int rm(char* file_path,filesystem* fs);
int mv(char* file_path, char* dest_file_path,filesystem* fs);
int mkdir(char* directory_name,filesystem* fs);
void find(char* name, int cur_dir,disk* di,filesystem* fs);
int cd(char* path,filesystem* fs);


int mount(char* disk_name,filesystem* fs);
int unmount(char* disk_name,filesystem* fs);
int create(char* disk_name,int size);
int format(char* disk_name,filesystem* fs);
int delete(char* disk_name,filesystem* fs);
void list_disks(struct filesystem* fs);

int init(filesystem* fs);

//tools
disk* find_disk(char* disk_name, filesystem* fs);
int find_aim(int cur_dir, char* aim_name, disk* di);
int find_dir(int cur_dir,char* aim_dir_name,disk* di);

typedef struct path_list{
  char** list;
  int len;
}path_list;
path_list* get_path_list(char* path);//remember to release the pointer
char* get_path(disk* di, int pos);//get the path of the position

typedef struct position{
  disk* di;
  int position;
}position;
position* search_position(path_list* pl, filesystem* fs);
//set the node state, -1 represent end,FALSE, -2 represent NEXT_BLOCK, -3 represent NULL pointer
void set_inode_pointer(int cur_value, int set_value, int position, disk* di);

void write_inode_to_disk(disk* di,int position);
int add_inode_pointer(int value, disk* di, int pos);
void write_table_to_disk(disk* di, int pos, int valid);
#endif
