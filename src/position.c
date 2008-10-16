#include "position.h"
/*-------------------------------------------------------.
 | file: position.c                                      |
 | contains: everything for finding legal moves and      |
 |           for representing the position.              |
  -------------------------------------------------------*/
static void do_passant(position pos, move mov, int src_opt_dst);
static int find_piece(position pos, piece p);
static int do_material_getval(piece cap_piece);
/* Data for how the pieces can move. */
static int king_dsts[] = {15, 16, 17, -1, 1, -17, -16, -15, 0};
static int bishop_dsts[] = {15, -17, 17, -15, 0};
static int rook_dsts[] = {16, -16,  1, -1, 0};
static int queen_dsts[] = {15, -17, 17, -15, 16, -16,  1, -1, 0};
static int knight_dsts[] = {31, 33, 18, -14, -31, -33, -18, 14, 0};
static int pawn_dsts[] = {16, 32, 15, 17, -16, -32, -17, -15};
/*                        ^--- white --^  ^---- black -----^ */

/*-------------------------------------------------------.
 | functions: repetition detection.                      |
  -------------------------------------------------------*/
static void rep_push(position pos, U64 key){
  if (pos->rep_sp > 1022) {
    printf("You have to increase the repetition stack!\n");
    exit(1);
  }
  pos->rep_stack[pos->rep_sp]=key;
  pos->rep_sp++;
}

static U64 rep_pop(position pos){
  pos->rep_sp--;
  return pos->rep_stack[pos->rep_sp];
} 
/*-------------------------------------------------------.
 | function: rep_search() -> int                         |
 | returns: the number of times the position on top of   |
 |          the repetition stack has occured.            |
  -------------------------------------------------------*/
int rep_search(position pos) {
  int sp=0, count=0;
  U64 key;
  if (pos->rep_sp>0) sp = pos->rep_sp-1;
  key = pos->rep_stack[sp];
  sp--;
  while(sp>=0) {
    if (pos->rep_stack[sp]==key) count++;
    sp--;
  }
  return count;
}


/*-------------------------------------------------------.
 | functions: Zobrist keys                               |
  -------------------------------------------------------*/
U64 rand64()
{
  return rand()^((U64)rand()<<15)^((U64)rand()<<30)^
    ((U64)rand()<<45)^((U64)rand()<<60);
}

void init_zobrist_key(position pos){
  int i=0, j=0;
  for (i=0; i <= 11; i++){
    for (j=0; j <= 127; j++)
      pos->zobrist_key[i][j]=rand64();
  }
}

int get_zobrist_piece(piece p){
  switch (p){
  case WK: return Z_WK;
  case WQ: return Z_WQ;
  case WR: return Z_WR;
  case WB: return Z_WB;
  case WN: return Z_WN;
  case WP: return Z_WP;
  case BK: return Z_BK;
  case BQ: return Z_BQ;
  case BR: return Z_BR;
  case BB: return Z_BB;
  case BN: return Z_BN;
  case BP: return Z_BP;
  }
  assert(0); 
  return 0;
}

U64 get_zobrist_key(position pos){
  board brd;
  int toMove, i;
  piece p;
  U64 key=0;

  brd = pos->b;
  toMove = getToMove(pos);

     for (i=0; i < 120; ) {
      p=get_piece(brd, i);
      switch (p){
      case WK: key = key ^ pos->zobrist_key[Z_WK][i]; break;
      case WQ: key = key ^ pos->zobrist_key[Z_WQ][i]; break;
      case WR: key = key ^ pos->zobrist_key[Z_WR][i]; break;
      case WB: key = key ^ pos->zobrist_key[Z_WB][i]; break;
      case WN: key = key ^ pos->zobrist_key[Z_WN][i]; break;
      case WP: key = key ^ pos->zobrist_key[Z_WP][i]; break;
      case BK: key = key ^ pos->zobrist_key[Z_BK][i]; break;
      case BQ: key = key ^ pos->zobrist_key[Z_BQ][i]; break;
      case BR: key = key ^ pos->zobrist_key[Z_BR][i]; break;
      case BB: key = key ^ pos->zobrist_key[Z_BB][i]; break;
      case BN: key = key ^ pos->zobrist_key[Z_BN][i]; break;
      case BP: key = key ^ pos->zobrist_key[Z_BP][i]; break;
      default: break;
      }
      if (!on_board(i+1)) i=i+9;
      else i++;
    }  
     if (toMove==BLACK) {
       key = key^pos->zobrist_key[0][Z_TOMOVE];
       if (pos->blackPassant)
	 key = key^pos->zobrist_key[1][(pos->passant_piece)+8];
     } else {
       if (pos->whitePassant)
	 key = key^pos->zobrist_key[1][(pos->passant_piece)+8]; 
     }
     if (pos->plyWhiteLostLongCastle) key = key^pos->zobrist_key[0][Z_WCL];
     if (pos->plyWhiteLostShortCastle) key = key^pos->zobrist_key[0][Z_WCS];
     if (pos->plyBlackLostLongCastle) key = key^pos->zobrist_key[0][Z_BCL];
     if (pos->plyBlackLostShortCastle) key = key^pos->zobrist_key[0][Z_BCS];

     return key;
}

/*-------------------------------------------------------.
 | function: new_chess_position() -> position            |
 | returns: the initial chess position.                  |
  -------------------------------------------------------*/
position new_chess_position(){
  position pos;
  pos = (position) malloc(sizeof(position_));
  pos->b=newboard();
  chess_board(pos->b);
  pos->toMove=WHITE;
  pos->side=BLACK; /* default */
  pos->blackPassant=0;
  pos->whitePassant=0; 
  pos->moves_to_draw=0; 
  pos->materialcount=0;
  pos->pieces=16;
  pos->value=0;
  pos->plycount=0;
  pos->plyWhiteLostShortCastle=0;
  pos->plyWhiteLostLongCastle=0;
  pos->plyBlackLostShortCastle=0;
  pos->plyBlackLostLongCastle=0;
  pos->move_stack=new_move_list_stack();

  init_move_list(pos->move_stack);
  pos->moves=new_hist_moves(pos->move_stack); 
  pos->outOfBook=0;
  init_zobrist_key(pos);
  pos->key=get_zobrist_key(pos);
  pos->hist_heuristic=new_history_heuristic();
  clearHistory(pos->hist_heuristic);
  pos->rep_sp=0;
  pos->white_king = 4;
  pos->black_king = 116;
  pos->white_is_castled = 0;
  pos->black_is_castled = 0;
  pos->openingbook = fopen("Book.txt", "r");
  return pos;
}

/*-------------------------------------------------------.
 | function: new_chess_position_fen(char *fen) -> position|
 | returns: the fen chess position.                       |
  -------------------------------------------------------*/
