/* Description : interface for chess rules implementation
   Author : thibaut milhaud
   Date : 01/08/22 */

#ifndef CHESS_H
#define CHESS_H

#include<stdlib.h>
#include<stdio.h>
#include<string.h>

/* BASIC DEFINITIONS */

#define ABS(i) ((i) < 0 ? -(i) : (i))

typedef unsigned int uint;
typedef unsigned char uchar;
typedef unsigned short ushort;

typedef enum colour {
  BLACK, WHITE
} COLOUR_t;

typedef enum pieces {
  EMPTY, PAWN, ROOK, KNIGHT, BISHOP, QUEEN, KING
} PIECE_t;

typedef enum compass {
  NORTH = 8, SOUTH = -8, EAST = 1, WEST = -1,
  NE = 9, NW = 7, SE = -7, SW = -9
} COMPASS_t;

typedef enum iter_mode{
  HUNTER, PREY
} ITERMODE_t;

#define MAX_MOVES 64

/* This const string will be used to convert pieces to char.
   Black are lowercase, and white uppercase.
   n stands for KNIGHT to differenciate from KING.
                             0123456789012 */
static const char pieces_string[] = " prnbqkPRNBQK";

static const PIECE_t back_row[] = {
  ROOK, KNIGHT, BISHOP, QUEEN, 
  KING, BISHOP, KNIGHT, ROOK
};

/* The calculs are here to ease the reading and do not matter because
   they are done during compilation */
static const int knight_moves[] = {
  2*8 + 1, 2*8 - 1, 2*1 - 8, 2*1 + 8,
  -2*8 + 1, -2*8 - 1, -2*1 + 8, -2*1 - 8
};

static const COMPASS_t compass_array[8] = {
  NORTH, SOUTH, EAST, WEST, NE, SE, NW, SW
};

#define PIECE2CHAR(p) \
  ((p) & 8 ? pieces_string[6 + ((p)&7)] : pieces_string[(p)&7])

#define COORD2INT(i,j) (8*(i)+(j))
#define INT2COORD(n,i,j) i = (n)/8; j = (n)%8

typedef struct game {
  /* Each cell contains a piece as define above and a colour.
     The colour is stored in the 4 bit (1<<3 = 8).
     0 stands for black and 1 for white.
     board[0] is A1, board[63] is H8*/
  uchar board[64];

  /* Moves played since beginning of the game.
     The 6 lower bits indicate the starting point of the move.
     The 6 next ones the destination.
     And the 3 upper bits indicates the wanted piece in case of promotion.*/
  ushort* moves;

  /* Number of played moves
     Allocated memory for moves */
  uint moves_len, max_moves;

  /* Numbers of moves since last piece taken or pawn moded */
  uint move_streak_len;

  /* the two lower bits indicates if black castle is available 
     (first for left rook and second for right one)
     and if the king moves the two are set to zero.
     the two next bits works the same for white castle.
     Then the twelve higher bits are storing kings positions:
      [4-9] - black king
      [10-15] - white king
   */
  ushort castle_kings;
} game_t;

typedef game_t* game_p;
/* tile t */
#define PIECE(t) ((t) & 7)

#define COLOUR(t) (((t) & 8)>>3)

#define BOARD(g,i,j) (g)->board[COORD2INT(i,j)]

#define PBOARD(g,pos) (g)->board[pos]

#define PPIECE(g,pos) PIECE(PBOARD(g,pos))

#define PCOLOUR(g,pos) COLOUR(PBOARD(g,pos))

#define CPIECE(g,i,j) ((BOARD(g,i,j))&7)

#define CCOLOUR(g,i,j) (((BOARD(g,i,j))&8)>>3)

#define OPPONENT(c) ((c) ? BLACK : WHITE)

/* Given a game_p g and a colour c, it is equal to 0 if the castle is 
   not available.
   Warning this does not check if the castle is POSSIBLE (king in chess,
   blocking pieces, ...) */
#define CASTLE_AVAILABLE(g,c) ((g)->castle_kings & (3 << ((c)*2)))

/* Given a game_p g and a colour c, this macro returns the position of 
   the c king in g */
#define KING_POS(g,c) ((g)->castle_kings & (63 << ((c)*6+4)))

#define MOVE_START(m) ((m)&63)

#define MOVE_END(m) (((m)&(63<<6))>>6)

#define DELTA_MOVE(m) (MOVE_END(m) - MOVE_START(m))

#define MOVE_PROM(m) (((m)&(7<<13))>>13)

#define EN_PASSANT(g,pos,d) \
  ((ABS(DELTA_MOVE((g)->moves[(g)->moves_len-1])) \
      == 16) \
  && ((pos + (2*PCOLOUR(g,pos)-1)*(d)) \
      == MOVE_END(g->moves[g->moves_len-1])) \
  && (g->board[pos+(2*PCOLOUR(g,pos)-1)*(d)] \
      == PAWN & ~(PCOLOUR(g,pos)<<3)))

/* Given a game, a colour and coordinates, this will set the position of the 
   chosen king in g->castle_kings to (8i+j)*/ 
#define SET_KING(g,c,i,j) \
  ((COORD2INT(i,j) << ((c)*6+4)) | \
   ((g)->castle_kings & (~(63 << ((c)*6+4)))))


/* return a pointer to a fresh new game */
game_p game_new(void);

void game_free(game_p g);

void board_print(game_p g);

void moves_print(game_p g);

void game_print(game_p g);

/* RULES */

/* check, pieces moves, castle, 3 times repetition, 50 play wihtout 
   "event", cannot move into a pice of the same colour, 
   cannot go through pieces, promotion, en passant, ...*/

/* Desc : Iterate from the position pos in the direction given by compass
   while not reaching an obstacle or a threat.
   Return : the number of empty tiles before obs or threat.
   If threat is not null and a threat is reached, *threat is set to 1.
   (or zero if it is an obstacle)

  TODO : comment better
 */
int iter_dir(game_p g, COLOUR_t c, ITERMODE_t m,
    COMPASS_t comp, int max, uchar pos, uchar* dests); 

int iter_knight(game_p g, COLOUR_t c, ITERMODE_t m, uchar pos, uchar* dests);

int is_tile_threatened_as_colour(game_p g, 
    uchar pos, COLOUR_t c, uchar* threats);

int is_color_in_check(game_p g, COLOUR_t c, uchar* threats);

int is_move_into_check(game_p g, ushort move);

int possible_moves_pos(game_p g, uchar pos, ushort* dests); 

/* 4 <= len <= 5 (carefull with null terminating character)
   check format ([a-h][1-8]){2}[prcbqk]{0,1}*/ 
int string_to_move(char* s, ushort *m);

/* Res must be a valid pointer of length at least 5 or 6 depending 
   on the case*/
void move_to_string(ushort m, char* res);

#endif
