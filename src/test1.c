#include "chess.h"

int main(void){
  game_p g;
  int count;
  ushort pmoves[200];

  g = game_new();

  board_print(g);

  count = possible_moves_colour(g, WHITE, pmoves);

  printf("count = %d\n", count);
  getc(stdin);
  moves_print_ext(pmoves, count);

  game_free(g);

  return 0;
}

