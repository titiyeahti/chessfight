
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<sys/types.h>
#include<unistd.h>

int main(int argc, char* argv[]){
  char buffer[1024];
  char temp;
  int len, i;

  for(;;){
    if((len=read(STDIN_FILENO, buffer, 1024)) < 0)
      return EXIT_FAILURE;

    buffer[len] = '\0';

    if(!strcmp(buffer, "exit\n"))
      return EXIT_SUCCESS;

    for(i=0;i<(len-1)/2;i++){
      temp = buffer[i];
      buffer[i] = buffer[len-2-i];
      buffer[len-2-i] = temp;
    }

    if(write(STDOUT_FILENO, buffer, len)<0)
      return EXIT_FAILURE;
  }
}

