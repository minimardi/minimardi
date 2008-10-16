#ifndef TRANS
#define TRANS
#include "position.h"

#define TT_UNKNOWN 0
#define TT_EXACT 1
#define TT_ALPHA 2
#define TT_BETA  3
#define MAX_TRANS_TABLE (1299721)

#define SEARCH_NO_HASH 1
//#define MAX_TRANS_TABLE (104729)

/* Transposition Table Entry */
typedef struct trans_table_entry_ *trans_table_entry;
typedef struct trans_table_entry_ {
  U64 key; /* the position that is stored */
  int depth; /* the searched depth */
  move best_move; /* the best move found at depth */
  int value; /* value of this position */
  int value_flag; /* is this value an EXACT, ALPHA or BETA value */
} trans_table_entry_;

/* FUNCTIONS */
int trans_table_insert(position pos, int depth, move best_move, int value, int value_flag);
trans_table_entry trans_table_search(position pos);
int trans_depth(trans_table_entry tt_entry);
move trans_best_move(trans_table_entry tt_entry);
int trans_value(trans_table_entry tt_entry);
int trans_flag(trans_table_entry tt_entry);

void push_key(U64 key);
U64 pop_key();
#endif
