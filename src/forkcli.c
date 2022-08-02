/* Usage :
   compile with :
    cc execvetest.c -o prog;
   to test :
    cc forkcli.c -o forkcli; ./forkcli ./prog 
 */

#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<unistd.h>
#include<errno.h>

int main(int argc, char* argv[]){
  pid_t fpid;
  /* child -> p1 -> parent 
     parent -> p2 -> child */
  int p1[2];
  int p2[2];
  int i, len;
  char* earg[argc-1];
  char* envp[] = {NULL};
  char buffer[1024];

  if(argc < 2){
    printf("Wrong number of arguments. Usage : ./forkcli program_path"
        " [argv1 ...]\n");
    return EXIT_FAILURE;
  }

  if(pipe(p1) < 0) 
    return EXIT_FAILURE;

  if(pipe(p2) < 0){
    close(p1[0]);
    close(p1[1]);
    return EXIT_FAILURE;
  }

  fpid = fork();
  if(fpid < 0){
    close(p1[0]);
    close(p1[1]);
    close(p2[0]);
    close(p2[1]);
    return EXIT_FAILURE;
  } 
  else if (fpid == 0) {
    /* Inside child */
    close(p1[1]);
    close(p2[0]);

    close(STDIN_FILENO);
    close(STDOUT_FILENO);

    if(dup2(p2[1], STDOUT_FILENO)<0)
      return EXIT_FAILURE;
    if(dup2(p1[0], STDIN_FILENO)<0)
      return EXIT_FAILURE;

    for(i=0; i<argc-2; i++){
      earg[i] = argv[i+2];
    }

    earg[argc-2] = NULL;

    if(execve(argv[1], earg, envp) < 0){
      return EXIT_FAILURE;
    }
  }
  else {
    /* in the parent process */
    close(p1[0]);
    close(p2[1]);

    for(;;){
      if(!fgets(buffer, 1024, stdin))
        continue;

      len = strlen(buffer)-1;
      buffer[len] = '\0';

      if(write(p1[1], buffer, len) < 0)
        return EXIT_FAILURE;

      if(!strcmp(buffer, "exit"))
        break;

      if((len = read(p2[0], buffer, 1024)) < 0)
        return EXIT_FAILURE;

      buffer[len] = '\0';
      printf("Response : %s\n", buffer);
    }

    /*awaiting for the child to end*/
    wait(NULL);

    return EXIT_SUCCESS;
  }
}

