/* Description : interface for chess rules implementation
   Author : thibaut milhaud
   Date : 01/08/22 */

#ifndef CHESS_H
#define CHESS_H

#include<stdlib.h>
#include<stdio.h>
#include<string.h>

/* BASIC DEFINITIONS */

typedef unsigned int uint;
typedef unsigned char uchar;
typedef unsigned short ushort;

typedef enum colour {
  BLACK, WHITE
} COLOUR;

typedef enum pieces {
  EMPTY, PAWN, ROOK, KNIGHT, BISHOP, QUEEN, KING
} PIECES;

#define MAX_MOVES 64

/* This const string will be used to convert pieces to char.
   Black are lowercase, and white uppercase.
   n stands for KNIGHT to differenciate from KING.
                             0123456789012 */
static const char pieces_string[] = " prnbqkPRNBQK";

static const PIECES back_row[] = 
  {ROOK, KNIGHT, BISHOP, QUEEN, 
  KING, BISHOP, KNIGHT, ROOK};


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

  /* Size of moves */
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

#define BOARD(g,i,j) (g)->board[COORD2INT(i,j)]

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

#define MOVE_PROM(m) (((m)&(7<<13))>>13)

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

/* 4 <= len <= 5 (carefull with null terminating character)
   check format ([a-h][1-8]){2}[prcbqk]{0,1}*/ 
ushort string_to_move(char* s);

/* Res must be a valid pointer of length at least 5 or 6 depending 
   on the case*/
void move_to_string(ushort m, char* res);

#endif
