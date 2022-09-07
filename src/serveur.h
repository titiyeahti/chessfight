#ifndef SERVER_H
#define SERVER_H
#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netdb.h>
#include<time.h>
#include"chess.h"

#define NAME_SIZE 128

/* add more if needed */
struct player{
  int socket;
  char name[NAME_SIZE];
} player_t;

typedef player_t* player_p;

int game(player_p p1, player_p p2);

/* while(1) 
    wait for connections
    if any 
      */
int app(void);

int  

#endif 
