#ifndef BOARD
#define BOARD
#define BOARD_SIZE 128
#include "piece.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
/* MACROS */

/* true if A is on the board */
#define on_board(A) (!((A) & 0x88))
/* get piece at pos B on board A*/
#define get_piece(A, B) A->board[B]
/* set piece B on square C on board A */
#define set_piece(A, B, C) A->board[C]=B
/* gives rank for a pos */
#define rank(P) (((P)/16)+1)
#define file(P) (((P) % 16)+1)
/*
    The chess board:
  
    112 113 114 115 116 117 118 119
    96  97  98  99  100 101 102 103
    80  81  82  83  84  85  86  87
    64  65  66  67  68  69  70  71
    48  49  50  51  52  53  54  55
    32  33  34  35  36  37  38  39
    16  17  18  19  20  21  22  23
    0   1   2   3   4   5   6   7
*/
typedef struct board_ *board;
typedef struct board_ {
  piece board[128];
} board_;

board newboard();
void empty_board(board b);
void free_board(board b);
void print_board(board b);
void chess_board(board b);
#endif

