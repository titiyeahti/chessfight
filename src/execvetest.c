
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<sys/types.h>
#include<unistd.h>

int main(int argc, char* argv[]){
  char buffer[1024];
  char reffub[1024];
  int len, i;
  fprintf(stderr, "INSIDE PROG\n");

  for(;;){
    if((len=read(STDIN_FILENO, buffer, 1024)) < 0)
      return EXIT_FAILURE;

    buffer[len] = '\0';

    if(!strcmp(buffer, "exit"))
      return EXIT_SUCCESS;

    for(i=0; i<len; i++)
      reffub[i] = buffer[len-1-i];

    reffub[len] = '\0';

    if(write(STDOUT_FILENO, reffub, len)<0)
      return EXIT_FAILURE;
  }
}

