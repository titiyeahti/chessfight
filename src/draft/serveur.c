#include "serveur.h"

int game(player_p p1, player_p p2){
  COLOUR_t curcol;
  player_p players[2];
  ushort moves[27];
  ushort move;
  uchar dest, origin, prom;
  ssize_t bytes;
  int i, nb_moves, toss, j;
  clock_t clock[2] = {0};
  clock_t deltac;
  /* for communication purposes */
  char buffer[256];
  game_p g = game_new();

  toss = rand() & 1;
  player[0] = toss ? p1 : p2;
  player[1] = !toss ? p1 : p2;


  /* send colours to players */
  deltac = clock();
  for(i=0;; i++){
    curcol = i&1;
    /*listen move for current player*/
    bytes = read(players[curcol], buffer, 256);
    if(bytes < 0){return EXIT_FAILURE;}
    buffer[bytes] = '\0';
    
    /*compute response time*/
    delatc = clock() - deltac;
    clock[curcol] += deltac;

    /*check response validity, syntax then feasibility*/
    /*syntax correct*/
    if(!string_to_move(buffer, &move)){
      dest = MOVE_END(move);
      origin = MOVE_START(move);
      /*trying to move a piece of its colour*/
      if(!PBOARD(g,origin) || PCOLOUR(g, origin) != curcol)
        goto incorrect_response;

      nb_moves = possible_moves_pos(g, origin, moves);
      /*checking feasibility*/
      for(j=0; j<nb_moves; j++) if(moves[j] == move) break;

      if(j == nb_moves) goto incorrect_response;

      /* Move is correct */
      move_do(g,move);

      /* look for stalemate, check, fifty moves and 3 times repeatitions */

      
      write(p[!curcol], buffer, bytes);
      deltac = clock();
      continue;
    }

incorrect_reponse: 
    /* choose the error to send*/
    write(p[curcol], 0, 1);
    deltac = clock();
    i--;
  }

  return EXIT_SUCCESS
}
