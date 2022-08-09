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

game_p game_copy(game_p g){
  game_p res = malloc(sizeof(game_t));
  memcpy(res, g, sizeof(game_t));
  res->moves = malloc(sizeof(ushort)*res->max_moves);
  memcpy(res->moves, g->moves, sizeof(ushort)*res->max_moves);

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

int iter_dir(game_p g, COLOUR_t c, COMPASS_t comp, 
    int max, uchar pos, uchar* dests){ 
  int count, dest, i;
  count = 0;
  i=0;
  max = max ? max : 8;
  for(dest=pos+comp; dest>0 && dest<64 && i<max; dest+=comp){
    if(!PBOARD(g, dest)){
      if(dest)
        dests[count] = dest;

      count++;
    }
    else {
      if(PCOLOUR(g,dest) != c){
        if(dest)
          dests[count] = dest;

        count++;
      }

      break;
    }

    i++;
  }

  return count;
}

int threats_dir(game_p g, COLOUR_t c, COMPASS_t comp, 
    uchar pos, uchar* threats){
  int count, i, ldv ;
  COMPASS_t pawn_e, pawn_w;
  PIECE_t p;
  i=1;
  ldv=1;
  
  /* iterrate from pos in the direction comp */
  for(pos=pos+comp; pos<64 && pos>0; pos+=comp){
    if(PCOLOUR(g,pos) == c)
      ldv =-1;
    else{
      p = PPIECE(g,pos);
      switch (p){
        case KING :
          if(i==1)
            goto save_threat;

        case PAWN :
          pawn_w = (1-2*c)*NW;
          pawn_e = (1-2*c)*NE;
          /* do struff */
          if(i==1 && (comp == pawn_e || comp == pawn_w))
            goto save_threat;

          break;
        case ROOK:
          if(!(comp%2) || (ABS(comp) < 2))
            goto save_threat;

          break;

        case BISHOP:
          if(comp%2 && (ABS(comp) > 2))
            goto save_threat;

          break;

        case QUEEN:
          goto save_threat;

          break;
      }

save_threat:
          if(threats)
            threats[count] = pos;

          count++;

    }

    /* increment i*/
    i++;
  }
  return ldv*count;
}

int threats_knight(game_p g, COLOUR_t c, uchar pos, uchar* threats){
  int count, i, dest;
  for(i=0; i<8; i++){
    dest = pos + knight_moves[i];
    if(dest > 0 && dest < 64){
      if(PBOARD(g, dest) == KNIGHT + (OPPONENT(c) << 3)){
        if(threats)
          threats[count] = dest;

        count ++;
      }
    }
  }

  return count;
}

int iter_knight(game_p g, COLOUR_t c, uchar pos, uchar* dests){
  int count = 0;
  int i;
  int dest;
  for(i=0; i<8; i++){
    dest = pos + knight_moves[i];
    /* in the board */
    if(dest > 0 && dest < 64){
      if((PBOARD(g,dest) && PCOLOUR(g,dest) != c) 
          || (!PBOARD(g,dest))){
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
  int count, i, temp;
  
  /* Look for knights around */
  count += threats_knight(g, c, pos, current_threats);


  for(i=0; i<8; i++){
    current_threats = current_threats ? threats + count : NULL;
    temp = threats_dir(g, c, compass_array[i], pos, current_threats);
    count += temp > 0 ? temp : 0;
  }

  return count;
}

int is_color_in_check(game_p g, COLOUR_t c, uchar* threats){
  return is_tile_threatened_as_colour(g, KING_POS(g,c), c, threats);
}


/* TODO */
int is_move_into_check(game_p g, ushort move){
  uchar origin = MOVE_START(move);
  uchar dest = MOVE_END(move);
  COLOUR_t c = PCOLOUR(g,origin);

  if( (origin%8 == KING_POS(g,c)%8) && origin%8 != dest%8)
    return 0;
  /* Check if the move offers a line of sight on the king
     either strait or diagonal.
     If it is the case check if there is a chess

     if on diagonal, ABS(i-ik) == ABS(j-jk)
     and direction is (i-ik), (j-jk)

     if on a line or column i==ik of j==jk
   */

  return 0;
}

int is_move_legal(game_p g, ushort move){
  uchar origin, dest, temp;
  COLOUR_t c;
  int res;

  origin = MOVE_START(move);
  dest = MOVE_END(move);
  c = PCOLOUR(g, origin);
  temp = PBOARD(g,dest);
  PBOARD(g,dest) = PBOARD(g,origin);
  PBOARD(g,origin) = EMPTY;

  res = is_color_in_check(g, c, NULL);

  PBOARD(g,origin) = PBOARD(g,dest);
  PBOARD(g,dest) = temp;

  return res;
}


int possible_moves_pos(game_p g, uchar pos, ushort* pmoves){
  int count, nb_moves, i, flag;
  uchar dest, val;
  ushort move;
  PIECE_t p;
  COLOUR_t c;
  COMPASS_t direc;
  /* 3*7 + 6 maximum number of attacked case by a queen*/
  uchar dests_array[27];
  uchar *dests = dests_array;
  val = PBOARD(g,pos);
  p = PIECE(val);
  c = COLOUR(val);
  
  count = 0;
  switch(p){
    case EMPTY:
      break;

    case PAWN:
      /* 1 step forward */
      /* 2 steps forward if on starting pos */
      direc = (2*c-1)*NORTH;
      flag = pos + direc;
      if(flag < 64 && flag > 0){
        if(!PBOARD(g,flag)){
          if(dests)
            dests[count] = pos + direc;
          
          count ++;
          
          /* If pawn on starting pos and tile 2 steps ahead is empty */
          if(pos>(47-40*c) && pos<(16-40*c) && !PBOARD(g,pos+2*direc)){
            if(dests)
              dests[count] = pos + 2*direc;
            
            count ++;
          }
        }
      }


      /* strike diagonal forward + en passant*/
      direc = (2*c-1)*NE;
      flag = pos + direc;
      if(flag < 64 && flag > 0){
        if(PBOARD(g,flag) && !(PBOARD(g,flag) & (c << 3))){
          if(dests)
            dests[count] = pos + direc;
          
          count ++;
        }

        /* Last move 2 tile north or south 
           and arrives next to pos
           and is an ennemy pawn
         */
        if(EN_PASSANT(g,pos,EAST)){
          if(dests)
            dests[count] = pos + direc;

          count ++;
        }
      }

      direc = (2*c-1)*NW;
      flag = pos + direc;
      if(flag < 64 && flag > 0){
        if(PBOARD(g,flag) && !(PBOARD(g,flag) & (c << 3))){
          if(dests)
            dests[count] = pos + direc;
          
          count ++;
        }

        /* Last move 2 tile north or south 
           and arrives next to pos
           and is an ennemy pawn
         */
        if(EN_PASSANT(g,pos,WEST)){
          if(dests)
            dests[count] = pos + direc;

          count ++;
        }
      }

      break;

    case ROOK:
      for(i=0; i<4; i++){
        dests += count;
        count += iter_dir(g, c, compass_array[i], 0, pos, dests);
      }

      break;

    case KNIGHT:
      count = iter_knight(g, c, pos, dests);
      break;

    case BISHOP:
      for(i=4; i<8; i++){
        dests = dests_array + count;
        count += iter_dir(g, c, compass_array[i], 0, pos, dests);
      }

      break;

    case QUEEN:
      for(i=0; i<8; i++){
        dests = dests_array + count;
        count += iter_dir(g, c, compass_array[i], 0, pos, dests);
      }

      break;

    case KING:
      /* one step every direction */
      for(i=0; i<8; i++){
        dests = dests_array + count;
        count += iter_dir(g, c, compass_array[i], 1, pos, dests);
      }

      /* long castle (left) */
      if(LEGAL_CASTLE(g,c,1)){
        if(PBOARD(g,pos-1) == EMPTY && PBOARD(g,pos-2) == EMPTY){
          flag = 0;
          for(i=0; i<3 && !flag; i++)
            flag = is_tile_threatened_as_colour(g, pos-i, c, NULL) ? 1 : 0;

          dests[count] = pos-2;
          count ++;
        }
      }

      /* short castle (right) */
      if(LEGAL_CASTLE(g,c,2)){
        if(PBOARD(g,pos+1) == EMPTY && PBOARD(g,pos+2) == EMPTY){
          flag = 0;
          for(i=0; i<3 && !flag; i++)
            flag = is_tile_threatened_as_colour(g, pos+i, c, NULL) ? 1 : 0;

          dests[count] = pos+2;
          count ++;
        }
      }

      break;
  }

  /* Compute the moves from the destinations */
  nb_moves = 0;
  if(p == KING){
    for(i=0; i<count; i++){
      if(!is_tile_threatened_as_colour(g, dests[i], c, NULL)){
        if(pmoves)
          pmoves[nb_moves] = pos + dests[i] << 6;

        nb_moves ++;
      }
    }
  }
  else{
    for(i=0; i<count; i++){
      move = pos + (dests[i] << 6);
      if(is_move_legal(g, move)){
        if(p == PAWN && (dests[i]/8 == 0 || dests[i]/8 == 7)){
          for(val=ROOK; val<KING; val++){
            if(pmoves)
              pmoves[nb_moves] = move + (val<<13);

            nb_moves ++;
          }
        }
        else{
          if(pmoves)
            pmoves[nb_moves] = move;

          nb_moves ++;
        }
      }
    }
  }

  return nb_moves; 
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