position new_chess_position_fen(char *fen){
 position pos;
 int i, r;
 piece p;

  pos = (position) malloc(sizeof(position_));
  pos->b=newboard();
  empty_board(pos->b);
  pos->side=BLACK; /* default */
  pos->blackPassant=0;
  pos->whitePassant=0; 
  pos->moves_to_draw=0; 
  pos->materialcount=0;
  i = 112;
  r = 0;
  while (*fen) {
    switch (*fen){
    case 'K': set_piece(pos->b, WK, i); i++; break;
    case 'Q': set_piece(pos->b, WQ, i); i++; break;
    case 'R': set_piece(pos->b, WR, i); i++; break;
    case 'B': set_piece(pos->b, WB, i); i++; break;
    case 'N': set_piece(pos->b, WN, i); i++; break;
    case 'P': set_piece(pos->b, WP, i); i++; break;
    case 'k': set_piece(pos->b, BK, i); i++; break;
    case 'q': set_piece(pos->b, BQ, i); i++; break;
    case 'r': set_piece(pos->b, BR, i); i++; break;
    case 'b': set_piece(pos->b, BB, i); i++; break;
    case 'n': set_piece(pos->b, BN, i); i++; break;
    case 'p': set_piece(pos->b, BP, i); i++; break;
    case '1': i += 1; break;
    case '2': i += 2; break;
    case '3': i += 3; break;
    case '4': i += 4; break;
    case '5': i += 5; break;
    case '6': i += 6; break;
    case '7': i += 7; break;
    case '8': i += 8; break;
    case '/': r++; i = 112 - r*16; break;
    default:  printf("Bad FEN position: piece %i\n", i); exit(1);
    }
    fen++;
    if ((*fen) == ' ') break;
  }    
  /* active color */
  fen++;
  switch (*fen){
  case 'w': pos->toMove=WHITE; fen++; break;
  case 'b': pos->toMove=BLACK; fen++; break;
  default:  printf("Bad FEN position: active color %c\n", *fen); exit(1);
  }

  fen++;
 
  pos->plyWhiteLostShortCastle=1;
  pos->plyWhiteLostLongCastle=1;
  pos->plyBlackLostShortCastle=1;
  pos->plyBlackLostLongCastle=1;
  
  /* castling */
  while (*fen){
    if ((*fen)=='-' || (*fen)==' ') {fen++; break;}
    switch (*fen){
    case 'K': pos->plyWhiteLostShortCastle=0; break;
    case 'Q': pos->plyWhiteLostLongCastle=0; break;
    case 'k': pos->plyBlackLostShortCastle=0; break;
    case 'q': pos->plyBlackLostLongCastle=0; break;
    }
    fen++;
  }

  /* en passant target square */
  
  if ((*fen) && (*fen)!='-' && (*fen) != ' ') {
    int sq;
    char file, rank;
    file = (*fen);
    fen++;
    if (!*fen) printf("Bad FEN position\n"); exit(1);
    rank = (*fen);
    sq=(file-97)+((rank-48-1)*16);
    if (sq < 0 || sq > 119) printf("Bad FEN position: en passant square\n"); exit(1);
    if (rank == 3) do_passant(pos, new_move(sq-16, sq+16), sq+16);
    else if (rank == 6)
      do_passant(pos, new_move(sq+16, sq-16), sq-16);
  }
  
  fen++;

  /* halfmove clock */
  if ((*fen)) {
    pos->moves_to_draw = (*fen);
  }
  
  pos->value=0;
  pos->plycount=2;
  pos->move_stack=new_move_list_stack();
  init_move_list(pos->move_stack);
  pos->moves=new_hist_moves(pos->move_stack); 
  pos->outOfBook=1;
  pos->key=get_zobrist_key(pos);
  pos->hist_heuristic=new_history_heuristic();
  clearHistory(pos->hist_heuristic);
  pos->rep_sp=0;
  pos->white_king = find_piece(pos, WK);
  pos->black_king = find_piece(pos, BK);
  pos->pieces=0;

  for (i=0; i<120; i++) {
    p = get_piece(getBoard(pos), i);
    if ((p != BP) && (p != WP) && (p != EMPTY)) pos->pieces++;
    pos->materialcount +=-do_material_getval(p);
  }
  return pos;
}

/*-------------------------------------------------------.
 | functions: to set stuff in the position               |
  -------------------------------------------------------*/
void setZobristKey(position pos, U64 key){
  pos->key=key;
}
void setOutOfBook(position pos){
  pos->outOfBook=1;
}
void setValue(position pos, int value, int nodes){
  pos->value=value;
  pos->nodes=nodes;
}
void setToMove(position pos, char tomove){
  pos->toMove=tomove;
} 
void setPassantPiece(position pos, char dst){
  pos->passant_piece = dst;
}
void setWhitePassant(position pos, boolean wp){
  pos->whitePassant=wp;
}
void setBlackPassant(position pos, boolean bp){
  pos->blackPassant=bp;
}
void setSideWhite(position pos){
  pos->side=WHITE;
}
void setSideBlack(position pos){
  pos->side=BLACK;
}
void incPly(position pos){
  pos->plycount++;
}
void decPly(position pos){
  pos->plycount--;
}

/*-------------------------------------------------------.
 | functions: to get stuff from the position             |
  -------------------------------------------------------*/
FILE *getBook(position pos){
  return pos->openingbook;
}

U64 getZobristKey(position pos){
  return pos->key;
}
int isCompBlack(position pos){
  if (pos->side==BLACK) return 1;
  return 0;
}
int getOutOfBook(position pos){
  return pos->outOfBook;
}
int getValue(position pos){
  return pos->value;
}
int getNodes(position pos){
  return pos->nodes;
}
move_list getPosMoves(position pos){
  return pos->moves;
}
piece getCompSide(position pos){
  return pos->side;
}
piece getOpponentSide(position pos){
  if (pos->side==BLACK) return WHITE;
  return BLACK;
}
char getToMove(position pos){
  return pos->toMove;
}
int getMaterial(position pos){
  return pos->materialcount;
}
int getPly(position pos){
  return pos->plycount;
}
char getWhitePassant(position pos){
  return pos->whitePassant;
}
char getBlackPassant(position pos){
  return pos->blackPassant;
}
void whiteNoShortCastle(position pos, int ply){
  pos->plyWhiteLostShortCastle=ply;
}
void whiteNoLongCastle(position pos, int ply){
  pos->plyWhiteLostLongCastle=ply;
}
void blackNoShortCastle(position pos, int ply){
  pos->plyBlackLostShortCastle=ply;
}
void blackNoLongCastle(position pos, int ply){
  pos->plyBlackLostLongCastle=ply;
}
/* 0 = castle legal, >0 = castle not legal */
int getBlackCastleLong(position pos){
  return pos->plyBlackLostLongCastle;
}
int getBlackCastleShort(position pos){
  return pos->plyBlackLostShortCastle;
}
int getWhiteCastleLong(position pos){
  return pos->plyWhiteLostLongCastle;
}
int getWhiteCastleShort(position pos){
  return pos->plyWhiteLostShortCastle;
}
int white_is_castled(position pos){
  return pos->white_is_castled;
}
int black_is_castled(position pos){
  return pos->black_is_castled;
}
history_heuristic get_hist_heuristic(position pos){
  return pos->hist_heuristic;
}
move_list_stack get_move_stack(position pos){
  return pos->move_stack;
}

/*-------------------------------------------------------.
 | function: free_position(position pos) -> void         |
 | effects: free the position pos                        |
  -------------------------------------------------------*/
void free_position(position pos){
  if (pos) {
    free_board(pos->b);
    if (pos->openingbook) fclose(pos->openingbook);
    free_history_heuristic(pos->hist_heuristic);
    free(pos);
  }
}

/*-------------------------------------------------------.
 | function: do_material_getval(piece cap_piece)->int    |
 | returns:  returns the value of cap_piece              |
  -------------------------------------------------------*/
int do_material_getval(piece cap_piece){
  switch (cap_piece) {
  case WK: return -VALUE_KING; break;
  case WQ: return -VALUE_QUEEN; break;
  case WR: return -VALUE_ROOK;  break;
  case WB: return -VALUE_BISHOP; break;
  case WN: return -VALUE_KNIGHT; break;
  case WP: return -VALUE_PAWN; break;
  case BK: return VALUE_KING; break;
  case BQ: return VALUE_QUEEN; break;
  case BR: return VALUE_ROOK; break;
  case BB: return VALUE_BISHOP; break;
  case BN: return VALUE_KNIGHT; break;
  case BP: return VALUE_PAWN; break;
  case EMPTY: return 0; break;    
  }
  assert(0);
  return 0;
}
/*-------------------------------------------------------.
 | function: do_material(position pos, move mov)->void   |
 | returns: void                                         |
 | effects: calculates the new value for the position    |
 |          for the move mov (a capture move).           |
  -------------------------------------------------------*/
void do_material(position pos, move mov){
  piece p, cap;
  if (is_capture_move(mov)){
    cap = get_cap_piece(mov);
    if ((cap != BP) && (cap != WP)) pos->pieces--;
    pos->materialcount=pos->materialcount+do_material_getval(cap);
    if (is_prom_move(mov)){
      pos->pieces++; /* canno't promote to a pawn */
      if (getToMove(pos)==WHITE) p=WP; else p=BP;
      pos->materialcount=pos->materialcount
	-do_material_getval(get_prom_piece(mov))
	+do_material_getval(p);
    }
  }
  else if (is_prom_move(mov)){
    pos->pieces++;
    if (getToMove(pos)==WHITE) p=WP; else p=BP;
    pos->materialcount=pos->materialcount
      -do_material_getval(get_prom_piece(mov))
      +do_material_getval(p);
  }
}
/*-------------------------------------------------------.
 | function: undo_material(position pos, move mov)->void |
 | returns: void                                         |
 | effects: undos the value given by move mov.           |
  -------------------------------------------------------*/
