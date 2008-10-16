#ifndef MOVE
#define MOVE
#define NULL_MOVE 0
#define NULL_MOVE_PRUNING (1 << 28)
#include "piece.h"
#include "board.h"
#include <string.h>
#include <ctype.h>
/*
move of the form: 
 28  27  26  25            21  20           16  
 Ca  Pa  P   prom_piece    C   cap_piece    src-dst
 1b  1b  1b  4bits         1b  4bits        16bits

 Cabit-Pabit-Pbit-4bits Cbit-4bits 16bits
                                   src-dst
 Cabit is set if the move is a castle move.
 Pabit is set if the move is a passant move.
 Pbit is set if the move is a promotion.
 Cbit is set if the move is a capture.
 Pbit is followed by the promotion piece.
 Cbit is followed by the captured piece.

Note: Pbit and Cbit can both be set in a move,
a move that captures and promotes.

To check a bit Bi in mov: mov & (1<<i) 
*/
typedef int move; /* a move is an int */

/* MACROS */
/* to get stuff from a move: */
/*-------------------------------------------------------.
 | macros: get_*(move mov) -> int                        |
 | returns:  the bits in move for promotion, src, dst    |
  -------------------------------------------------------*/
#define get_prom_piece(M) getbits(M, 24, 4)
#define get_cap_piece(M) getbits(M, 19, 4)
#define get_src(M) getbits(M, 15, 8)
#define get_dst(M) getbits(M, 7, 8)
/* to test a move: */
/*-------------------------------------------------------.
 | macros: is_*_move(move mov) -> boolean                |
 | returns:  true/false if the move mov is of type:      |
 |           castle, passant, promotion, capture.        |
 | note:     a move can be a mixture of this, ex passant |
 |           is always also a capture. A promotion move  |
 |           can also be a capture move.                 |
  -------------------------------------------------------*/
#define is_castle_move(M) getbits(M, 27, 1)
#define is_passant_move(M) getbits(M, 26, 1)
#define is_prom_move(M) getbits(M, 25, 1)
#define is_capture_move(M) getbits(M, 20, 1)
#define legal_move(M) (!(is_capture_move(M)&&(get_cap_piece(M)==WK || get_cap_piece(M)==BK)))
/* create a a move: */
#define new_move(SRC, DST) (((SRC)<<8)|(DST))
#define new_move_with_cap(SRC, DST, CAP) (((((SRC)<<8)|(DST))|(CAP<<16))|(1<<20))
/* FUNCTIONS 
   note: many of these can also be macros, but these aren't called as often as
   the above macros.
*/
move set_mov_castle(move mov);
move new_move_with_prom(int src, int dst, piece prom_piece);
move new_move_with_passant(int src, int dst, piece cap);
move new_move_all(int src, int dst, piece cap, piece prom, int ca, int pa);
void print_move_alg(move mov);
void print_move(move mov);
void move_to_alg(move mov, char *movealg);
move alg_to_move(char *movealg, char side);
int debug_legal_move(move mov);
#endif
