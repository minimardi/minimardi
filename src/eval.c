#include "eval.h"
/*-------------------------------------------------------.
 | file: eval.c                                          |
 | contains: functions for static evaluation             |
  -------------------------------------------------------*/

static int pawn_pos[128] = {
  0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,   60,60,60,60,60,60,60,60,
  1, 1, 1, -1, -1, 1, 1, 1, 10,10,10,10,10,10,10,10,
  1, 2, 2, 8, 8, 2, 2, 1,   2, 2, 4, 9, 9, 4, 2, 2,
  2, 2, 4, 9, 9, 4, 2, 2,   1, 2, 2, 8, 8, 2, 2, 1,
  10,10,10,10,10,10,10,10,  1, 1, 1, -1, -1, 1, 1, 1,
  60,60,60,60,60,60,60,60,  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0
};
static int good_piece_square[128]= {
  0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 9, 9, 9, 9, 0, 0,
  0, 0, 7, 7, 7, 7, 0, 0,   0, 0, 8, 8, 8, 8, 0, 0,
  0, 0, 7, 8, 8, 7, 0, 0,   0, 0, 7, 8, 8, 7, 0, 0,
  0, 0, 7, 8, 8, 7, 0, 0,   0, 0, 7, 8, 8, 7, 0, 0,
  0, 0, 8, 8, 8, 8, 0, 0,   0, 0, 7, 7, 7, 7, 0, 0,
  0, 0, 9, 9, 9, 9, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0
};
#define X -15
static int good_king_pos[128] = {
  9, 9, 9, 0, 0, 0, 9, 9,   X, X, X, X, X, X, X, X,  
  X, X, X, X, X, X, X, X,   X, X, X, X, X, X, X, X,  
  X, X, X, X, X, X, X, X,   X, X, X, X, X, X, X, X,   
  X, X, X, X, X, X, X, X,   X, X, X, X, X, X, X, X,   
  X, X, X, X, X, X, X, X,   X, X, X, X, X, X, X, X,  
  X, X, X, X, X, X, X, X,   X, X, X, X, X, X, X, X,  
  X, X, X, X, X, X, X, X,   X, X, X, X, X, X, X, X,  
  X, X, X, X, X, X, X, X,   9, 9, 9, 0, 0, 0, 9, 9,
};

/*-------------------------------------------------------.
 | function: eval(position pos) -> int                   |
 | returns: the static evaluation of position pos.       |
 | note: value is always positive                        |
  -------------------------------------------------------*/
int eval(position pos){
  piece p;
  int toMove, i, value=0, wvalue=0, bvalue=0,wQonBoard=0, bQonBoard=0, wKpos=0, bKpos=0;
  board brd;
  int white_pawns[8]; 
  int black_pawns[8];
  int white_rooks[10];
  int black_rooks[10];
  int white_file[8], black_file[8];
  int piece_count=0, bp=0, wp=0, br=0, wr=0;
  int white_two_bishop = 0;
  int black_two_bishop = 0, dst;


  for (i=0; i<8; i++) {
    white_pawns[i] = -1;
    black_pawns[i] = -1;
    white_file[i] = 0;
    black_file[i] = 0;
  }
  for (i=0; i<10; i++) {
    white_rooks[i] = -1;
    black_rooks[i] = -1;
  }

  toMove = getToMove(pos);
  brd = getBoard(pos);

  /* material */
  if (toMove==WHITE) value=getMaterial(pos);
  else value=-getMaterial(pos);

  /* now we check where all the piece are */
  for (i=0; i < 120; ) {
    p=get_piece(brd, i);
    switch (p){
    case WK: 
      wKpos=i; 
      break;
    case WQ: 
      wQonBoard=1; 
      piece_count++;
      break;
    case WR:
      white_rooks[wr]=i;
      wr++;
      piece_count++; 
      break;
    case WB: 
      white_two_bishop++;
      piece_count++; 
      break;
    case WN: 
      piece_count++;
      wvalue += good_piece_square[i]; 
      break;
    case WP:
      white_pawns[wp] = i;
      wp++;
      wvalue += pawn_pos[i]; 
      break;
    case BK: 
      bKpos=i; 
      break;
    case BQ: 
      bQonBoard=1; 
      piece_count++;
      break;
    case BR:
      black_rooks[br]=i;
      br++;
      piece_count++;
      break;
    case BB: 
      black_two_bishop++;
      piece_count++;
      break;
    case BN: 
      piece_count++;
      bvalue += good_piece_square[i+8]; 
      break;
    case BP:
      black_pawns[bp] = i;
      bp++;
      bvalue += pawn_pos[i+8]; 
      break;
    default: break;
    }
    if (!on_board(i+1)) i=i+9;
    else i++;
  }  
  
  if (piece_count < 6) { /* endgame */
    wvalue += good_piece_square[wKpos];
    bvalue += good_piece_square[bKpos];
  }
  else { /* opening/middlegame */
    wvalue += good_king_pos[wKpos];
    bvalue += good_king_pos[bKpos];
    if (white_is_castled(pos)) wvalue += CASTLED;
    if (black_is_castled(pos)) bvalue += CASTLED;
  }
  
  /* check pawn formation */

  /* doubled pawns */
  for (i=0; white_pawns[i] != -1 && i<8; i++)  
    white_file[file(white_pawns[i])-1]++; 
  for (i=0; black_pawns[i] != -1 && i<8; i++) 
    black_file[file(black_pawns[i])-1]++; 
  for (i=0; i < 8; i++) {
    if (white_file[i] > 1) wvalue -= DOUBLE_PAWN * (white_file[i]-1);
    if (black_file[i] > 1) bvalue -= DOUBLE_PAWN * (black_file[i]-1);
  }

  /* check open files for the rooks */

  for (i=0; white_rooks[i] != -1 && i<10; i++) {
    if (rank(white_rooks[i]) == 7) wvalue += ROOK_SEVENTH;
    for(dst = white_rooks[i] + 16; on_board(dst); dst += 16) {
      if (is_empty(get_piece(brd, dst))) continue;
      else if (is_different(get_piece(brd, dst), WR)) {
	wvalue += ROOK_SEMI_OPEN;
	break;
      }
      else break; /* same color on piece */
    }
    if (!on_board(dst)) /* Open file */
      wvalue += ROOK_OPEN_FILE;
  }

  for (i=0; black_rooks[i] != -1 && i<10; i++) {
    if (get_piece(brd, black_rooks[i]) != BR) {
      printf("Eval error: %i\n", get_piece(brd, black_rooks[i]));
      exit(1);
    }
    if (rank(black_rooks[i]) == 2) bvalue += ROOK_SEVENTH;
    for(dst = black_rooks[i] - 16; on_board(dst); dst -= 16) {
      if (is_empty(get_piece(brd, dst))) continue;
      else if (is_different(get_piece(brd, dst), BR)) {
	bvalue += ROOK_SEMI_OPEN;
	break;
      }
      else break;
    }
    if (!on_board(dst))
      bvalue += ROOK_OPEN_FILE;
  }
  /* check for the two bishops */
  if (white_two_bishop > 1) wvalue += TWO_BISHOP;
  if (black_two_bishop > 1) bvalue += TWO_BISHOP;

  /* Now we are done */
  if (toMove==WHITE) value += wvalue-bvalue;
  else value += bvalue-wvalue;
  return value;
}
