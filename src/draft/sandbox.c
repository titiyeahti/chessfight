#include"chess.h"

void main(void){
  game_p g;
  char buffer[6];
  int i;
  g = game_new();
  g->moves[0] = 12;
  g->moves[0] += 20 << 6;
  g->moves[1] = 51;
  g->moves[1] += 35 << 6;
  g->moves[2] = 20;
  g->moves[2] += 28 << 6;
  g->moves[3] = 35;
  g->moves[3] += 27 << 6;
  g->moves_len = 4;
  game_print(g);
  game_free(g);

  ushort m;
  char* moves_test[] = {"a1b2", "abcd", "a3d4ed", "a7a8e", 
    "a7a8n"};
  for(i=0; i<5; i++){
    puts(moves_test[i]);
    if(!string_to_move(moves_test[i], &m)){
      move_to_string(m, buffer);
      printf("computes : %s\n", buffer);
    }
  }
}
