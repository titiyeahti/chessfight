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
  MSGTYPE_t msgt;
  ssize_t bytes;
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

  /* read the code */
  if((bytes = read(sock, &msgt, 1)) < 1){
    player_free(p);
    return -1;
  }

  if(msgt != m_IDENTITY){
    player_free(p);
    return -1;
  }

  if((bytes = read(sock, p->username, MAX_NAME_LENGTH - 1)) < 0){
    player_free(p);
    return -1;
  }

  p->username[bytes] = '\0';

  server->max_fd = MAX(server->max_fd, sock);
  server->players[server->nb_players] = p;
  server->nb_players ++;

  index = player_queue_up(server, p);

  return index;
}

int player_disconnect(serv_p serveur, player_p p, char* buffer){
  match_p m = p->m;
  int index = p->index;
  server->nb_players --;
  if(!nb_players) return 0;

  server->players[index] = server->players[nb_players];
  server->players[nb_players] = NULL;

  if(m){
    match->players[(p == match->players[1])] = NULL;
    match_end(server, m, OPPO_LEFT, buffer);
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
  char buffer[2];
  ssize_t bytes;
  /* match creation */
  match_p m = match_new();
  m->players[0] = p1;
  m->players[1] = p2;
  m->current_player = WHITE;

  /* add to match queue */
  m->index = server->nb_matchs;
  server->matchs[server->nb_matchs] = m;
  server->nb_matchs ++;

  buffer[0] = m_START;

  buffer[1] = WHITE;
  bytes = write(p2->socket, buffer, 2);
  if(bytes < 2){
    match_free(m);
    return -1;
  }
  
  buffer[1] = BLACK;
  bytes = write(p1->socket, buffer, 2);
  if(bytes < 2){
    match_free(m);
    return -1;
  }

  return server->nb_matchs;
}

int match_end(serv_p server, match_p m, OUTCOME_t out, char* buffer){
  ssize_t bytes, offset;
  int i;
  player_p* p;

  for(p = m->players; p < m->players + 2; p++){
    if(!(*p)) 
      continue;

    /*write outcome*/
    buffer[0] = m_END;
    buffer[1] = out;
    if((bytes = writes(*p->socket, buffer, 2)) < 2){
      fprintf(stderr, "match_end outcome\n");
    }

    /*write recap*/
    buffer[0] = m_RECAP;
    offset = 1;
    
    memcpy(buffer + offset, m->timers, sizeof(m->timers));
    offset += sizeof(m->timers);

    memcpy(buffer + offset, m->g->moves, m->g->moves_len);
    offset += sizeof(ushort)*m->g->moves_len;

    if((bytes = writes(*p->socket, buffer, offset)) < 1){
      fprintf(stderr, "match_end oppoleft recap\n");
    }

    (*p)->g = NULL;
    i = player_queue_up(server, *p);
  }

  /* Decrease number of matchs 
     Move last match to m place
     Update index of the moved match
     Delete the old reference
   */

  server->nb_matchs --;
  server->matchs[m->index] = server->matchs[server->nb_matchs];
  server->matchs[m->index]->index = m->index;
  server->matchs[server->nb_matchs] = NULL;

  match_free(m);

  return EXIT_SUCCESS;
}

/* MESSAGES */
ssize_t message_read(int sock, void* buffer, size_t bufsize){
  ssize_t bytes;
  if((bytes = read(sock, buffer, bufsize)) < 1){
    fprintf(stderr, "message_read\n");
    bytes = 0;
  }

  return bytes;
}

ssize_t message_write(int sock, void* buffer, size_t len){
  ssize_t bytes;
  if((bytes = read(sock, buffer, len)) < 0){
    fprintf(stderr, "message_read\n");
  }

  return bytes;
}

/* APPLICATIONS */
int app_server(const char* port){
  int i;
  ssize_t bytes;
  ushort move;
  MSGTYPE_t msgt;
  player_p *cur, *p1, *p2;
  player_p oppo;
  char buffer[BUFSIZE];
  fd_set ind;
  serv_p server;

  if(!(server = serv_new(port))){
    fprintf(stderr, "serv_new\n");
    exit(EXIT_FAILURE);
  }

  for(;;){
    /* Start a match */
    if(server->nb_player - 2*server->nb_matchs > 1){
      ITER_PLAYERS(serv, p1, NULL){
        if(!(*p1)->match){
          ITER_PLAYER(serv, p2, p1+1)
            if(!(*p2)->match)
              break;

          break;
        }
      }

      i = match_start(serv, *p1, *p2);
    }

    FD_ZERO(&ind);
    FD_SET(server->socket, &ind);
    ITER_PLAYERS(serv, cur, NULL)
      FD_SET((*cur)->socket, &ind);

    select(server->max_fd, &ind, NULL, NULL, NULL);

    /* Smth written on stdin */
    if(FD_ISSET(STDIN_FILENO, &ind)){
      bytes = read(STDIN_FILENO, buffer, BUFSIZE);
      if(bytes < 0){
        exit(EXIT_FAILURE);
      }
    }

    /* New Connexion */
    if(FD_ISSET(server->socket, &ind)){
      i = player_connect(server);
    }

    ITER_PLAYERS(serv, cur, NULL){
      if(FD_ISSET((*cur)->socket, &ind)){
        
        /* at the moment we do not care if a player talks outside of a game */
        if(!(*cur)->game){
          bytes = read((*cur)->socket, buffer, BUFSIZE);
          continue;
        }

        /*read code*/
        /*read msg*/
        /*do stuff*/
        if((bytes = read((*cur)->socket, &msgt, 1))<1){
          player_disconnect(serv, *cur, buffer);
          break;
        }

        switch(msgt){
          case m_CHAT :
            bytes = read((*cur)->socket, buffer+1, BUFSIZE-2);
            buffer[0] = msgt;
            buffer[bytes+1] = '\0';
            /* if *cur is 0 then oppo i 1 and opposite */
            oppo = (*cur)->g->players[(*cur) == (*cur)->g->players[0]];
            bytes = write(oppo->socket, buffer, bytes + 1);
            break;

          case m_MOVE :
            bytes = read((*cur)->socket, &move, 2);
            /* get origin pos from move;
               check if move is in 
                possible_moves_pos(g, pos, uchar pmoves[27])
               if yes, do move, 
                  check if mat or draw
                    game_end(server, g, out),
                  otw
                    send_move to oppo
               if not, send back invalid;
             */
      }
    }
  }

  return EXIT_SUCCESS;
}

int app_client(char* port, char* address, char* name);
