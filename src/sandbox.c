#include"chess.h"

void main(void){
  game_p g;
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
}