void undo_material(position pos, move mov){
  piece p, cap;
  if (is_capture_move(mov)) {
    cap = get_cap_piece(mov);
    if ((cap != BP) && (cap != WP)) pos->pieces++;
    pos->materialcount=pos->materialcount-do_material_getval(cap);
    if (is_prom_move(mov)) {
      pos->pieces--;
      if (getToMove(pos)==WHITE) p=WP; else p=BP;
      pos->materialcount=pos->materialcount
	+do_material_getval(get_prom_piece(mov))
	-do_material_getval(p);
    }
  }
  else if (is_prom_move(mov)){
    pos->pieces--;
    if (getToMove(pos)==WHITE) p=WP; else p=BP;
    pos->materialcount=pos->materialcount
      +do_material_getval(get_prom_piece(mov))
      -do_material_getval(p);
  }
}
/*-------------------------------------------------------.
 | function: do_passant(pos, move mov, int src) -> void  |
 | effects: if mov is a pawn move (2 square move) and    |
 |          the move gives the opponent the right to     |
 |          capture en passant, the passant_piece and    |
 |          the boolean value is set.                    |
 | note: src is the location of the piece (pawn)         |
 | note: the function assumes that the piece on src is   |
 |       the side toMove in pos                          |
  -------------------------------------------------------*/
void do_passant(position pos, move mov, int src_opt_dst){
  piece p;
  int src, dst, srcleft, srcright;
  board brd;

  src = get_src(mov);
  dst = get_dst(mov);
  brd = getBoard(pos);
  p = get_piece(brd, src_opt_dst);

  if ((p==WP) && (on_board(src+32))&&(dst==(src+32))) {
    srcleft=dst-1; 
    srcright=dst+1;
    if (on_board(srcleft) && (get_piece(brd, srcleft)==BP)) {
      setBlackPassant(pos, 1);
      setPassantPiece(pos, dst);
    }
    else if (on_board(srcright) && (get_piece(brd, srcright)==BP)) {
      setBlackPassant(pos, 1);
      setPassantPiece(pos, dst);
    }
  }
  else if ((p==BP) && (on_board(src-32))&&(dst==(src-32))) {
    srcleft=dst-1; 
    srcright=dst+1;
    if (on_board(srcleft) && (get_piece(brd, srcleft)==WP)) {
      setWhitePassant(pos, 1);
      setPassantPiece(pos, dst);
    }
    else if (on_board(srcright) && (get_piece(brd, srcright)==WP)) {
      setWhitePassant(pos, 1);
      setPassantPiece(pos, dst);
    }
  }
}

/*-------------------------------------------------------.
 | function: do_move(position pos, move mov)->void       |
 | returns: void                                         |
 | effects: makes the move mov in position pos, changes  |
 |          toMove.                                      |
 | note: mov HAS to be a "psuedo legal" move in pos.     |
  -------------------------------------------------------*/
void do_move(position pos, move mov){
  int src, dst;
  int z_wcs, z_wcl, z_bcs, z_bcl, z_pass, z_bpass, z_wpass; /* Saved Vars */
  piece p;
  board brd;
  U64 new_key;

  /* Save vars for easy comparision after the pos changes */
  z_wcs = getWhiteCastleShort(pos);
  z_wcl = getWhiteCastleLong(pos);
  z_bcl = getBlackCastleLong(pos);
  z_bcs = getBlackCastleShort(pos);
  z_wpass = pos->whitePassant;
  z_bpass = pos->blackPassant;
  z_pass = pos->passant_piece;
  new_key = getZobristKey(pos);

  if (mov==NULL_MOVE_PRUNING) { /* NULLMOVE */
    if (pos->toMove==WHITE) 
      pos->toMove=BLACK;
    else pos->toMove=WHITE;
    setWhitePassant(pos, 0); /* Clear passant values */
    setBlackPassant(pos, 0);
    pos->passant_piece = 0;
    new_key = new_key^pos->zobrist_key[0][Z_TOMOVE];
    if (z_wpass) new_key ^= pos->zobrist_key[1][(z_pass)+8];
    else if (z_bpass) new_key ^= pos->zobrist_key[1][(z_pass)+8];
    setZobristKey(pos, new_key);
    push_hist(pos->move_stack, NULL_MOVE_PRUNING);
    incPly(pos);
    return;
  }

  push_hist(pos->move_stack, mov);
  src = get_src(mov);
  dst = get_dst(mov);
  brd = getBoard(pos);
  p = get_piece(brd, src);
  assert(on_board(src) && on_board(dst) && (p!=EMPTY));
  incPly(pos);

  if (is_capture_move(mov)) {
    do_material(pos, mov);
    /* A capture move => we have to remove the cap piece from the key */
    new_key ^= pos->zobrist_key[get_zobrist_piece(get_cap_piece(mov))][dst];
  } 
  else if (is_prom_move(mov)) do_material(pos, mov);

  /* delete the piece we are moving, and then add it when we know what piece it is (prom?) */
  new_key ^= pos->zobrist_key[get_zobrist_piece(p)][src];

  set_piece(brd, p, dst);  /* note: promotion is done below */
  set_piece(brd, EMPTY, src);

  /* check for Rook and King move => whiteCastleX =0; */
  switch(p) {
  case WK: 
    pos->white_king = dst;
    if (!getWhiteCastleShort(pos)) whiteNoShortCastle(pos, getPly(pos));
    if (!getWhiteCastleLong(pos)) whiteNoLongCastle(pos, getPly(pos));
    break;
  case BK:
    pos->black_king = dst;
    if (!getBlackCastleShort(pos)) blackNoShortCastle(pos, getPly(pos));
    if (!getBlackCastleLong(pos)) blackNoLongCastle(pos, getPly(pos));
    break;
  case WR:
    if (src==0 && !getWhiteCastleLong(pos))  whiteNoLongCastle(pos, getPly(pos));
    if (src==7 && !getWhiteCastleShort(pos))  whiteNoShortCastle(pos, getPly(pos));
    break;
  case BR:
    if (src==112 && !getBlackCastleLong(pos)) blackNoLongCastle(pos, getPly(pos));
    if (src==119 && !getBlackCastleShort(pos))  blackNoShortCastle(pos, getPly(pos));    
    break;
  }
  /* passant not taken */
  if (is_white_piece(p) && (getWhitePassant(pos)) && !(is_passant_move(mov))){
    setWhitePassant(pos, 0);
    pos->passant_piece=0; /* Add: tor dec 11 00:13:45 CET 2003*/
  }
  else if (is_black_piece(p) && (getBlackPassant(pos)) && !(is_passant_move(mov))){
    setBlackPassant(pos, 0);
    pos->passant_piece=0; /* Add: tor dec 11 00:13:45 CET 2003 */
  }
  do_passant(pos, mov, dst); /* sets rights if the move gives "en passant" for opp. */

  /* Now we manipulate the board */
  if (is_castle_move(mov)) /* castle */
    {
      if (getToMove(pos)==WHITE) {
	pos->white_is_castled = 1;
	if (dst==6) {
	  set_piece(brd, EMPTY, 7);
	  set_piece(brd, WR, 5);
	  new_key ^= pos->zobrist_key[get_zobrist_piece(WR)][7];
	  new_key ^= pos->zobrist_key[get_zobrist_piece(WR)][5];
	  if (!getWhiteCastleShort(pos)) whiteNoShortCastle(pos, getPly(pos));
	  if (!getWhiteCastleLong(pos)) whiteNoLongCastle(pos, getPly(pos)); 
	}
	else /* dst==2 */
	  {
	    set_piece(brd, EMPTY, 0);
	    set_piece(brd, WR, 3);
	    new_key ^= pos->zobrist_key[get_zobrist_piece(WR)][0];
	    new_key ^= pos->zobrist_key[get_zobrist_piece(WR)][3];
	    if (!getWhiteCastleShort(pos)) whiteNoShortCastle(pos, getPly(pos));
	    if (!getWhiteCastleLong(pos)) whiteNoLongCastle(pos, getPly(pos));  
	  }
      }
      else /* toMove == BLACK */
	{
	  pos->black_is_castled = 1;
	  if (dst==118) {
	    set_piece(brd, EMPTY, 119);
	    set_piece(brd, BR, 117);
	    new_key ^= pos->zobrist_key[get_zobrist_piece(BR)][119];
	    new_key ^= pos->zobrist_key[get_zobrist_piece(BR)][117];
	    if (!getBlackCastleShort(pos)) blackNoShortCastle(pos, getPly(pos));
	    if (!getBlackCastleLong(pos)) blackNoLongCastle(pos, getPly(pos)); 
	    
	  }
	  else /* dst==114 */
	    {
	      set_piece(brd, EMPTY, 112);
	      set_piece(brd, BR, 115);
	      new_key ^= pos->zobrist_key[get_zobrist_piece(BR)][112];
	      new_key ^= pos->zobrist_key[get_zobrist_piece(BR)][115];
	      if (!getBlackCastleShort(pos)) blackNoShortCastle(pos, getPly(pos));
	      if (!getBlackCastleLong(pos)) blackNoLongCastle(pos, getPly(pos)); 
	    }
	}
    }
  else if (is_passant_move(mov)) /* passant */
    {
      if (getToMove(pos)==WHITE) {
	set_piece(brd, EMPTY, dst-16);
	setWhitePassant(pos, 0);
	pos->passant_piece=0; /* Add: tor dec 11 00:13:45 CET 2003*/
      }
      else {
	set_piece(brd, EMPTY, dst+16);
	setBlackPassant(pos, 0);
	pos->passant_piece=0; /* Add: tor dec 11 00:13:45 CET 2003*/
      }
    }
  else if (is_prom_move(mov)) /* promotion */
    {
      p = get_prom_piece(mov); 
      assert(p);
      set_piece(brd, p, dst); /* note we overwrite */
      
    }
  /* change side to move */
  if (pos->toMove==WHITE) 
    pos->toMove=BLACK;
  else pos->toMove=WHITE;
  /* Do some zobrist stuff */
  /* now if the move was a prom, p is set to the prom_piece else it is unchanged.*/
  new_key ^= pos->zobrist_key[get_zobrist_piece(p)][dst];
  /* change side to move */
  new_key = new_key^pos->zobrist_key[0][Z_TOMOVE];
   
  if (z_wcs != getWhiteCastleShort(pos)) new_key ^= pos->zobrist_key[0][Z_WCS];
  if (z_wcl != getWhiteCastleLong(pos)) new_key ^= pos->zobrist_key[0][Z_WCL];
  if (z_bcl != getBlackCastleLong(pos)) new_key ^= pos->zobrist_key[0][Z_BCL];
  if (z_bcs != getBlackCastleShort(pos)) new_key ^= pos->zobrist_key[0][Z_BCS];
  if (z_wpass) new_key ^= pos->zobrist_key[1][(z_pass)+8];
  if (z_bpass) new_key ^= pos->zobrist_key[1][(z_pass)+8];
  if (getWhitePassant(pos)) new_key ^= pos->zobrist_key[1][(pos->passant_piece)+8];
  if (getBlackPassant(pos)) new_key ^= pos->zobrist_key[1][(pos->passant_piece)+8];
  setZobristKey(pos, new_key);
  /* push this new key to the repetition stack */
  rep_push(pos, new_key);
}

