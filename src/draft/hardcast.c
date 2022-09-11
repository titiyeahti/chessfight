#include<stdlib.h>
#include<stdio.h>
#include<string.h>

int main(void){
  char* test = "Bonjour";
  int res = *((int*) test);

  printf("%x%x%x%x\n", test[0], test[1], test[2], test[3]); 
  printf("%x\n", res);
  
  char buffer[5];
  int toto = 1 + (2 << 8) + (3 << 16) + (4 << 24);
  memcpy(buffer, &toto, sizeof(int));

  buffer[4] = 0;

  printf("%d\n", toto);

  memcpy(&toto, buffer, sizeof(int));
  
  printf("%d\n", toto);
  printf("%d\n", *((int*)buffer));
  return 0;
}
