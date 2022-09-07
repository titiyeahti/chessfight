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
#define MAX_PLAYERS (FD_SETSIZE - 16)
#define MAX_LISTEN 128
#define BUFSIZE 4096

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
  INVALID = 0,    /*len 0*/
  MOVE = 1,       /*len 2 sending (ushort) move*/
  START = 2,      /*len 1 sending color*/
  END = 3,        /*len 1 sending outcome*/
  RECAP = 4,      /*len 2*sizeof clock_t + moves_len*sizeof ushort */
  CHAT = 5,       /*len +- strlen but care with null terminating char*/
  IDENTITY = 6    /*len max_name_lenght BUT this may change if we want to 
                      add mode info to player later, such as human or anything*/
} MSGTYPE_t;

typedef struct player player_t;
typedef struct match match_t;
typedef player_t* player_p;
typedef match_t* match_p;

struct player{
  int socket, index;
  char username[MAX_NAME_LENGTH];
  match_p m;
};

struct match{
  player_p players[2];
  clock_t timers[2];
  COLOUR_t current_player;
  game_p g;
  clock_t last_write;
};

typedef struct serv{
  player_p players[MAX_PLAYERS];
  match_p matchs[(MAX_PLAYERS+1)/2];
  int socket, nb_players, nb_matchs, max_fd;
} serv_t;

typedef serv_t* serv_p;

/* SERVER */

int connexion_init(const char* port); 

serv_p serv_new(const char* port);

/* PLAYER */
player_p player_new(void);

void player_free(player_p p);

int player_queue_up(serv_p server, player_p player);

int player_connect(serv_p server);

int player_disconnect(serv_p serveur, player_p p);

/* MATCH */

match_p match_new(void);

void match_free(match_p m);

int match_start(serv_p server, player_p p1, player_p p2);

/*    Pop the match,
      put the two player at the right position in the queue (players), 
      send the recap to players*/
int match_end(serv_p server, match_p m);

/* MESSAGES */
ssize_t message_read(int sock, void* buffer, size_t bufsize);

int message_write(int sock, char* buffer);

#endif