/*-------------------------------------------------------.
 | function: undo_move(position pos, move mov) -> void   |
 | returns: void                                         |
 | effects: undos the move mov in position pos, changes  |
 |          to toMove.                                   |
 | note: mov HAS to be a "pseudo legal" move in pos.     |
  -------------------------------------------------------*/
void undo_move(position pos, move mov){
  int src, dst, ply;
  int z_wcs, z_wcl, z_bcs, z_bcl, z_pass, z_bpass, z_wpass; /* Saved Vars */
  move opp_mov;
  piece p;
  board brd;
  U64 new_key;


  /* Save vars for easy comparision after the pos changes */
  z_wcs = getWhiteCastleShort(pos);
  z_wcl = getWhiteCastleLong(pos);
  z_bcl = getBlackCastleLong(pos);
  z_bcs = getBlackCastleShort(pos);
  z_wpass = pos->whitePassant;
  z_bpass = pos->blackPassant;
  z_pass = pos->passant_piece;
  new_key = getZobristKey(pos);

  if (mov==NULL_MOVE_PRUNING) { /* NULLMOVE */
    pop_hist(pos->move_stack, getPosMoves(pos));
    if (pos->toMove==WHITE) 
      pos->toMove=BLACK;
    else pos->toMove=WHITE;

    setWhitePassant(pos, 0);
    setBlackPassant(pos, 0);
    opp_mov = look_hist(pos->move_stack, getPosMoves(pos));
    if (opp_mov!=NULL_MOVE) do_passant(pos, opp_mov, get_dst(opp_mov)); 
    if (pos->whitePassant) new_key ^= pos->zobrist_key[1][(pos->passant_piece)+8];
    else if (pos->blackPassant) new_key ^= pos->zobrist_key[1][(pos->passant_piece)+8];
    new_key = new_key^pos->zobrist_key[0][Z_TOMOVE];
    setZobristKey(pos, new_key);
    decPly(pos);
    return;
  }
  
  pop_hist(pos->move_stack, getPosMoves(pos));

  src = get_src(mov);
  dst = get_dst(mov);
  brd = getBoard(pos);
  p = get_piece(brd, dst);
  assert(on_board(src) && on_board(dst) && (p!=EMPTY));

  if (p==WK) pos->white_king = src;
  else if (p==BK) pos->black_king = src;
	 
  ply=getPly(pos);
  if (ply>1){
    if (ply==getWhiteCastleLong(pos)) whiteNoLongCastle(pos, 0); /* makes castle true */
    if (ply==getWhiteCastleShort(pos)) whiteNoShortCastle(pos, 0);
    if (ply==getBlackCastleLong(pos)) blackNoLongCastle(pos, 0);
    if (ply==getBlackCastleShort(pos)) blackNoShortCastle(pos, 0);
  }
  decPly(pos);
  
  if ((p==WP) && (on_board(src+32))&&(dst==(src+32))) {
    int srcleft, srcright;
    srcleft=dst-1; 
    srcright=dst+1;
    if (on_board(srcleft) && (get_piece(brd, srcleft)==BP)) {
      setBlackPassant(pos, 0);
      pos->passant_piece=0; /* Add: tor dec 11 00:13:45 CET 2003*/
    }
    if (on_board(srcright) && (get_piece(brd, srcright)==BP)) {
      setBlackPassant(pos, 0);
      pos->passant_piece=0; /* Add: tor dec 11 00:13:45 CET 2003*/
    }
  }
  
  if ((p==BP) &&(on_board(src-32))&&(dst==(src-32))) {
    int srcleft, srcright;
    srcleft=dst-1; 
    srcright=dst+1;
    if (on_board(srcleft) && (get_piece(brd, srcleft)==WP)) {
      setWhitePassant(pos, 0);
      pos->passant_piece=0; /* Add: tor dec 11 00:13:45 CET 2003*/
    }
     if (on_board(srcright) && (get_piece(brd, srcright)==WP)) {
       setWhitePassant(pos, 0);
       pos->passant_piece=0; /* Add: tor dec 11 00:13:45 CET 2003*/
     }
  }

  /* change side to move */
  if (pos->toMove==WHITE)
    pos->toMove=BLACK;
  else pos->toMove=WHITE;

  if (is_capture_move(mov) || is_prom_move(mov)) undo_material(pos, mov);
 

  /* Now we manipulate the board */
  if (is_prom_move(mov)) {
    if (is_capture_move(mov)){
      assert(make_same_color(p, WP));
      assert(get_cap_piece(mov));
      set_piece(brd, get_cap_piece(mov), dst); 
      set_piece(brd, make_same_color(p,WP), src); 
      new_key ^= pos->zobrist_key[get_zobrist_piece(p)][dst];
      new_key ^= pos->zobrist_key[get_zobrist_piece(make_same_color(p,WP))][src];
      new_key ^= pos->zobrist_key[get_zobrist_piece(get_cap_piece(mov))][dst];
    }
    else
      {
	assert(make_same_color(p, WP));
	set_piece(brd, EMPTY, dst);
	set_piece(brd, make_same_color(p,WP), src);
	new_key ^= pos->zobrist_key[get_zobrist_piece(p)][dst];
	new_key ^= pos->zobrist_key[get_zobrist_piece(make_same_color(p,WP))][src];
      }
  }
  else
    if (is_capture_move(mov)) {
      new_key ^= pos->zobrist_key[get_zobrist_piece(p)][dst];
      new_key ^= pos->zobrist_key[get_zobrist_piece(p)][src];
      new_key ^= pos->zobrist_key[get_zobrist_piece(get_cap_piece(mov))][dst];
      if (is_passant_move(mov)) {
	if (is_white_piece(p)){
	  set_piece(brd, WP, src);
	  set_piece(brd, EMPTY, dst);
	  set_piece(brd, BP, dst-16);
	  pos->passant_piece=dst-16; /* the pawn that can be taken */
	}
	else /* BLACK */
	  {
	    set_piece(brd, BP, src);
	    set_piece(brd, EMPTY, dst);
	    set_piece(brd, WP, dst+16);  
	    pos->passant_piece=dst+16;
	  }
      }
      else
	{
	  assert(get_cap_piece(mov));
	  set_piece(brd, get_cap_piece(mov), dst);
	  set_piece(brd, p, src);
	}
    }
    else 
      if (is_castle_move(mov)) {
	switch (dst) {
	case 6: {
	  pos->white_is_castled = 0;
	  set_piece(brd, WK, 4); 
	  set_piece(brd, EMPTY, 6);
	  set_piece(brd, WR, 7); 
	  set_piece(brd, EMPTY, 5); 
	  whiteNoShortCastle(pos, 0);
	  new_key ^= pos->zobrist_key[get_zobrist_piece(WK)][6];
	  new_key ^= pos->zobrist_key[get_zobrist_piece(WK)][4];
	  new_key ^= pos->zobrist_key[get_zobrist_piece(WR)][5];
	  new_key ^= pos->zobrist_key[get_zobrist_piece(WR)][7];
	  break;
	}
	case 2: {
	  pos->white_is_castled = 0;
	  set_piece(brd, WK, 4); 
	  set_piece(brd, EMPTY, 2);
	  set_piece(brd, WR, 0); 
	  set_piece(brd, EMPTY, 3); 
	  whiteNoLongCastle(pos, 0);
	  new_key ^= pos->zobrist_key[get_zobrist_piece(WK)][2];
	  new_key ^= pos->zobrist_key[get_zobrist_piece(WK)][4];
	  new_key ^= pos->zobrist_key[get_zobrist_piece(WR)][3];
	  new_key ^= pos->zobrist_key[get_zobrist_piece(WR)][0];
	  break;
	}
	case 118: {
	  pos->black_is_castled = 0;
	  set_piece(brd, BK, 116); 
	  set_piece(brd, EMPTY, 118);
	  set_piece(brd, BR, 119); 
	  set_piece(brd, EMPTY, 117);
	  blackNoShortCastle(pos, 0);
	  new_key ^= pos->zobrist_key[get_zobrist_piece(BK)][118];
	  new_key ^= pos->zobrist_key[get_zobrist_piece(BK)][116];
	  new_key ^= pos->zobrist_key[get_zobrist_piece(BR)][117];
	  new_key ^= pos->zobrist_key[get_zobrist_piece(BR)][119];
	  break;
	}
	case 114: {
	  pos->black_is_castled = 0;
	  set_piece(brd, BK, 116); 
	  set_piece(brd, EMPTY, 114);
	  set_piece(brd, BR, 112); 
	  set_piece(brd, EMPTY, 115); 
	  blackNoLongCastle(pos, 0);
	  new_key ^= pos->zobrist_key[get_zobrist_piece(BK)][114];
	  new_key ^= pos->zobrist_key[get_zobrist_piece(BK)][116];
	  new_key ^= pos->zobrist_key[get_zobrist_piece(BR)][115];
	  new_key ^= pos->zobrist_key[get_zobrist_piece(BR)][112];
	  break;
	}
	}
      }
      else
	/* just a move */
	{
	  set_piece(brd, EMPTY, dst);
	  assert(p);
	  set_piece(brd, p, src);
	  new_key ^= pos->zobrist_key[get_zobrist_piece(p)][dst];
	  new_key ^= pos->zobrist_key[get_zobrist_piece(p)][src];
	}
  /* undo move that was not a passant move (but the position */
  /* had a legal passant move) => we have to set the variable. */
  opp_mov = look_hist(pos->move_stack, getPosMoves(pos));
  if (opp_mov!=NULL_MOVE) do_passant(pos, opp_mov, get_dst(opp_mov)); 

 
  /* Do some zobrist stuff */
  
  new_key = new_key^pos->zobrist_key[0][Z_TOMOVE];
   
  if (z_wcs != getWhiteCastleShort(pos)) new_key ^= pos->zobrist_key[0][Z_WCS];
  if (z_wcl != getWhiteCastleLong(pos)) new_key ^= pos->zobrist_key[0][Z_WCL];
  if (z_bcl != getBlackCastleLong(pos)) new_key ^= pos->zobrist_key[0][Z_BCL];
  if (z_bcs != getBlackCastleShort(pos)) new_key ^= pos->zobrist_key[0][Z_BCS];
  if (z_wpass) new_key ^= pos->zobrist_key[1][(z_pass)+8];
  if (z_bpass) new_key ^= pos->zobrist_key[1][(z_pass)+8];
  if (getWhitePassant(pos)) new_key ^= pos->zobrist_key[1][(pos->passant_piece)+8];
  if (getBlackPassant(pos)) new_key ^= pos->zobrist_key[1][(pos->passant_piece)+8];
  setZobristKey(pos, new_key);
  /* pop this key from the repetition stack */
  rep_pop(pos);
}

