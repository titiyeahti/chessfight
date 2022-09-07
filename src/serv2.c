#include "serv2.h"

/** Init connexion
  * Connect player
  * Server loop
  * Client loop
  * Write UTILS.h and CONSTANT_VALUES.def
  * Uniformize error managment (errno.h + macro in utils.h)
  */

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

player_p player_new(void){
  player_p ret = malloc(sizeof(player_t));
  ret->socket = -1;
  ret->index = -1;
  ret->username[0] = '\0';
  ret->m = NULL;

  return ret;
}

int player_queue_up(serv_p server, player_p p){
  int temp;
  player_p *cur;
  for(cur = server->players; cur < server->nb_players + MAX_PLAYERS; cur++){
    if(!(*cur)->g){
      if((*cur)->index != p->index){
        server->players[(*cur)->index] = p;
        server->players[p->index] = (*cur);
        temp = (*cur)->index;
        (*cur)->index = p->index;
        p->index = temp;
      }

      break;
    }
  }
}

int player_connect(serv_p server){
  struct sockaddr_in paddr = {0};
  size_t addrlen = sizeof(paddr);
  player_p p;
  int sock, index;

  if(server->nb_players >= MAX_PLAYERS){
    fprintf(stderr, "player connect, server is full\n");
    return -1;
  }

  if((sock = accept(server->socket, &paddr, &psize)) < 0){
    fprintf(stderr, "player_connect\n");
    return -1;
  }

  p = player_new();
  p->socket = sock;
  p->index = nb_players;

  server->max_fd = max(server->max_fd, sock);
  server->players[server->nb_players] = p;
  server->nb_players ++;

  index = player_queue_up(server, p);
}

void player_free(player_p p){
  close(p->socket);
  free(p);
  p = NULL;
}

int player_disconnect(serv_p serveur, player_p p){
  /*TODO
   match = p->m
   index = p->index
   free(p) // frippé lol
   nb_players--
   if(!nb_players)
     return success

   players[index] = players[nb_players];
   players[nb_players] = NULL;

   if match
     match_end(match_p m, OUTCOME_t out);
     (end the match, i.e. queue_up opponent, send message to opponent)
   */
}

serv_p serv_new(port){
  int sock;
  serv_p ret;

  if((sock = connectio_init(port)) < 0){
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
    server_players[i] = NULL;
  }

  for(i=0; i<server->nb_matchs; i++){
    match_free(server->matchs[i]);
    server->matchs[i] = NULL;
  }

  close(server->socket);
}

client_p client_new(char* name);

int match_start(serv_p server, player_p p1, player_p p2);

/*    Pop the match,
    put the two player at the right position in the queue (players), 
    send the recap to players*/
int match_end(serv_p server, match_p m);
