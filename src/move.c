#include "move.h"
/*-------------------------------------------------------.
 | function: set_move_*(move mov, X) -> move             |
 | returns:  move mov with X apllied to it, possible     |
 |           values for X: src, dst, capture, promotion. |
 |           set_mov_passant(mov) and set_move_castle(mov)|
 |           set the bit for these kinds of moves        |
  -------------------------------------------------------*/
move set_mov_src(move mov, int src){
  assert(on_board(src));
  mov=mov | (src<<8);
  return mov;
}
move set_mov_dst(move mov, int dst){
  assert(on_board(dst));
  mov=mov | dst;
  return mov;
}
move set_mov_cap_piece(move mov, piece cap){
  assert(valid_piece_not_empty(cap));
  mov=mov | (cap<<16);
  mov=mov | (1 << 20); /* the Cap bit */
  return mov;
}
move set_mov_prom_piece(move mov, piece prom){
  assert(valid_piece_not_empty(prom));
  mov=mov | (prom<<21);
  mov=mov | (1 << 25); 
  return mov;
}
move set_mov_passant(move mov){
  mov=mov | (1 << 26); /* the passant bit */
  return mov; 
}
move set_mov_castle(move mov){
  mov=mov | (1 << 27); /* the castle bit */
  return mov; 
}

move new_move_with_prom(int src, int dst, piece prom_piece){
  move tmp=0;

  assert(on_board(src) && on_board(dst) && valid_piece_not_empty(prom_piece));
  tmp=set_mov_src(tmp, src);
  tmp=set_mov_dst(tmp, dst);
  tmp=set_mov_prom_piece(tmp, prom_piece);
  return tmp;
}

move new_move_with_passant(int src, int dst, piece cap){
  move tmp=0;

  assert(on_board(src) && on_board(dst) && valid_piece_not_empty(cap));
  tmp=new_move_with_cap(src, dst, cap);
  tmp=set_mov_passant(tmp);
  return tmp;
}

/* note new_move_all , buggy, ex cap_piece=0 gives problems... */
move new_move_all(int src, int dst, piece cap, 
		  piece prom, int ca, int pa){
  move tmp=0;

  assert(on_board(src) && on_board(dst) && valid_piece_not_empty(cap)
	 && valid_piece_not_empty(prom));
  tmp=set_mov_src(tmp, src);
  tmp=set_mov_dst(tmp, dst);
  tmp=set_mov_cap_piece(tmp, cap);
  tmp=set_mov_prom_piece(tmp, prom);
  if (pa) tmp=set_mov_passant(tmp);
  if (ca) tmp=set_mov_castle(tmp);
  return tmp;
} 
/*-------------------------------------------------------.
 | function: alg_to_move(char *movealg, char side)->move |
 | returns: given moves of type e2e4 converts to         |
 |          internal representation.                     |
 | note:    the move doesn't contain any other           |
 |          information than src, dst, and opt promotion |
 |          To get the "real" internal move a call to    |
 |          move_generator should give a move with the   |
 |          same src and dst.                            |
  -------------------------------------------------------*/
move alg_to_move(char *movealg, char side){
  char temp[20];
  int src, dst;
  move mov;
  strcpy(temp, movealg);
  src=(temp[0]-97)+((temp[1]-48-1)*16);
  dst=(temp[2]-97)+((temp[3]-48-1)*16);
  mov=new_move(src,dst);
  if (tolower(temp[4])=='q') mov=new_move_with_prom(src,dst, make_same_color(side,BQ));
  if (tolower(temp[4])=='r') mov=new_move_with_prom(src,dst, make_same_color(side,BR));
  if (tolower(temp[4])=='b') mov=new_move_with_prom(src,dst, make_same_color(side,BB));
  if (tolower(temp[4])=='n') mov=new_move_with_prom(src,dst, make_same_color(side,BN));
  return mov;
}

