#ifndef SERV_H
#define SERV_H

#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<sys/select.h>
#include<netdb.h>
#include<time.h>
#include<errno.h>
#include"chess.h"

#define MAX_NAME_LENGTH 32
#define MAX_PLAYERS 2048
#define MAX_LISTEN 128

typedef enum outcome{
  BLACK_WON = 1,
  WHITE_WON = 2,
  DRAW = 3,
  OPPO_LEFT = 4
} OUTCOME_t;

/* comm protocol 
   first byte = msg type
   if size is fixed, the message immediately follows
   otw 
   the message len is sent in an uint before the message*/

typedef enum msgtype{
  MOVE = 0,       /*len 2 sending (ushort) move*/
  INVALID = -1,   /*len 0*/
  START = 1,      /*len 1 sending color*/
  END = 2,        /*len 1 sending outcome*/
  RECAP = 3,      /*len 2*sizeof clock_t + moves_len*sizeof ushort */
  CHAT = 4        /*len +- strlen but care with null terminating char*/
} MSGTYPE_t;

typedef struct player{
  int socket, index;
  char username[MAX_NAME_LENGTH];
  match_t* m;
} player_t;

typedef player_t* players;

typedef struct match{
  player_t* players[2];
  clock_t timers[2];
  char current_player;
  game_p game;
  clock_t last_write;
  int index;
} match_t;

typedef match_t* match_p;

typedef struct serv{
  player_p players[MAX_PLAYERS];
  match_p matchs[(MAX_PLAYERS+1)/2];
  int socket, nb_players, nb_matchs, max_fd;
} serv_t;

typedef serv_t* serv_p;

int connexion_init(const char* port); 

player_p player_new(void);

int player_insert(serv_p server, player_p player);

serv_p serv_new(const char* port);

int player_connect(serv_p server);

int player_disconnect(serv_p serveur, player_p p);

int match_start(serv_p server, player_p p1, player_p p2);

/*    Pop the match,
      put the two player at the right position in the queue (players), 
      send the recap to players*/
int match_end(serv_p server, match_p m);

#endif
