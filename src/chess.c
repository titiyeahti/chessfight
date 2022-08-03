#include"chess.h"

game_p game_new(void){
  int i;
  int j;
  game_p res = malloc(sizeof(game_t));

  /* board */
  for(j=0; j<8; j++){
    /* white pieces */
    BOARD(res,0,j) = back_row[j] + 8;
    BOARD(res,1,j) = PAWN + 8;

    /* black pieces */
    BOARD(res,7,j) = back_row[j];
    BOARD(res,6,j) = PAWN;

    /* no man's land */
    for(i=2; i<6; i++)
      BOARD(res,i,j) = EMPTY; 
  }

  res->moves = malloc(sizeof(ushort)*MAX_MOVES);
  res->moves_len = 0;
  res->max_moves = MAX_MOVES;
  res->move_streak_len = 0;

  res->castle_kings = 60 << 4;
  res->castle_kings += 4 << 10;
  res->castle_kings += 7;

  return res;
}

void game_free(game_p g){
  free(g->moves);
  free(g);
}

void board_print(game_p g){
  int i, j;
  for(i=7; i>-1; i--){
    printf("%d  ", i+1);
    for(j=0; j<8; j++)
      printf("%c ", PIECE2CHAR(BOARD(g, i, j)));
    
    printf("\n");
  }

  printf("\n   A B C D E F G H\n");
  printf("\n");
}

void moves_print(game_p g){
  int i;
  char move[6];
  for(i=0; i<g->moves_len; i++){
    move_to_string(g->moves[i], move);
    if(!(i&1))
      printf("%d.%s", i/2+1, move);
    else{
      printf(" %s", move);
      if(!(i == g->moves_len - 1))
        printf(", ");
    }

    if((i == g->moves_len - 1))
      printf("\n");
  }
}

void game_print(game_p g){
  board_print(g);
  moves_print(g);
}

int iter(game_p g, COLOUR_t c, uchar pos, COMPASS_t comp, int *threat){
  int count=0;
  for(;;){
    pos += comp;
    if(pos<0 || pos>63)
      break;

    if(g->board[pos]){
      if(threat)
        *threat = (COLOUR(g->board[pos]) == c) ? 0 : 1;

      break;
    }

    count ++;
  }

  return count;
}

int is_tile_threatened_as_colour(game_p g, 
    uchar pos, COLOUr_t c, uchar* threats){
  /* TODO */
  /* Look for knights around
     Look lines, columns or diags around the tile until 
     obstacle or threat */
  return 0;
}

int is_color_in_check(game_p g, COLOUR_t c, uchar* threats){
  return is_tile_threatened_as_colour(g,KING_POS(g,c), c, threats);
}

int is_move_into_check(game_p g, ushort move){
  /* TODO */
  /* Check if the move offers a line of sight on the king
     either strait or diagonal.
     If it is the case check if there is a chess

     if on diagonal, ABS(i-ik) == ABS(j-jk)
     and direction is (i-ik), (j-jk)

     if on a line or column i==ik of j==jk
   */

  return 0;
}

int possible_moves_pos(game_p g, uchar pos, ushort* pmoves){
  int count, i, j;
  uchar dest;
  PIECE_t p;
  COLOUR_t c;
  INT2COORD(pos,i,j);
  p = CPIECE(g,i,j);
  c = CCOLOUR(g,i,j);
  
  count = 0;
  switch(p){
    case EMPTY:
      break;

    case PAWN:
      /* 1 step forward */
      /* 2 steps forward if on starting pos */
      /* En passant (row 4(w) and 5(w) so 3 and 4 in our case)
         and if there is an enemy pawn that arrived next to pos
         last round*/
      /* Promote */
      /* strike diagonal forward */
      break;

    case ROOK:
      /* row, columns */
      break;

    case KNIGHT:
      /* check the eight possible tiles */
      break;

    case BISHOP:
      /* diagonals */
      break;

    case QUEEN:
      /* rows, columns, diagonals */
      break;

    case KING:
      /* one step every direction */
      /* castle */
      break;
  }

}


int string_to_move(char* s, ushort *m){
  int len = strlen(s);
  int i, flag;
  if(len<4 || len> 5)
    return EXIT_FAILURE;

  for(i=0; i<4; i++){
    if((i&1) && ((s[i] < '1') || (s[i] > '8')))
      return EXIT_FAILURE;
    if((!(i&1)) && ((s[i] < 'a') || (s[i] > 'h')))
      return EXIT_FAILURE;
  }

  if(len==5){
    flag = 0;
    for(i=2; i<6 && !flag; i++)
      flag = (s[4] == pieces_string[i]) ? i : 0;

    if(!flag) 
      return EXIT_FAILURE;
  }

  /* start */
  *m = COORD2INT((int) (s[1] - '1'),
      (uint) (s[0] - 'a'));

  /* dest */
  *m += COORD2INT((int) (s[3] - '1'),
      (uint) (s[2] - 'a')) << 6;

  /* prom */
  *m += flag << 13; 

  return EXIT_SUCCESS;
}

void move_to_string(ushort m, char* res){
  int i, j, prom;
  /* i -> 1-8
     j -> a-h
   */
  INT2COORD(MOVE_START(m), i, j);
  res[0] = (char) j + 'a'; res[1] = (char) i + '1';

  INT2COORD(MOVE_END(m), i, j);
  res[2] = (char) j + 'a'; res[3] = (char) i + '1';

  prom = MOVE_PROM(m);

  if(prom){
    res[4] = pieces_string[prom];
    res[5] = '\0';
  }
  else {
    res[4] = '\0';
  }
}
