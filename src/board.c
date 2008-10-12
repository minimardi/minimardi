#include "board.h"
/*-------------------------------------------------------.
 | file: board.c                                         |
 | contains: functions for manipulating the board.       |
 | note: the array is actually 128 elements big,         |
 |       but only 64 are used for the pieces.            |
  -------------------------------------------------------*/

/*-------------------------------------------------------.
 | function: newboard() -> board                         |
 | returns: a new 8x8 chess board                        |
  -------------------------------------------------------*/
board newboard(){
  board b;
  b = (board) malloc(sizeof(board_));
  return b;
}

/*-------------------------------------------------------.
 | function: free_board(board b) -> void                 |
 | effects: free the board b                             |
  -------------------------------------------------------*/
void free_board(board b){
  free(b);
}

/*-------------------------------------------------------.
 | function: empty_board(board b) -> void                |
 | returns: void                                         |
 | effects: sets all squares to empty on board b         |
 | note:  should not be exported.                        |
  -------------------------------------------------------*/
void empty_board(board b){
  int i=0;
  while (i<128){
    b->board[i]=EMPTY;
    i++;
  }
}

/*-------------------------------------------------------.
 | function: chess_board(board b) -> void                |
 | returns:                                              |
 | effects:  sets the board to the starting position.    |
  -------------------------------------------------------*/
void chess_board(board b){
  empty_board(b);
  /* White */
  b->board[0]=WR;
  b->board[1]=WN;
  b->board[2]=WB;
  b->board[3]=WQ;
  b->board[4]=WK;
  b->board[5]=WB;
  b->board[6]=WN;
  b->board[7]=WR;
  b->board[16]=WP;
  b->board[17]=WP;
  b->board[18]=WP;
  b->board[19]=WP;
  b->board[20]=WP;
  b->board[21]=WP;
  b->board[22]=WP;
  b->board[23]=WP;
  /* Black */
  b->board[112]=BR;
  b->board[113]=BN;
  b->board[114]=BB;
  b->board[115]=BQ;
  b->board[116]=BK;
  b->board[117]=BB;
  b->board[118]=BN;
  b->board[119]=BR;
  b->board[96]=BP;
  b->board[97]=BP;
  b->board[98]=BP;
  b->board[99]=BP;
  b->board[100]=BP;
  b->board[101]=BP;
  b->board[102]=BP;
  b->board[103]=BP;
}

/*-------------------------------------------------------.
 | function: print_board(board b) -> void                |
 | effects: prints the board b to stdout.                |
  -------------------------------------------------------*/
void print_board(board b){
  int i=112;
  piece p;
  while (i > -1) {
    if (!(i & 0x88)){
    p=b->board[i];
    switch(p) {
    case WK: {printf("wK"); break;}
    case WQ: {printf("wQ"); break;}
    case WR: {printf("wR"); break;}
    case WB: {printf("wB"); break;}
    case WN: {printf("wN"); break;}
    case WP: {printf("wP"); break;}
    case BK: {printf("bK"); break;}
    case BQ: {printf("bQ"); break;}
    case BR: {printf("bR"); break;}
    case BB: {printf("bB"); break;}
    case BN: {printf("bN"); break;}
    case BP: {printf("bP"); break;}
    case EMPTY: {printf(".."); break;}
    default: {printf("??:%i",p); break;}
    }
    i++;
    } else {
      printf("\n");
      i=i-24;
    }
  }
}
