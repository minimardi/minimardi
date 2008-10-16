#ifndef SCORE
#define SCORE
#include "piece.h"
#define CAPTURES_FIRST 1000000 /* Make sure that captures are always tried before other moves */
#define PAWN_SCORE 6 
#define KNIGHT_SCORE 5
#define BISHOP_SCORE 4
#define ROOK_SCORE 3 
#define QUEEN_SCORE 2
#define KING_SCORE 1
#define CAP_KING_SCORE 6
#define CAP_QUEEN_SCORE 5
#define CAP_ROOK_SCORE  4 
#define CAP_BISHOP_SCORE 3 
#define CAP_KNIGHT_SCORE 2 
#define CAP_PAWN_SCORE 1
#define PROM_Q CAPTURES_FIRST-1
#define PROM_R CAPTURES_FIRST-2
#define PROM_B CAPTURES_FIRST-3
#define PROM_N CAPTURES_FIRST-4
int score_cap(piece p, piece cap);
int score_prom_cap(piece prom, piece cap);
int score_prom(piece prom);
#endif
