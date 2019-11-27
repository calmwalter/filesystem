
#include"fs.h"
#include<stdio.h>



int main(){
  superblock sb;
  sb.inode_size=100;
  sb.inode_number=100;
  sb.block_number=100;
  sb.block_size=100;
  printf("size of superblock:%d\n",sizeof(sb));

  FILE *fp = fopen("test","wb");
  fwrite(&sb,sizeof sb,1,fp);
  fclose(fp);
  
  superblock sb1;
  fp = fopen("test","rb");
  fread(&sb1,sizeof sb1,1,fp);
  fclose(fp);
  printf("%d",sb1.inode_size);
  return 0;
}