/*-------------------------------------------------------.
 | function: last_move_null(pos) -> boolean              |
 | returns: true if last move was a nullmove.            |
  -------------------------------------------------------*/
int last_move_null(position pos){
  if (look_hist(pos->move_stack, getPosMoves(pos))==NULL_MOVE_PRUNING)
    return 1;
  return 0;
}

/*-------------------------------------------------------.
 | function: pieces(position pos) -> int                 |
 | returns: # pieces in the position.                    |
  -------------------------------------------------------*/
int pieces(position pos){
  return pos->pieces;
}

/*-------------------------------------------------------.
 | function: legal_opponent_check(position pos)->boolean |
 | returns: true/false                                   |
 | note: help function for legal_opponent_move           |
 |       and isGameOver.                                 |
  -------------------------------------------------------*/
int legal_opponent_check(position pos){
  move_list moves;
  move mov;

  moves = move_generator(pos);
  mov = pop(pos->move_stack, moves);
  while (mov!=NULL_MOVE){
    if (!legal_move(mov)) {
      free_move_list(pos->move_stack,moves);
      return 0;
    }
    mov = pop(pos->move_stack, moves);
  }
  free_move_list(pos->move_stack,moves);
  return 1;  
}

/*-------------------------------------------------------.
 | function: legal_opponent_move(position pos, move mov) |
 |                                  -> move              |
 | returns:  the internal represention of the move if    |
 |           the move is legal, else 0.                  |
 | note:     the input (mov) needs only to have a src    |
 |           and dst part, the rest is added by the      |
 |           function.                                   |
  -------------------------------------------------------*/
move legal_opponent_move(position pos, move mov){
  move_list moves;
  move mov2;
  int src, dst, src2,dst2, next_move;

  src = get_src(mov);
  dst = get_dst(mov);
  moves = move_generator(pos);
  mov2 = pop(pos->move_stack,moves);
  while (mov2!=NULL_MOVE){
    do_move(pos, mov2);
    next_move=legal_opponent_check(pos);
    undo_move(pos, mov2);

    src2=get_src(mov2);
    dst2=get_dst(mov2);
    if (is_prom_move(mov) && (src==src2) && (dst==dst2) && next_move &&
	get_prom_piece(mov) == get_prom_piece(mov2)){
      free_move_list(pos->move_stack,moves);
      return mov2;
    }     
    else if ((src==src2) && (dst==dst2) && next_move && (!is_prom_move(mov))) {
      free_move_list(pos->move_stack,moves);
      return mov2;
    } 
    mov2 = pop(pos->move_stack,moves);
  }
  free_move_list(pos->move_stack,moves);
  return NULL_MOVE;
}

