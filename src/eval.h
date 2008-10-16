#ifndef EVAL
#define EVAL
#include "piece.h"
#include "position.h"
#include "board.h"
#define DOUBLE_PAWN 40
#define CASTLED 30
#define ROOK_OPEN_FILE 50
#define ROOK_SEMI_OPEN 30
#define ROOK_SEVENTH 40
#define TWO_BISHOP 50
#define TRADE 10
int eval(position pos);
#endif
