#ifndef PIECE
#define PIECE
#define NDEBUG
#include <stdio.h>
#include <assert.h>
/* MACROS */
#define getbits(X, P, N) (((X)>>((P)+1-(N))) & ~(~0<<(N)))
/* true if piece P is the EMPTY piece */
#define is_empty(P)  (P & EMPTY)
/* true if P1 and P2 are not the same color */
#define is_different(P1, P2) (!(is_empty(P1) || is_empty(P2))&&((P1>>3)^(P2>>3)))
/* true if P is a WHITE piece */
#define is_white_piece(P) ((P) & WHITE)
/* true if P is a BLACK piece */
#define is_black_piece(P) (!((P) & WHITE) && (!(P & EMPTY)))
/*
5bits:
   b4    b3    b2    b1   b0
 empty  color  ^-- piece -^ 

piece & WHITE -> if piece is a White piece
!(piece & WHITE) && (!piece & EMPTY) -> 
if piece is a Black piece
*/
#define WK 9      /* 1001 */
#define WQ 10     /* 1010 */
#define WR 11     /* 1011 */
#define WB 12     /* 1100 */
#define WN 13     /* 1101 */
#define WP 14     /* 1110 */
#define BK 1      /* 0001 */
#define BQ 2      /* 0010 */
#define BR 3      /* 0011 */
#define BB 4      /* 0100 */
#define BN 5      /* 0101 */
#define BP 6      /* 0110 */
#define EMPTY 16  /* 10000 */

#define BLACK 0   /* 0000 */
#define WHITE 8   /* 1000 */
typedef unsigned char piece;

/* FUNCTIONS */
/* for debug: */
int valid_piece(piece p);
int valid_piece_not_empty(piece p);

piece make_same_color(piece p1, piece p2);
#endif
