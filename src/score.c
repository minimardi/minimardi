#include "score.h"

/*-------------------------------------------------------.
 | function: score_capture_move(piece p, piece cap)->int |
 | returns:  mvv/lva value for capture pxcap.            |
  -------------------------------------------------------*/
int score_cap(piece p, piece cap){
  int a=0, b=0;
 
  switch(p){
  case WK: a = KING_SCORE ; break;
  case WQ: a = QUEEN_SCORE; break;
  case WR: a = ROOK_SCORE;  break;
  case WB: a = BISHOP_SCORE; break;
  case WN: a = KNIGHT_SCORE; break;
  case WP: a = PAWN_SCORE; break;
  case BK: a = KING_SCORE ; break;
  case BQ: a = QUEEN_SCORE; break;
  case BR: a = ROOK_SCORE;  break;
  case BB: a = BISHOP_SCORE; break;
  case BN: a = KNIGHT_SCORE; break;
  case BP: a = PAWN_SCORE; break;
  default: assert(0);
  }
  switch(cap){
  case WK: b = CAP_KING_SCORE ; break;
  case WQ: b = CAP_QUEEN_SCORE; break;
  case WR: b = CAP_ROOK_SCORE;  break;
  case WB: b = CAP_BISHOP_SCORE; break;
  case WN: b = CAP_KNIGHT_SCORE; break;
  case WP: b = CAP_PAWN_SCORE; break;
  case BK: b = CAP_KING_SCORE ; break;
  case BQ: b = CAP_QUEEN_SCORE; break;
  case BR: b = CAP_ROOK_SCORE;  break;
  case BB: b = CAP_BISHOP_SCORE; break;
  case BN: b = CAP_KNIGHT_SCORE; break;
  case BP: b = CAP_PAWN_SCORE; break;
  default: assert(0);
  }
  return b*10+a+CAPTURES_FIRST;
}

/*-------------------------------------------------------.
 | function: score_prom(piece prom) -> int               |
 | returns: the score for promoting to piece prom.       |
  -------------------------------------------------------*/
int score_prom(piece prom){
  switch (prom){
  case WQ: return PROM_Q;
  case WR: return PROM_R;
  case WB: return PROM_B;
  case WN: return PROM_N; 
  case BQ: return PROM_Q;
  case BR: return PROM_R;
  case BB: return PROM_B;
  case BN: return PROM_N;
  default: assert(0);
  }
  printf("score_prom: got invalid piece!\n");
  return 0;
}

/*-------------------------------------------------------.
 | function: score_prom_cap(piece prom, piece cap) -> int|
 | returns:  the score for promoting  with capture.      |
  -------------------------------------------------------*/
int score_prom_cap(piece prom, piece cap){
  switch (prom){
  case WQ: return (PROM_Q)+score_cap(WP, cap);
  case WR: return (PROM_R)+score_cap(WP, cap);
  case WB: return (PROM_B)+score_cap(WP, cap);
  case WN: return (PROM_N)+score_cap(WP, cap); 
  case BQ: return (PROM_Q)+score_cap(WP, cap);
  case BR: return (PROM_R)+score_cap(WP, cap);
  case BB: return (PROM_B)+score_cap(WP, cap);
  case BN: return (PROM_N)+score_cap(WP, cap);
  default: assert(0);
  }
  printf("score_prom: got invalid piece!\n");
  return 0;
}
