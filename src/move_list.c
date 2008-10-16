#include "move_list.h"
/*-------------------------------------------------------.
 | file: move_list.c                                     |
 | contains: the stack for the moves.                    |
  -------------------------------------------------------*/
move_list_stack new_move_list_stack(){
  move_list_stack mv_lst;
  mv_lst = (move_list_stack) malloc(sizeof(move_list_stack_));
  return mv_lst;
}

void init_move_list(move_list_stack mv_lst){
  mv_lst->move_sp = 0;
  mv_lst->hist_sp = 0;
}
/*-------------------------------------------------------.
 | function: new_moves() -> move_list                    |
 | returns: a move_list, it's only the old stack pointer,|
 |          needed for knowing where the list starts.    |
  -------------------------------------------------------*/
move_list new_moves(move_list_stack mv_lst){
  return mv_lst->move_sp;
}

/*-------------------------------------------------------.
 | function: new_hist_moves() -> move_list               |
 | returns: a new history move_list.                     |
  -------------------------------------------------------*/
move_list new_hist_moves(move_list_stack mv_lst){
  return mv_lst->hist_sp;
}

/*-------------------------------------------------------.
 | function: push(move mov) -> void                      |
 | effects: pushes the move mov                          |
  -------------------------------------------------------*/
void push(move_list_stack mv_lst, move mov, int score){
  if (mv_lst->move_sp>MAX_MOVE_STACK-1) {
    printf("You have to increase the MOVE_STACK\n");
    exit(1);
  }
  mv_lst->move_stack[mv_lst->move_sp].move=mov;
  mv_lst->move_stack[mv_lst->move_sp].score=score;
  mv_lst->move_sp++;
}

/*-------------------------------------------------------.
 | function: pop(move_list old_sp) -> move               |
 | returns: pops the move on the stack.                  |
 | note: returns the NULL_MOVE if there are no more      |
 |       moves left.                                     |
  -------------------------------------------------------*/
move pop(move_list_stack mv_lst, move_list old_sp){
  if (old_sp < mv_lst->move_sp){
    mv_lst->move_sp--;
    return mv_lst->move_stack[mv_lst->move_sp].move;
  } else return NULL_MOVE;
} 

/*-------------------------------------------------------.
 | function: push_hist(move mov) -> void                 |
 | effects: pushes the move mov into the history stack.  |
  -------------------------------------------------------*/
void push_hist(move_list_stack mv_lst, move mov){
  mv_lst->hist_stack[mv_lst->hist_sp].move=mov;
  mv_lst->hist_sp++;
}

/*-------------------------------------------------------.
 | function: pop_hist(move_list old_sp) -> move          |
 | returns: pops the move on the history stack.          |
 | note: returns the NULL_MOVE if there are no more      |
 |       moves left.                                     |
  -------------------------------------------------------*/
move pop_hist(move_list_stack mv_lst, move_list old_sp){
  if (old_sp < mv_lst->hist_sp){
    mv_lst->hist_sp--;
    return mv_lst->hist_stack[mv_lst->hist_sp].move;
  } else return NULL_MOVE;
} 

/*-------------------------------------------------------.
 | function: look_hist(move_list old_sp) -> move         |
 | returns: the move next to top on the stack.           |
 | note: it doesnt pop the move, its still there.        |
  -------------------------------------------------------*/
move look_hist(move_list_stack mv_lst, move_list old_sp){
  if (old_sp < mv_lst->hist_sp)
    return mv_lst->hist_stack[mv_lst->hist_sp-1].move;
  else return NULL_MOVE;  
}

/*-------------------------------------------------------.
 | function: look(move_list old_sp, int look_at)->void   |
 | returns: the look_at:th move on the stack.            |
 | note: it doesnt pop the move, its still there.        |
  -------------------------------------------------------*/
move look(move_list_stack mv_lst, move_list old_sp, int look_at){
  int i;
  i = old_sp + look_at;
  if (i < mv_lst->move_sp)
    return mv_lst->move_stack[i].move;
  else return NULL_MOVE;
}

move look_stack(move_list_stack mv_lst, move_list old_sp, int look_at){
  int i;
  i = mv_lst->move_sp-look_at;
  if (i < old_sp) return NULL_MOVE;
  return mv_lst->move_stack[i].move;
}
/*-------------------------------------------------------.
 | function: look_hist(move_list old_sp, int look_at)->void |
 | returns: the look_at:th move on the stack.            |
 | note: it doesnt pop the move, its still there.        |
  -------------------------------------------------------*/
move look_at_hist(move_list_stack mv_lst, move_list old_sp, int look_at){
  int i;
  i = old_sp + look_at;
  if (i < mv_lst->hist_sp)
    return mv_lst->hist_stack[i].move;
  else return NULL_MOVE;
}

/*-------------------------------------------------------.
 | function: free_move_list(move_list old_sp) -> void    |
 | effects: free's all moves.                            |
 | note: this is much faster than calling pop many times.|
  -------------------------------------------------------*/
void free_move_list(move_list_stack mv_lst, move_list old_sp){
  mv_lst->move_sp = old_sp;
}

/*-------------------------------------------------------.
 | function: sort_move_scores(move_list moves) -> void   |
 | effects: sorts the list moves.                        |
  -------------------------------------------------------*/
void sort_move_scores(move_list_stack mv_lst, move_list moves){
  int i, j;
  move_score mov;
  
  if (moves >= mv_lst->move_sp) return;
  if (moves == mv_lst->move_sp-1) return; /* only 1 move. */
    for (j=moves+1; j < mv_lst->move_sp; j++){
      mov = mv_lst->move_stack[j];
      i = j-1;
      while (i>moves-1 &&mv_lst->move_stack[i].score > mov.score){
	if (i>MAX_MOVE_STACK-1) {
	  printf("You have to increase the MOVE_STACK to %i\n", i);
	  exit(0);
	}
	mv_lst->move_stack[i+1] = mv_lst->move_stack[i];
	i--;
      }
     mv_lst->move_stack[i+1]=mov;
    }
}
