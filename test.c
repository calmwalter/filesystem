#include"fs.h"
#include<stdio.h>
#include<string.h>

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
  account ac;
  ac.authority=ADMINISTRATOR;
  strcpy(ac.name,"root");
  strcpy(ac.password,"root");
  filesystem fs;
  init(&fs);
  fs.user=&ac;

  fs.create("hello", 1024*1024*128,&fs);
  fs.mount("hello", &fs);
  //fs.list_disks(&fs);
  //fs.unmount("hello",&fs);
  //fs.list_disks(&fs);
  //fs.format("hello",&fs);
  //fs.delete("hello",&fs);
  fs.cd("/hello",&fs);
  printf("%d\n",fs.current_directory);
  fs.mkdir("halo",&fs);
  fs.mkdir("yolo",&fs);
  fs.mkdir("champion",&fs);
  fs.mkdir("cascadia",&fs);
  fs.mkdir("calm",&fs);
  fs.write("dotaeee","fsdfsd",&fs);
  fs.ls(&fs);
  // fs.mkdir("walter",&fs);
  // fs.mkdir("dota",&fs);

  // fs.cd("/hello/walter",&fs);
  // fs.mkdir("walter",&fs);
  // fs.mkdir("sam",&fs);
  // fs.mkdir("kola",&fs);
  // printf("%d\n",fs.current_directory);
  // printf("%s\n",__get_path(fs.current_disk,fs.current_directory));
  // fs.ls(&fs);
  // fs.cd("/",&fs);
  // fs.find("walter",fs.current_directory,fs.current_disk,&fs);
  // fs.cd("/hello/walter",&fs);
  // fs.write("html", "sjdghfefegafuerfjedbfaergfkrbfhdbvagfkuabedfhasdgfkurefudbfhjfgke", &fs);
  // fs.read("html", &fs);
  
  return 0;
}
