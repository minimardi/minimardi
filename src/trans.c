#include "trans.h"

/* The Transposition Table */
static trans_table_entry_ trans_table[MAX_TRANS_TABLE];

int trans_depth(trans_table_entry tt_entry){
  return tt_entry->depth;
}
move trans_best_move(trans_table_entry tt_entry){
  return tt_entry->best_move;
}

int trans_value(trans_table_entry tt_entry){
  return tt_entry->value;
}
int trans_flag(trans_table_entry tt_entry){
  return tt_entry->value_flag;
}

int trans_table_hash_NOT_USED(U64 key, int i){
  return ((key+i) % MAX_TRANS_TABLE);
}

/*-------------------------------------------------------.
 | function: trans_table_insert(position pos, int depth, | 
 |           move bestmove, int value, int value_flag)   |
 | returns: 1 if succesful, 0 if the table is full.      |
 | effetcs: add's pos to the transposition table, if pos |
 | is already stored its updated if depth>stored depth.  |
  -------------------------------------------------------*/
int trans_table_insert(position pos, int depth, move best_move, int value, int value_flag){
  U64 key;
  trans_table_entry_ new_entry;
  key = getZobristKey(pos);
  
  if (depth<trans_table[key % MAX_TRANS_TABLE].depth)
    return 0;
  new_entry.key=key;
  new_entry.depth=depth;
  new_entry.best_move=best_move;
  new_entry.value=value;
  new_entry.value_flag=value_flag;

  trans_table[key % MAX_TRANS_TABLE]=new_entry;
  return 1; 
}

trans_table_entry trans_table_search(position pos){
  U64 key;
  
  key = getZobristKey(pos);
  if (trans_table[(key % MAX_TRANS_TABLE)].key==key) 
    return &trans_table[(key % MAX_TRANS_TABLE)];
  else return NULL;
}


