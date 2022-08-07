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

int iter_dir(game_p g, COLOUR_t c, ITERMODE_t m,
    COMPASS_t comp, int max, uchar pos, uchar* dests){ 
  int count=0;
  int dest;
  max = max ? max : 8;
  for(dest=pos+comp; dest>0 && dest<64 && count<max; dest+=comp){
    if(g->board[dest]){
      /* If we are looking for the threat, there is only one value to 
         return per direction*/ 
      if(dest)
        dests[m==PREY ? 0 : count] = dest;

      count++;
      break;
    }
    
    if(m == HUNTER && dests)
      dests[count] = dest;

    count ++;
  }

  return count;
}

int iter_knight(game_p g, COLOUR_t c, ITERMODE_t m, uchar pos, uchar* dests){
  int count = 0;
  int i;
  int dest;
  for(i=0; i<8; i++){
    dest = pos + knight_moves[i];
    /* in the board */
    if(dest > 0 && dest < 64){
      /* if it is an ennemy case OR we hunt and this is an empty case */
      if((g->board[dest] && COLOUR(g->board[dest]) != c) 
          || (!g->board[dest] && HUNTER)){
        if(dests)
          dests[count] = dest;

        count++;
      }
    }
  }

  return count;
}

int is_tile_threatened_as_colour(game_p g, 
    uchar pos, COLOUR_t c, uchar* threats){
  uchar *current_threats = threats;
  int count, i;
  
  /* Look for knights around*/
  count += iter_knight(g, c, PREY, pos, current_threats);


  /* Look lines, columns or diags around the tile until 
     obstacle or threat */
  for(i=0; i<8; i++){
    current_threats = current_threats ? threats + count : NULL;
    count += iter_dir(g, c, PREY, compass_array[i], 0, pos, current_threats);
  }

  return count;
}

int is_color_in_check(game_p g, COLOUR_t c, uchar* threats){
  return is_tile_threatened_as_colour(g, KING_POS(g,c), c, threats);
}

int is_move_into_check(game_p g, ushort move){
  uchar origin = MOVE_START(move);
  uchar dest = MOVE_END(move);


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
  int count, nb_moves, i;
  uchar dest, val;
  PIECE_t p;
  COLOUR_t c;
  COMPASS_t direc;
  /* 3*7 + 6 maximum number of attacked case by a queen*/
  uchar dests_array[27];
  uchar *dests = pmoves ? dests_array : NULL;
  val = g->board[pos];
  p = PIECE(val);
  c = COLOUR(val);
  
  count = 0;
  switch(p){
    case EMPTY:
      break;

    case PAWN:
      /* 1 step forward */
      direc = (1-2*c)*NORTH;
      /* 2 steps forward if on starting pos */
      /* strike diagonal forward + en passant*/
      direc = (1-2*c)*NE;
      direc = (1-2*c)*NW;
      /* En passant (row 4(w) and 5(b) so 3 and 4 in our case)
         and if there is an enemy pawn that arrived next to pos
         last round*/
      /* Promote : processed at the end of function */
      break;

    case ROOK:
      for(i=0; i<4; i++){
        dests = dests ? dests_array + count : NULL;
        count += iter_dir(g, c, HUNTER, compass_array[i], 0, pos, dests);
      }

      break;

    case KNIGHT:
      count += iter_knight(g, c, HUNTER, pos, dests);
      break;

    case BISHOP:
      for(i=4; i<8; i++){
        dests = dests ? dests_array + count : NULL;
        count += iter_dir(g, c, HUNTER, compass_array[i], 0, pos, dests);
      }

      break;

    case QUEEN:
      for(i=0; i<8; i++){
        dests = dests ? dests_array + count : NULL;
        count += iter_dir(g, c, HUNTER, compass_array[i], 0, pos, dests);
      }

      break;

    case KING:
      /* one step every direction */
      for(i=0; i<8; i++){
        dests = dests ? dests_array + count : NULL;
        count += iter_dir(g, c, HUNTER, compass_array[i], 1, pos, dests);
      }
      /* TODO castle */
      break;
  }

  nb_moves = 0;
  if(p == KING){
  }
  else{
    if(p == PAWN){
      /* PROMOTE : if reach last row add the 
         4 possible promotion moves (qnrb)*/
  }

  return count;
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