void debug_print_cap_list(move_list lstptr,position pos){
  move mov;
  int i=1;
  piece p, cap;
  mov = look_stack(pos->move_stack,lstptr, 1);
  while (mov!=NULL_MOVE&&is_capture_move(mov)) {
    printf("Mini: ");
    cap = get_cap_piece(mov);
    p = get_piece(getBoard(pos),get_src(mov));
    switch(p){
    case WK: printf("WK"); break;
    case WQ: printf("WQ"); break;
    case WR: printf("WR"); break;
    case WB: printf("WB"); break;
    case WN: printf("WN"); break;
    case WP: printf("WP"); break;
    case BK: printf("BK"); break; 
    case BQ: printf("BQ"); break;  
    case BR: printf("BR"); break;  
    case BB: printf("BB"); break; 
    case BN: printf("BN"); break; 
    case BP: printf("BP"); break;
    }
    printf("x");
    switch(cap){
    case WK: printf("WK"); break;
    case WQ: printf("WQ"); break;
    case WR: printf("WR"); break;
    case WB: printf("WB"); break;
    case WN: printf("WN"); break;
    case WP: printf("WP"); break;
    case BK: printf("BK"); break; 
    case BQ: printf("BQ"); break;  
    case BR: printf("BR"); break;  
    case BB: printf("BB"); break; 
    case BN: printf("BN"); break; 
    case BP: printf("BP"); break;
    }
    printf(" ");
    print_move_alg(mov);
    i++;
    mov=look_stack(pos->move_stack,lstptr, i);
  }
}

/*-------------------------------------------------------.
 | function: print_position(pos) -> void                 |
 | returns: void, prints the board                       |
  -------------------------------------------------------*/
void print_position(position pos){
  printf("\n");
  print_board(getBoard(pos));
  printf(" ply:%i\n", pos->plycount);
  printf(" whitePassant: %i\n", pos->whitePassant);
  printf(" blackPassant: %i\n", pos->blackPassant);
  printf(" white_king: %i\n", pos->white_king);
  printf(" black_king: %i\n", pos->black_king);
  printf(" whiteCastleLongCheck: %i\n",getWhiteCastleLong(pos));
  printf(" whiteCastleShortCheck: %i\n",getWhiteCastleShort(pos));
  printf(" blackCastleLongCheck: %i\n",getBlackCastleLong(pos));
  printf(" blackCastleShortCheck: %i\n",getBlackCastleShort(pos));
  printf(" pieces: %i\n", pos->pieces);

}

/*-------------------------------------------------------.
 | function: is_pawn_start_pos(piece p, int src)->boolean|
 | returns: true if the pawn is on the starting position.|
 | note: help function for move_pawn.                    |
  -------------------------------------------------------*/
int is_pawn_start_pos(piece p, int src){
  int check;
  assert(on_board(src) && valid_piece_not_empty(p));
  check = src/16;
  if ((check==1)&&(is_white_piece(p))) return 1;
  if ((check==6)&&(is_black_piece(p))) return 1;
  return 0;
}

/*-------------------------------------------------------.
 | function: is_pawn_seventh(piece p, int src)->boolean  |
 | returns: true if the pawn is on the seventh rank      |
 |          for white this is rank=7, for black it is    |
 |          rank=2.                                      |
 | note: this is used when checking for legal promotion  |
 |       moves. (move_pawn)                              |
  -------------------------------------------------------*/
int is_pawn_seventh(piece p, int src){
  int check;
  assert(on_board(src) && valid_piece_not_empty(p));
  check = src/16;
  if ((check==6)&&(is_white_piece(p))) return 1;
  if ((check==1)&&(is_black_piece(p))) return 1;
  return 0;
}

/*-------------------------------------------------------.
 | function: is_pawn_fifth(piece p, int src) -> boolean  |
 | returns: true if the pawn is on the "fifth" rank,     |
 |          for white this is rank=5, for black it is    |
 |          rank=4.                                      |
 | note: this is used when checking for legal passant    |
 |       moves, the pawn that captures en passant is     |
 |       always on the "fifth rank". (move_pawn)         | 
  -------------------------------------------------------*/
int is_pawn_fifth(piece p, int src){
  int check;
  assert(on_board(src) && valid_piece_not_empty(p));
  check = src/16;
  if ((check==4)&&(is_white_piece(p))) return 1;
  if ((check==3)&&(is_black_piece(p))) return 1;
  return 0;
}

/*-------------------------------------------------------.
 | function: move_pawn_add_proms(move_list moves,        | 
 |           char toMove, int src, int dst) -> void      |
 | returns: void                                         |
 | effects: adds all promotions for pawn @ src with      |
 |          capture, to dst                              |
 | note: help function for move_pawn.                    |
  -------------------------------------------------------*/
void move_pawn_add_proms(position pos, move_list moves, char toMove, int src, int dst,
			 piece cap){
  assert(valid_piece_not_empty(cap));
  assert(valid_piece_not_empty(make_same_color(toMove, WQ)));
  assert(valid_piece_not_empty(make_same_color(toMove, WR)));
  assert(valid_piece_not_empty(make_same_color(toMove, WB)));
  assert(valid_piece_not_empty(make_same_color(toMove, WN)));
  push(pos->move_stack,new_move_all(src,dst,cap,make_same_color(toMove, WQ),0,0), score_prom_cap(WQ, cap));
  push(pos->move_stack,new_move_all(src,dst,cap,make_same_color(toMove, WR),0,0), score_prom_cap(WR, cap));
  push(pos->move_stack,new_move_all(src,dst,cap,make_same_color(toMove, WB),0,0), score_prom_cap(WB, cap));
  push(pos->move_stack,new_move_all(src,dst,cap,make_same_color(toMove, WN),0,0), score_prom_cap(WN, cap));
}

/*-------------------------------------------------------.
 | function: move_pawn_add_proms_nocap(move_list moves,  | 
 |           char toMove, int src, int dst) -> void      |
 | returns: void                                         |
 | effects: adds all promotions for pawn @ src without   |
 |          capture, to dst                              |
 | note: help function for move_pawn.                    |
  -------------------------------------------------------*/
void move_pawn_add_proms_nocap(position pos, move_list moves, char toMove, int src, int dst){
  assert(make_same_color(toMove, WQ));
  assert(make_same_color(toMove, WR));
  assert(make_same_color(toMove, WB));
  assert(make_same_color(toMove, WN));
  push(pos->move_stack,new_move_with_prom(src,dst,make_same_color(toMove, WQ)),score_prom(WQ));
  push(pos->move_stack,new_move_with_prom(src,dst,make_same_color(toMove, WR)),score_prom(WR));
  push(pos->move_stack,new_move_with_prom(src,dst,make_same_color(toMove, WB)),score_prom(WB));
  push(pos->move_stack,new_move_with_prom(src,dst,make_same_color(toMove, WN)),score_prom(WN));
}

/*-------------------------------------------------------.
 | function: move_pawn(move_list moves, position pos,    |
 |                     int src) -> void                  |
 | returns: void                                         |
 | effects: adds pawn moves for pawn @ src to moves      |
  -------------------------------------------------------*/
