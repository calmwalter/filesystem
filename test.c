#include"fs.h"
#include<stdio.h>


int main(int argc, char **argv){
  /*
  printf("sizeof int: %d\n",sizeof(int));
  printf("sizeof char: %d\n",sizeof(char));
  printf("sizeof int*: %d\n",sizeof(int*));
  printf("sizeof char*: %d\n",sizeof(char*));
  
  superblock sb;
  sb.inode_size=100;
  sb.inode_number=100;
  sb.block_number=100;
  sb.block_size=100;
  printf("size of superblock: %d bytes\n",sizeof(sb));

  inode node;
  //node.number=12345;
  node.type = TYPE_FILE;
  printf("size of inode: %d bytes\n",sizeof(node));
  
  FILE *fp = fopen("test","wb");
  fwrite(&sb,sizeof sb,1,fp);
  fwrite(&node,sizeof node,10000,fp);
  fclose(fp);
  
  superblock sb1;
  inode node1;
  fp = fopen("test","rb");
  fread(&sb1,sizeof sb1,1,fp);
  fread(&node1,sizeof node1,1,fp);
  fclose(fp);
  
  printf("%d\n",sb1.inode_size);
  printf("%d\n",node1.type);

  create("hello",1024*1024*128);
  printf("%d\n",sizeof(char));
  */
  
  ////////////////////////////////////

  //test filesystem
  filesystem fs;
  init(&fs);
  fs.create("hello", 1024*1024*128);
  fs.mount("hello", &fs);
  //fs.list_disks(&fs);
  //fs.unmount("hello",&fs);
  //fs.list_disks(&fs);
  //fs.format("hello",&fs);
  //fs.delete("hello",&fs);
  fs.cd("hello",&fs);
  return 0;
}
