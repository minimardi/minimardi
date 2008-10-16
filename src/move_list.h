#ifndef MOVE_LIST
#define MOVE_LIST
#include "move.h"
#define MAX_MOVE_STACK 2024
#define MAX_HIST_STACK 512 /* max plies for a single game */

/* 
   struct: move_score 
   note: the move stack elements are of this type, no
   need to use this type outside this module, just sort
   the moves and the best move is returned by a pop.
*/
typedef struct move_score {
  unsigned int move; /* the move */
  int score; /* how good this move is */
}move_score;

/* 
   move_list is the old stack pointer, when a new list is created
   the current stack pointer is returned.
*/
typedef int move_list;

/*
  move_list_stack: A stack for the moves while searching, and for past move.
 */
typedef struct move_list_stack_ *move_list_stack;
typedef struct move_list_stack_ {
  move_score move_stack[MAX_MOVE_STACK];
  move_score hist_stack[MAX_HIST_STACK];
  int move_sp;
  int hist_sp;
} move_list_stack_;

/* FUNCTIONS */
move_list_stack new_move_list_stack();
move_list new_moves(move_list_stack mv_lst);
move_list new_hist_moves(move_list_stack mv_lst);
void push(move_list_stack mv_lst,move mov, int score);
move pop(move_list_stack mv_lst,move_list old_sp);
move look(move_list_stack mv_lst,move_list old_sp, int look_at);
move look_at_hist(move_list_stack mv_lst,move_list old_sp, int look_at);
void init_move_list(move_list_stack mv_lst);
void push_hist(move_list_stack mv_lst,move mov);
move pop_hist(move_list_stack mv_lst,move_list old_sp);
move look_hist(move_list_stack mv_lst,move_list old_sp);
void free_move_list(move_list_stack mv_lst,move_list old_sp);
void sort_move_scores(move_list_stack mv_lst,move_list moves);
move look_stack(move_list_stack mv_lst,move_list old_sp, int look_at);
#endif
