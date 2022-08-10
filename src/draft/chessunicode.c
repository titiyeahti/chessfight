#include<stdlib.h>
#include<stdio.h>
#include<wchar.h>
#include<locale.h>

void main(void){
  int i;
  wchar_t pieces[13];

  setlocale(LC_CTYPE, "");
  for(i=0; i<12; i++)
    pieces[i] = 0x2654 + i;

  pieces[12] = '\0';

  wprintf(L"%ls\n", pieces);
}
