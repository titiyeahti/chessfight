#include <stdio.h>
#include <stdlib.h>

void frint(int* a){
  free(a);
  a = NULL;
}

void main(void){
  int* a = malloc(sizeof(int));
  int* b = a;

  *a = 10;

  frint(a);

  int* c = malloc(sizeof(int));

  *c = 127;

  printf("%p %p %p\n", a, b, c);
  printf("%d\n", *a);
}