void move_pawn(move_list moves, position pos, int src){
  piece toMove;
  int i;
  board brd;

  assert(on_board(src));
  brd = getBoard(pos);
  toMove=get_piece(brd, src); /* the piece we are moving */
  assert(!(is_white_piece(toMove) && getToMove(pos)==BLACK));
  assert(!(is_black_piece(toMove) && getToMove(pos)==WHITE));

  if (is_white_piece(toMove)) i=0; /* select dst from the array. */
  else i=4;
  /* position of the pawn is either < 7th rank (white), > 2nd rank (black) */
  if (!(is_pawn_seventh(toMove, src))){
    /* start pos, 2 squares jump */
    if (is_pawn_start_pos(toMove, src) &&
	on_board(src+pawn_dsts[i]) &&
	on_board(src+pawn_dsts[i+1]) &&
	is_empty(get_piece(brd, src+pawn_dsts[i])) &&
	is_empty(get_piece(brd, src+pawn_dsts[i+1]))){
      assert(debug_legal_move(new_move(src, src+pawn_dsts[i+1])));
      push(pos->move_stack,new_move(src, src+pawn_dsts[i+1]),
	   hist_score2(pos->hist_heuristic,src,src+pawn_dsts[i+1]));
    }
    /* move up */
    if (on_board(src+pawn_dsts[i]) && 
	is_empty(get_piece(brd, src+pawn_dsts[i]))){
      assert(debug_legal_move(new_move(src, src+pawn_dsts[i])));
      push(pos->move_stack,new_move(src, src+pawn_dsts[i]),
	   hist_score2(pos->hist_heuristic,src,src+pawn_dsts[i]));
    }
    /* capture */
    if (on_board(src+pawn_dsts[i+2]) &&
	is_different(get_piece(brd, src+pawn_dsts[i+2]), toMove)){
      piece cap;
      assert(debug_legal_move(new_move_with_cap(src, src+pawn_dsts[i+2], 
						get_piece(brd, src+pawn_dsts[i+2]))));
      cap = get_piece(brd, src+pawn_dsts[i+2]);
      push(pos->move_stack,new_move_with_cap(src, 
					     src+pawn_dsts[i+2],cap), score_cap(toMove,cap ));
    }
    if (on_board(src+pawn_dsts[i+3])&&
	is_different(get_piece(brd, src+pawn_dsts[i+3]), toMove)){
      piece cap;
      assert(debug_legal_move(new_move_with_cap(src, src+pawn_dsts[i+3], 
						get_piece(brd,src+pawn_dsts[i+3]))));
      cap = get_piece(brd, src+pawn_dsts[i+3]);
      push(pos->move_stack,new_move_with_cap(src, src+pawn_dsts[i+3],cap), score_cap(toMove, cap));  
    }
    /* en passant */
    if ((getWhitePassant(pos) && is_white_piece(toMove) &&  is_pawn_fifth(toMove,src))) 
      { 
	char leftsrc, rightsrc;
	leftsrc=src-1;
	rightsrc=src+1;
	if (leftsrc==(pos->passant_piece)){
	  assert(debug_legal_move(new_move_with_passant(src,src+15,BP)));
	  push(pos->move_stack,new_move_with_passant(src,src+15,BP),score_cap(toMove,BP));
	}
	if (rightsrc==(pos->passant_piece)){
	  assert(debug_legal_move(new_move_with_passant(src,src+17,BP)));
	  push(pos->move_stack,new_move_with_passant(src,src+17,BP),score_cap(toMove,BP));
	}
      }
    
    else if ((getBlackPassant(pos) && is_black_piece(toMove) &&  is_pawn_fifth(toMove,src))) 
      { 
	char leftsrc, rightsrc;
	leftsrc=src-1;
	rightsrc=src+1;
	if (leftsrc==(pos->passant_piece)){
	  assert(debug_legal_move(new_move_with_passant(src,src-17,WP)));
	  push(pos->move_stack,new_move_with_passant(src,src-17,WP), score_cap(toMove,WP));
	}
	if (rightsrc==(pos->passant_piece)){
	  assert(debug_legal_move(new_move_with_passant(src,src-15,WP)));
	  push(pos->move_stack,new_move_with_passant(src,src-15,WP), score_cap(toMove,WP));
	}
      } 
  } 
  /* promotion */
  if (is_pawn_seventh(toMove, src)){
    /* move up */
    if (on_board(src+pawn_dsts[i]) && 
	is_empty(get_piece(brd,  src+pawn_dsts[i]))) {
      move_pawn_add_proms_nocap(pos, moves, toMove, src, src+pawn_dsts[i]);
    }
    /* promotion with capture */
    if (on_board(src+pawn_dsts[i+2])&&
	is_different(get_piece(brd, src+pawn_dsts[i+2]), toMove)){
      move_pawn_add_proms(pos,moves, toMove, src, 
			  src+pawn_dsts[i+2], 
			  get_piece(brd,src+pawn_dsts[i+2]));
    }
    if (on_board(src+pawn_dsts[i+3]) && 
	is_different(get_piece(brd, src+pawn_dsts[i+3]), toMove)){
      move_pawn_add_proms(pos, moves, toMove, src, 
			  src+pawn_dsts[i+3], 
			  get_piece(brd,src+pawn_dsts[i+3]));
    }
  }
}

/*-------------------------------------------------------.
 | function: move_knight(move_list moves, position pos,  |
 |                       int src, int dsts[]) -> void    |
 | returns: void                                         |
 | effects: adds knight moves to the list moves.         |
  -------------------------------------------------------*/
void move_knight(move_list moves, position pos, int src, int dsts[]){
  char toMove;
  int dst, i=0;
  piece p;
  board brd;
  
  brd = getBoard(pos);
  toMove=get_piece(brd, src); 
  dst = src+dsts[i];
  while(dsts[i] != 0) { 
    if (on_board(dst)) {
      p=get_piece(brd, dst);
      assert(valid_piece(p));
      if (is_empty(p)) {
	assert(debug_legal_move(new_move(src,dst)));
	push(pos->move_stack,new_move(src,dst),hist_score2(pos->hist_heuristic, src,dst));
      } 
      else if (is_different(p, toMove)){ /* CAPTURE */
	assert(debug_legal_move(new_move_with_cap(src, dst, p)));
	push(pos->move_stack,new_move_with_cap(src, dst, p), score_cap(toMove, p));
      } 
    }
    i++;
    dst = src+dsts[i];
  }
}

/*-------------------------------------------------------.
 | function:  move_pieces(move_list moves, position pos, |
 |                        int src, int dsts[])- > void   |
 | returns: void                                         |
 | effects: adds all "psuedo moves" to the list moves for|
 |          pieces: Queen, Rook and Bishop.              |
  -------------------------------------------------------*/
void move_pieces(move_list moves, position pos, int src, int dsts[]){
  char toMove;
  int dst, i=0;
  piece p;
  board brd;

  brd = getBoard(pos);
  toMove=get_piece(brd, src); 
  while(dsts[i]!=0){
    dst = src+dsts[i];
    while(on_board(dst)) {
      p=get_piece(brd, dst);
      assert(valid_piece(p));
      if (is_empty(p)) {
	assert(debug_legal_move(new_move(src,dst)));
	push(pos->move_stack,new_move(src, dst), hist_score2(pos->hist_heuristic, src,dst));
      } 
      else if (is_different(p, toMove)){ /* CAPTURE */
	assert(debug_legal_move(new_move_with_cap(src, dst, p)));
	push(pos->move_stack,new_move_with_cap(src, dst, p), score_cap(toMove, p));
	break;
      } 
      else break; /* p is same color of the piece we are moving. */
      dst += dsts[i];
    }
    i++;
  }
}

/*-------------------------------------------------------.
 | function: not_in_check(position pos, char src,        |
 |                        char toMove)->boolean          |
 | returns: true if the king toMove is not in check      |
 | note: toMove is either BK or WK, src is the src of    |
 |       the king.                                       |
  -------------------------------------------------------*/
int not_in_check(position pos, int src, int toMove){
  int dst, i;
  piece p;
  board brd;

  brd = getBoard(pos);
  for (i=0; i < 8; i++) {
    for (dst=src+queen_dsts[i]; on_board(dst);  dst+=queen_dsts[i]) {
      p=get_piece(brd, dst);
      if (is_empty(p)) continue;
      if (is_different(p, toMove))
	switch (p) {
	case WK: 
	case BK: if (dst==src+queen_dsts[i]) return 0; break;
	case WQ: 
	case BQ: return 0;
	case WR: 
	case BR: if (abs(queen_dsts[i]) == 16 || abs(queen_dsts[i]) == 1) return 0; break;
	case WB: 
	case BB: if (abs(queen_dsts[i]) == 15 || abs(queen_dsts[i]) == 17) return 0; break;
	case WP: if ((dst == src - 15) || (dst == src - 17)) return 0; break;
	case BP: if ((dst == src + 15) || (dst == src + 17)) return 0; break;
	default: break; 
	}
      break; /* piece same color as toMove */
    }  
  }
  for (i=0; knight_dsts[i]!=0; i++) {
    dst = src + knight_dsts[i];
    if (on_board(dst)) {
      p=get_piece(brd, dst);
      if (is_different(p, toMove) && ((p==WN) || (p==BN))) return 0;
    }
  }
  return 1; /* not in check */
}

/*-------------------------------------------------------.
 | function: move_king(move_list moves, position pos,    |
 |                     char src, int dsts[]) -> void     |
 | returns: void                                         |
 | effects: adds all "pseudo legal" moves of the king on |
 |          the move to the list of moves.               |
 | note: castling is a king move.                        | 
  -------------------------------------------------------*/
