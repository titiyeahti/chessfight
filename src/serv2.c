#include "serv2.h"

/** Init connexion
  * Connect player
  * Server loop
  * Client loop
  * Write UTILS.h and CONSTANT_VALUES.def
  * Uniformize error managment (errno.h + macro in utils.h)
  */


/* SERVER */

int connexion_init(const char* port){
  int sock, s;
  struct addrinfo hints = {0}, *results, *rp;

  hints.ai_family = AF_UNSPEC;      /* Allow IPv4 or IPv6 */
  hints.ai_socktype = SOCK_STREAM;  /* Stream socket */
  hints.ai_flags = AI_PASSIVE;      /* Wildcard */
  hints.ai_protocol = 0;            /* Any protocol */
  hints.ai_canonname = NULL;
  hints.ai_addr = NULL;
  hints.ai_next = NULL;
  
  s = getaddrinfo(NULL, port, &hints, &results);
  if(s != 0){
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
    return -1;
  }

  for(rp = results; rp != NULL; rp = rp->ai_next){
    sock = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);

    if(sock < 0)
      continue;

    if(bind(sock, rp->ai_addr, rp->ai_addrlen) == 0)
      break;            /*victory*/

    close(sock);
  }

  if(rp == NULL){               /* No address succeeded */
    freeaddrinfo(results);
    fprintf(stderr, "Could not connect\n");
    return -1;
  }

  freeaddrinfo(results);

  if(listen(sock, MAX_LISTEN) < 0){
    close(sock);
    fprintf(stderr, "Deaf program cannot listen to others\n");
    return -1;
  }

  return sock;
}

serv_p serv_new(const char* port){
  int sock;
  serv_p ret;

  if((sock = connexion_init(port)) < 0){
    fprintf(stderr, "serv_new\n");
    return NULL;
  }

  ret = malloc(sizeof(serv_t));
  ret->nb_players = 0;
  ret->nb_matchs = 0;
  ret->socket = sock;
  ret->max_fd = sock;

  return ret;
}

void serv_free(serv_p server){
  int i;
  for(i=0; i<server->nb_players; i++){
    player_free(server->players[i]);
    server->players[i] = NULL;
  }

  for(i=0; i<server->nb_matchs; i++){
    match_free(server->matchs[i]);
    server->matchs[i] = NULL;
  }

  close(server->socket);

  free(server);
}

/* PLAYER */

player_p player_new(void){
  player_p ret = malloc(sizeof(player_t));
  ret->socket = -1;
  ret->index = -1;
  ret->username[0] = '\0';
  ret->m = NULL;

  return ret;
}

void player_free(player_p p){
  close(p->socket);
  free(p);
}

int player_queue_up(serv_p server, player_p p){
  int temp;
  player_p *cur;
  for(cur = server->players; cur < server->players + MAX_PLAYERS; cur++){
    if((*cur)->m){
      server->players[(*cur)->index] = p;
      server->players[p->index] = (*cur);
      temp = (*cur)->index;
      (*cur)->index = p->index;
      p->index = temp;

      return temp; 
    }
  }

  return p->index;
}

int player_connect(serv_p server){
  struct sockaddr_in paddr = {0};
  socklen_t addrlen = sizeof(paddr);
  player_p p;
  int sock, index;

  if(server->nb_players >= MAX_PLAYERS){
    fprintf(stderr, "player connect, server is full\n");
    return -1;
  }

  if((sock = accept(server->socket, 
          (struct sockaddr *) &paddr, &addrlen)) < 0){
    fprintf(stderr, "player_connect\n");
    return -1;
  }

  p = player_new();
  p->socket = sock;
  p->index = server->nb_players;

  server->max_fd = MAX(server->max_fd, sock);
  server->players[server->nb_players] = p;
  server->nb_players ++;

  index = player_queue_up(server, p);

  return index;
}

int player_disconnect(serv_p serveur, player_p p){
  match_p m = p->m;
  int index = p->index;
  server->nb_players --;
  if(!nb_players) return 0;

  server->players[index] = server->players[nb_players];
  server->players[nb_players] = NULL;

  if(m){
    match->players[(p == match->players[1])] = NULL;
    match_end(m, OPPO_LEFT);
  }

  player_free(p);
}

/* MATCH */

match_p match_new(void){
  match_p ret;
  ret = malloc(sizeof(match_t));
  ret->players[0] = NULL;
  ret->players[1] = NULL;
  ret->timers[0] = 0;
  ret->timers[1] = 0;
  ret->current_player = -1;
  ret->g = game_new();
  return ret;
}

void match_free(match_p m){
  game_free(m->g);
  m->g = NULL;
  free(m);
}

int match_start(serv_p server, player_p p1, player_p p2){
  /* match creation */
  match_p m = match_new();
  m->players[0] = p1;
  m->players[1] = p2;
  m->current_player = WHITE;

  /* add to match queue */
  server->matchs[server->nb_matchs] = m;
  
  /* send message to both players */
}

/*    Pop the match,
    put the two player at the right position in the queue (players), 
    send the recap to players*/
int match_end(serv_p server, match_p m);

/* MESSAGES */
ssize_t message_read(int sock, void* buffer, size_t bufsize){
  ssize_t bytes;
  if((bytes = read(sock, buffer, bufsize)) < 1){
    fprintf(stderr, "message_read\n");
    bytes = 0;
  }

  return bytes;
}



int message_write(int sock, char* buffer){

}
