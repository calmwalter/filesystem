#include<stdio.h>
#include<string.h>
#include<stdlib.h>
int main(){
    char* a = (char*)malloc(sizeof(char)*5);
    a = "helloword";
    char* b = (char*)malloc(sizeof(char)*6);
    strncpy(b,a,6);
    printf("%s\n",b);
    return 0;
}