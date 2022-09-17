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
#define MAX(a,b) ((a)>(b) ? (a) : (b))
#define MAX_MOVES 64

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

/* This const string will be used to convert pieces to char.
   Black are lowercase, and white uppercase.
   n stands for KNIGHT to differenciate from KING.
                             0123456789012 */

static const PIECE_t back_row[] = {
  ROOK, KNIGHT, BISHOP, QUEEN, 
  KING, BISHOP, KNIGHT, ROOK
};

/* The calculs are here to ease the reading and do not matter because
   they are done during compilation */
static const int knight_moves[] = {
  17, 15, -6, 10,
  -15, -17, 6, -10
};

static const char pieces_string[] = ".prnbqkPRNBQK";

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

  /* captures[2*i] = piece+colour<<3 
     captures[2*i+1] = #move of the capture
   */
  uchar captures[64];

  /* Number of played moves
     Allocated memory for moves */
  uint moves_len, max_moves, captures_len;

  /* Numbers of moves since last piece taken or pawn moded */
  uint moves_streak_len;

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

/* BOARD */
#define ITER_BOARD(g,cur) for(cur=g->board; cur < g->board + 64; cur++)

#define BOARD(g,i,j) (g)->board[COORD2INT(i,j)]

#define CPIECE(g,i,j) ((BOARD(g,i,j))&7)

#define CCOLOUR(g,i,j) (((BOARD(g,i,j))&8)>>3)

#define PBOARD(g,pos) (g)->board[pos]

#define PPIECE(g,pos) PIECE(PBOARD(g,pos))

#define PCOLOUR(g,pos) COLOUR(PBOARD(g,pos))

#define OPPONENT(c) ((c) ? BLACK : WHITE)

/* Given a game_p g and a colour c, it is equal to 0 if the castle is 
   not available.
   Warning this does not check if the castle is POSSIBLE (king in chess,
   blocking pieces, ...) */
#define CASTLE_AVAILABLE(g,c) ((g)->castle_kings & (3 << ((c)*2)))

/* Given a game_p g and a colour c, this macro returns the position of 
   the c king in g */
#define KING_POS(g,c) ((g)->castle_kings & (63 << ((c)*6+4)))

/* Given a game, a colour and coordinates, this will set the position of the 
   chosen king in g->castle_kings to (8i+j)*/ 
#define SET_KING(g,c,pos) \
  (((pos) << ((c)*6+4)) | \
   ((g)->castle_kings & (~(63 << ((c)*6+4)))))


/* if side == 1 check the long castle
   if side == 2 check the short castle */
#define LEGAL_CASTLE(g,c,side) (((side)<<((c)*2)) & (g)->castle_kings)

#define LAST_MOVE(g) (g)->moves[(g)->moves_len-1] 

#define MOVE_START(m) ((m)&63)

#define MOVE_END(m) (((m)&(63<<6))>>6)

#define DELTA_MOVE(m) (MOVE_END(m) - MOVE_START(m))

#define MOVE_PROM(m) (((m)&(7<<13))>>13)

/* me stands for move end, and must be provided
   return 1 if the following 3 statements are true
   Last move is a pawn
   Last move is two tiles North or south
   Depending on the colour and direction, did the move arrived next to pos
   */
#define EN_PASSANT(g,pos,d,me) \
  (me = MOVE_END(LAST_MOVE(g)) && \
  (PIECE(me) == PAWN) \
  && ((ABS(me - MOVE_START(LAST_MOVE(g)))) == 16) \
  && ((pos + (2*PCOLOUR(g,pos)-1)*(d)) == me))


/*!
  \brief Allocates memory for a new game and return a pointer to it.
  \returns A new game pointer.
*/
game_p game_new(void);

/*!
  \brief Performs a deep copy of \a g and returns it.
  \param g game pointer
  \returns A pointer to a deep copy of \a g.
*/
game_p game_copy(game_p g);

/*!
  \brief Frees the memory used by \a g.
  \param g game pointer
*/
void game_free(game_p g);

/*!
  \brief Print the current state of the board of \a g to stdout.
  \param g game pointer
*/
void board_print(game_p g);

void moves_print_ext(ushort* moves, int len);
/*!
  \brief Print the moves played during \a g to stdout.
  \param g game pointer
*/
void moves_print(game_p g);

/*!
  \brief Print the current state of the game \a g to stdout.
  \param g game pointer
*/
void game_print(game_p g);

/* RULES */

/*!
  Look for the availables destinations in the choosed direction \a comp.
  Stop when an obstacle is meet. If the obstacle is an opponent, it counts 
  as a valid destination. If \a dests is not null, possibles destinations
  are stored inside it.
  \param g game pointer
  \param c current colour
  \param comp direction you want check
  \param max max number of steps you want to take, ignored if 0
  \param pos starting point of the search
  \param dests table where the available destinations are stored, if \a
  dests is NULL, it is ignored
  \returns The number of possible destinations.
*/
int iter_dir(game_p g, COLOUR_t c, COMPASS_t comp, 
    int max, uchar pos, uchar* dests); 

/*!
  Look for threats for \a pos in the direction \a comp. 
  If an obstacle is meet before any threats, check indirects threats instead.
  In this case a negative value is returned. 
  If /a threats is not null, direct of indirect threats are stored in it.
  \param g A game pointer
  \param c current colour
  \param comp direction you want to check
  \param pos starting point of the search
  \param threats table to store threats meet, ignored if NULL
  \returns The number of direc threats if any and minus the number 
  of indirect threats otherwise.
*/
int threats_dir(game_p g, COLOUR_t c, COMPASS_t comp, 
    uchar pos, uchar* threats);

/*!
  Look for the availables destinations from \a pos for a knight.
  If \a dests is not null, possibles destinations are stored inside it.
  \param g game pointer
  \param c current colour
  \param pos starting point of the search
  \param dests table where the available destinations are stored,
  ignored if NULL
  \returns The number of possible destinations.
*/
int iter_knight(game_p g, COLOUR_t c, uchar pos, uchar* dests);

/*!
  Look for knights threatening \a pos for.
  If \a dests is not null, possibles destinations are stored inside it.
  \param g game pointer
  \param c current colour
  \param pos starting point of the search
  \param threats table where the threats are stored, ignored if NULL
  \returns The number of threatening knights.
*/
int threats_knight(game_p g, COLOUR_t c, uchar pos, uchar* threats);

/*!
  \param g game pointer
  \param c current colour
  \param pos starting point of the search
  \param threats table where the threats are stored, ignored if NULL
  \returns The number of ennemies threatening \a pos.
*/
int is_tile_threatened(game_p g, COLOUR_t c, uchar pos, uchar* threats);

int is_color_in_check(game_p g, COLOUR_t c, uchar* threats);

int is_move_into_check(game_p g, ushort move);

int is_move_legal(game_p g, ushort move);

/* testing protocol 
   game 
   input move
   print game
   print possibles moves colour

   check captures
   check effective moves
   en passant ^^'
   check 
   ...
 */

/* WARNING */
int possible_moves_pos(game_p g, uchar pos, ushort* pmoves); 

/* Warning but less */
int possible_moves_colour(game_p g, COLOUR_t c, ushort* pmoves);

/* VERY DANGEROUS */
int move_do(game_p g, ushort move);

/* 4 <= len <= 5 (carefull with null terminating character)
   check format ([a-h][1-8]){2}[prcbqk]{0,1}*/ 
int string_to_move(char* s, ushort *m);

/* Res must be a valid pointer of length at least 5 or 6 depending 
   on the case*/
void move_to_string(ushort m, char* res);

#endif
