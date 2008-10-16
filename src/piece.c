#include "piece.h"

/*-------------------------------------------------------.
 | function: make_same_color(piece p1, piece p2)-> piece |
 | returns:  piece p2 with the same color as p1          |
  -------------------------------------------------------*/
piece make_same_color(piece p1, piece p2){
  assert(valid_piece(p1) || ((p1==WHITE) || (p1==BLACK)));
  assert(valid_piece(p2) || ((p2==WHITE) || (p2==BLACK)));
  if (is_white_piece(p1)) return (p2 | 8);
  if (is_black_piece(p1)) return (p2 & 7);
  return 0;
}

/*-------------------------------------------------------.
 | function: valid_piece(piece p) -> boolean             |
 | returns:  true if piece p is a valid piece            |
 | note: the empty is a valid piece                      |
  -------------------------------------------------------*/
int valid_piece(piece p){
  switch (p) {
  case WK: return 1; 
  case WQ: return 1; 
  case WR: return 1; 
  case WB: return 1;
  case WN: return 1; 
  case WP: return 1; 
  case BK: return 1; 
  case BQ: return 1; 
  case BR: return 1; 
  case BB: return 1; 
  case BN: return 1;
  case BP: return 1; 
  case EMPTY: return 1;     
  }
  return 0;
}

/*-------------------------------------------------------.
 | function: valid_piece_not_empty(piece p) -> boolean   |
 | returns:  true if piece p is a valid piece            |
 | note: the empty piece is not valid                    |
  -------------------------------------------------------*/
int valid_piece_not_empty(piece p){
  switch (p) {
  case WK: return 1; 
  case WQ: return 1; 
  case WR: return 1; 
  case WB: return 1;
  case WN: return 1; 
  case WP: return 1; 
  case BK: return 1; 
  case BQ: return 1; 
  case BR: return 1; 
  case BB: return 1; 
  case BN: return 1;
  case BP: return 1; 
  case EMPTY: return 0;     
  }
  return 0;
}