void move_to_alg(move mov, char *movealg){ 
  int src,dst;
  int c1,r1, c2, r2;
  src=get_src(mov);
  dst=get_dst(mov);
  if (src<0 || src > 119 || dst < 0 || dst > 119) {
    printf("move_to_alg: src or dst bad\n");
    exit(1);
  }
  r1 = src/16+1+48;
  c1 = src%16+97;
  r2 = dst/16+1+48;
  c2 = dst%16+97;
  movealg[0]=c1;
  movealg[1]=r1;
  movealg[2]=c2;
  movealg[3]=r2;
  movealg[4]=10;

  if (is_prom_move(mov)) {
    piece p;
    p=get_prom_piece(mov);
    switch (p){
    case WQ: movealg[4]='q'; movealg[5]=10; break;
    case WR: movealg[4]='r'; movealg[5]=10; break;
    case WB: movealg[4]='b'; movealg[5]=10; break;
    case WN: movealg[4]='n'; movealg[5]=10; break; 
    case BQ: movealg[4]='q'; movealg[5]=10; break; 
    case BR: movealg[4]='r'; movealg[5]=10; break; 
    case BB: movealg[4]='b'; movealg[5]=10; break; 
    case BN: movealg[4]='n'; movealg[5]=10; break;
    }
  }	   
}

/*-------------------------------------------------------.
 | function: print_move_alg(move mov)-> void             |
 | returns: void                                         |
 | note: prints the move mov, with the string move first,|
 |       so that xboard recognizes the move.             |
  -------------------------------------------------------*/
void print_move_alg(move mov){
  int src,dst;
  char c1,r1, c2, r2;
  src=get_src(mov);
  dst=get_dst(mov);
  r1 = src/16+1;
  c1 = src%16+97;
  r2 = dst/16+1;
  c2 = dst%16+97;
  if (is_prom_move(mov)) {
    piece p;
    p=get_prom_piece(mov);
    switch (p){
    case WQ: printf("move %c%i%c%iq\n",c1,r1,c2,r2); break;
    case WR: printf("move %c%i%c%ir\n",c1,r1,c2,r2); break;
    case WB: printf("move %c%i%c%ib\n",c1,r1,c2,r2); break;
    case WN: printf("move %c%i%c%in\n",c1,r1,c2,r2); break; 
    case BQ: printf("move %c%i%c%iq\n",c1,r1,c2,r2); break; 
    case BR: printf("move %c%i%c%ir\n",c1,r1,c2,r2); break; 
    case BB: printf("move %c%i%c%ib\n",c1,r1,c2,r2); break; 
    case BN: printf("move %c%i%c%in\n",c1,r1,c2,r2); break;
    }
  }
  else printf("move %c%i%c%i\n",c1,r1,c2,r2);
}

/*-------------------------------------------------------.
 | function: print_move(move mov)-> void                 |
 | returns: void                                         |
 | note: prints the move mov.                            |
  -------------------------------------------------------*/
void print_move(move mov){
  int src,dst;
  char c1,r1, c2, r2;
  src=get_src(mov);
  dst=get_dst(mov);
  r1 = src/16+1;
  c1 = src%16+97;
  r2 = dst/16+1;
  c2 = dst%16+97;
  if (is_prom_move(mov)) {
    piece p;
    p=get_prom_piece(mov);
    switch (p){
    case WQ: printf("%c%i%c%iq",c1,r1,c2,r2); break;
    case WR: printf("%c%i%c%ir",c1,r1,c2,r2); break;
    case WB: printf("%c%i%c%ib",c1,r1,c2,r2); break;
    case WN: printf("%c%i%c%in",c1,r1,c2,r2); break; 
    case BQ: printf("%c%i%c%iq",c1,r1,c2,r2); break; 
    case BR: printf("%c%i%c%ir",c1,r1,c2,r2); break; 
    case BB: printf("%c%i%c%ib",c1,r1,c2,r2); break; 
    case BN: printf("%c%i%c%in",c1,r1,c2,r2); break;
    }
  }
  else printf("%c%i%c%i",c1,r1,c2,r2);
}

int debug_legal_move(move mov){
  int src,dst;

  src=get_src(mov);
  dst=get_dst(mov);
  if (is_prom_move(mov)){
    piece p;
    p=get_prom_piece(mov);
    if (!valid_piece_not_empty(p)) return 0;
  }
  if (is_capture_move(mov)) {
    piece p;
    p = get_cap_piece(mov);
    if (!valid_piece_not_empty(p)) return 0;
    }
  if (on_board(src) && on_board(dst))
    return 1;
  return 0;
}
