#include<stdio.h>

int main(){
  int a=5;
  int b=100;
  FILE* fp = fopen("hello","rb+");
  if(!fp){
    printf("open stream failed\n");
    return 0;
  }
  fseek(fp, 0, SEEK_SET);
  fwrite(&a, sizeof(int), 1, fp);
  fclose(fp);

  
  
  return 0;
}