void move_king(move_list moves, position pos, int src, int dsts[]){
  char toMove;
  int dst, i=0;
  piece p;
  board brd;
  
  brd = getBoard(pos);

  toMove=get_piece(brd, src);
  dst = src+dsts[i];
  while(dsts[i] != 0) { 
    if (on_board(dst)) {
      p=get_piece(brd, dst);
      assert(valid_piece(p));
      if (is_empty(p)) {
	/* non-capture move */
	assert(debug_legal_move(new_move(src, dst)));
	push(pos->move_stack,new_move(src,dst), hist_score2(pos->hist_heuristic, src,dst));
      } 
      else if (is_different(p, toMove)){
	/* capture move */
	assert(debug_legal_move(new_move_with_cap(src, dst, p)));
	push(pos->move_stack,new_move_with_cap(src, dst, p), score_cap(toMove,p));
      } 
    }
    i++;
    dst = src+dsts[i];
  }
  /* Now we check if castle is possible:
     If rook is captured, you cant castle.
     if king has moved both short and long are false.
     if a1 Rook moved long is false, and h1 short is false. */
  /* WHITE */
  if (is_white_piece(toMove)) { 
    if (!getWhiteCastleShort(pos) && is_empty(get_piece(brd, 5)) && 
	is_empty(get_piece(brd,6)) &&(get_piece(brd, 7)==WR) &&
	(not_in_check(pos, 4, WK) && not_in_check(pos, 5, WK) && 
	 not_in_check(pos, 6, WK))){
      assert(debug_legal_move(set_mov_castle(new_move(4, 6))));
      push(pos->move_stack,set_mov_castle(new_move(4, 6)),hist_score2(pos->hist_heuristic, 4,6));
    }
    if (!getWhiteCastleLong(pos) && is_empty(get_piece(brd, 3)) && 
	is_empty(get_piece(brd,2)) && is_empty(get_piece(brd,1)) &&
	(get_piece(brd, 0)==WR) && (not_in_check(pos, 4, WK) && 
				    not_in_check(pos, 3, WK) && 
				    not_in_check(pos, 2, WK))){
      assert(debug_legal_move(set_mov_castle(new_move(src, 2))));
      push(pos->move_stack,set_mov_castle(new_move(src, 2)),hist_score2(pos->hist_heuristic,src,2));
    }
  }
  /* BLACK */
  else if (is_black_piece(toMove)) {
    if (!getBlackCastleShort(pos) && is_empty(get_piece(brd, 117)) && 
	is_empty(get_piece(brd,118)) && 
	(get_piece(brd, 119)==BR) &&
	(not_in_check(pos, 116, BK) && not_in_check(pos, 117, BK) && 
	 not_in_check(pos, 118, BK))){
      assert(debug_legal_move(set_mov_castle(new_move(src, 118))));
      push(pos->move_stack,set_mov_castle(new_move(src, 118)),hist_score2(pos->hist_heuristic,src,118));
    }
    if (!getBlackCastleLong(pos) && is_empty(get_piece(brd, 115)) && 
	is_empty(get_piece(brd,114)) && is_empty(get_piece(brd, 113)) &&
	(get_piece(brd, 112)==BR) &&
	not_in_check(pos, 116, BK) && not_in_check(pos, 115, BK) && 
	not_in_check(pos, 114, BK)){
      assert(debug_legal_move(set_mov_castle(new_move(src, 114))));
      push(pos->move_stack,set_mov_castle(new_move(src, 114)),hist_score2(pos->hist_heuristic,src,114));
    }
  }
}

/*-------------------------------------------------------.
 | function:  move_generator(position pos) -> move_list  |
 | returns:    all "psuedo moves" in position pos, that  | 
 | is, illegal moves where the king can be captured on   |
 | the next move are returned also, this must be checked | 
 | by the search algorithm, if there exists a move that  |
 | captures the king the last move was illegal.          |
 | note: if there are only illegal moves in a position   |
 |       that means the position is a draw or checkmate  |
  -------------------------------------------------------*/
move_list move_generator(position pos){
  move_list moves;
  piece p;
  int toMove, i;
  board brd;

  moves = new_moves(pos->move_stack);
  toMove = getToMove(pos);
  brd = getBoard(pos);

  if (toMove==WHITE) {
    for (i=0; i < 120; ) {
      assert(on_board(i));
      p=get_piece(brd, i);
      switch (p){
      case WK: move_king(moves,pos, i, king_dsts); break;
      case WQ: move_pieces(moves,pos, i, queen_dsts); break;
      case WR: move_pieces(moves,pos, i, rook_dsts); break;
      case WB: move_pieces(moves,pos, i, bishop_dsts);break;
      case WN: move_knight(moves,pos, i, knight_dsts);break;
      case WP: move_pawn(moves,pos, i); break;
      default: break;
      }
      if (!on_board(i+1)) i=i+9;
      else i++;
    }  
  } 
  else {
    for (i=0; i < 120; ) {
      assert(on_board(i));
      p=get_piece(brd, i);
      switch (p){
      case BK: move_king(moves,pos, i, king_dsts); break;
      case BQ: move_pieces(moves,pos, i, queen_dsts); break;
      case BR: move_pieces(moves,pos, i, rook_dsts); break;
      case BB: move_pieces(moves,pos, i, bishop_dsts); break;
      case BN: move_knight(moves,pos, i, knight_dsts);break;
      case BP: move_pawn(moves,pos, i); break;
      default: break;
      }
      if (!on_board(i+1)) i=i+9;
      else i++;
    }    
  }
  return moves;
}

/*-------------------------------------------------------.
 | function: find_piece(position pos, piece p) -> int    |
 | returns: the src for piece p in position p            |
  -------------------------------------------------------*/
int find_piece(position pos, piece p){
  int i;
  for (i=0; i < 120; ) {
    if (p==get_piece(getBoard(pos), i)) return i;
    if (!on_board(i+1)) i=i+9;
    else i++;
  }
  assert(0); /* this can't be happening. */
  return 0;
}   

/*-------------------------------------------------------.
 | function: draw_or_checkmate(position pos) -> int      |
 | returns: 0 if draw, 1 if checkmate                    |
 | note: REQUIERS that the side (toMove) has no legal    |
 |       moves in this position.                         |
 |       This one is called by the search                |
  -------------------------------------------------------*/
int draw_or_checkmate(position pos){
  piece toMove;
  int src;
  if (getToMove(pos)==WHITE) toMove=WK;
  else toMove=BK;
  src = find_piece(pos, toMove);
  if (not_in_check(pos,src,toMove)) return 0;
  return 1;
}

/*-------------------------------------------------------.
 | function: in_check(position pos) -> int               |
 | returns: 0 - if not in check                          |
 |          1 - if in chek                               |
 | note: a more general function than not_in_check.      |
  -------------------------------------------------------*/
int in_check(position pos){
  piece toMove;
  int src;
  if (getToMove(pos)==WHITE) {
    toMove=WK;
    src = pos->white_king;
  }
  else { 
    toMove=BK;
    src = pos->black_king;
  }
  if (not_in_check(pos,src,toMove)) return 0;
  return 1;
}
/*-------------------------------------------------------.
 | function: in_check_illegal(position pos) -> int       |
 | returns: 0 - if not in check                          |
 |          1 - if in check                              |
 | note: illegal check meaning the side on move can      |
 |       capture the king, so the position is illegal!.  |
  -------------------------------------------------------*/
int in_check_illegal(position pos){
  piece toMove;
  int src;
  if (getToMove(pos)==WHITE) { 
    toMove=BK;
    src = pos->black_king;
  }
  else {
    toMove= WK;
    src = pos->white_king;
  }
  if (not_in_check(pos,src,toMove)) return 0;
  return 1;
}

/*-------------------------------------------------------.
 | function: isGameOver(position pos) -> int             |
 | returns: 0 - if not Draw or Checkmate                 |
 |          1 - if Draw or Checkmate                     |
  -------------------------------------------------------*/
int isGameOver(position pos){
  move_list moves;
  move mov;

  moves = move_generator(pos);
  mov = pop(pos->move_stack, moves);
  while (mov!=NULL_MOVE){
    do_move(pos, mov);
    if (legal_opponent_check(pos)) { /* Found a legal move. */
      undo_move(pos, mov);
      free_move_list(pos->move_stack,moves);
      return 0;
    }
    undo_move(pos, mov);
    mov = pop(pos->move_stack, moves);
  }
  free_move_list(pos->move_stack, moves);
  /* no legal moves found for the player on the Move. */
  return 1;  
}

