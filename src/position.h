#ifndef POSITION
#define POSITION
#include "board.h"
#include "piece.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "move.h"
#include "score.h" /* score different kind of moves, captures, promotions etc. */
#include <assert.h>
#include "move_list.h" /* mov_generator pushes the moves */
#include "hist.h" /* we have to score the moves */

/* this one is called alot, better to have it as macro,
   gives pos->b
 */
#define getBoard(P) P->b

/* zobrist key index for all pieces, side to move and passant status 
   used for fast look up in the zobrist array */
#define Z_WK 0
#define Z_WQ 1
#define Z_WR 2
#define Z_WB 3
#define Z_WN 4
#define Z_WP 5
#define Z_BK 6
#define Z_BQ 7
#define Z_BR 8
#define Z_BB 9
#define Z_BN 10
#define Z_BP 11
#define Z_TOMOVE 8
#define Z_PASSANT 9
#define Z_WCL 11
#define Z_WCS 12
#define Z_BCL 13
#define Z_BCS 14

/* value of the pieces */
#define VALUE_PAWN 100
#define VALUE_KNIGHT 400
#define VALUE_BISHOP 400
#define VALUE_ROOK 600
#define VALUE_QUEEN 1200
#define VALUE_KING 400

/* 64 bit int for zobrist keys */
#ifndef WIN32
typedef unsigned long long U64;
#endif
#ifdef WIN32
typedef unsigned __int64 U64;
#endif
typedef char boolean;

/* the POSITION */
typedef struct position_ *position;
typedef struct position_ {
  board b; /* the board */
  unsigned char side; /* the side the computer is playing */
  unsigned char toMove; /* player on the move */
  boolean blackPassant; /* can black capture en passant */
  boolean whitePassant; /* can white capture en passant */
  move_list moves; /* the moves that lead to the current position (if any). */
  int materialcount;  /* material count in the position. */
  int pieces; /* number of pieces in the position */
  int value; /* value of this position */
  unsigned int nodes; /* nodes searched (that gave value) */
  unsigned char passant_piece; /* the location of the passant pawn. */
  unsigned char moves_to_draw; /* for 50 move draw Rule. */
  int plycount; /* number of plys that have been played. */
  int plyWhiteLostShortCastle; /* ply White lost short castle, */
  int plyWhiteLostLongCastle;  /* makes undo_move easy. */
  int plyBlackLostShortCastle;
  int plyBlackLostLongCastle;
  int outOfBook; /* is the postion still book */
  U64 key; /* unique (almost) zobrist key for this position */
  U64 zobrist_key[12][BOARD_SIZE];

  U64 rep_stack[1024]; /* for repetition detection */
  int rep_sp; /* repetition stack pointer */

  int white_king; /* position of white king, for in_check() */
  int black_king; 

  int white_is_castled; /* is true if white has castled */
  int black_is_castled;
  history_heuristic hist_heuristic; /* history heuristic for scoring moves*/
  move_list_stack move_stack; /* stack for moves while searching and past moves */

  FILE *openingbook;
} position_;

/* FUNCTIONS */
position new_chess_position();
position new_chess_position_fen(char *fen);
move_list move_generator(position pos);
U64 getZobristKey(position pos);
char getToMove(position pos);
int getMaterial(position pos);
move_list getPosMoves(position pos);
int getOutOfBook(position pos);
void setToMove(position pos, char tomove);
void do_move(position pos, move mov);
void undo_move(position pos, move mov);
void print_move_list(move_list lstptr);
void print_position(position pos);
void debug_print_cap_list(move_list lstptr, position pos);
void free_position(position pos);
move legal_opponent_move(position pos, move mov);
int draw_or_checkmate(position pos);
int in_check_illegal(position pos);
piece getOpponentSide(position pos);
void setSideWhite(position pos);
void setSideBlack(position pos);
piece getCompSide(position pos);
int isCompBlack(position pos);
int isGameOver(position pos);
int getValue(position pos);
int getNodes(position pos);
void setValue(position pos, int value, int nodes);
void setOutOfBook(position pos);
int in_check(position pos);
void init_zobrist_key();
U64 get_zobrist_key(position pos);
int rep_search(position pos);
int getPly(position pos);
int white_is_castled(position pos);
int black_is_castled(position pos);
int last_move_null(position pos);
int pieces(position pos);
FILE *getBook(position pos);
history_heuristic get_hist_heuristic(position pos);
move_list_stack get_move_stack(position pos);
#endif
