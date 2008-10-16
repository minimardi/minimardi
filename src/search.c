#include "search.h"
/*-------------------------------------------------------.
 | file: search.c                                        |
 | contains: functions for finding good moves.           |
  -------------------------------------------------------*/
int alphabeta(position pos, int depth, int alpha, int beta, pv *root_pv,  search_data data);
int mtdf(position pos, int f, int depth, search_data data);
int quiescent_failsoft(position pos, int depth,  int alpha, int beta, search_data data);
int alphabeta_failsoft(position pos, int depth, int alpha, int beta, pv *root_pv,  search_data data);

/*-------------------------------------------------------.
 | function: quiescent(position pos, int alpha, int beta,|
 |                     search_data data) -> int          |
 | returns: the static value for position pos for side on|
 |          the move (positive=good). If pos isn't a     |
 |          quiet position it will continue the search   |
 |          until a quiet position is found.             |
 | note: quiet position means there are no good captures.|
  -------------------------------------------------------*/
int quiescent(position pos, int depth,  int alpha, int beta, search_data data){
  move_list moves;
  move mov;
  int value;
  pv not_used;

  data->nodes++;
  data->q_nodes++;
  if (depth <= 0) 
    return eval(pos);
  if (in_check(pos)) 
    return alphabeta(pos, 1, alpha, beta, &not_used, data);
  value = eval(pos);
  if (value >= beta) 
    return beta;
  if (value > alpha)
    alpha = value;
  moves = move_generator(pos);
  sort_move_scores(get_move_stack(pos),moves);
  mov = pop(get_move_stack(pos),moves);
  if (mov==NULL_MOVE || !is_capture_move(mov)) { /* NO Captures */
    free_move_list(get_move_stack(pos),moves);
    return alpha;
  }
  while (mov!=NULL_MOVE && is_capture_move(mov)){
    do_move(pos, mov); 
    if (!in_check_illegal(pos))
      value = -quiescent(pos,depth-1, -beta, -alpha, data);
    else value=-INFINITY;
    undo_move(pos, mov);
    if (value >= beta) { 
      free_move_list(get_move_stack(pos),moves);
      return beta;
    }
    if (value > alpha) alpha = value;
    mov = pop(get_move_stack(pos),moves);
  }
  free_move_list(get_move_stack(pos),moves);
  return alpha;

}

/*-------------------------------------------------------.
 | function: perft_doit(position pos, int depth,         |
 |                      search_data data) -> int         |
 | note: help_functio for perft.                         |
  -------------------------------------------------------*/
int perft_doit(position pos, int depth, search_data data){
  int best=-100, value=0;
  move_list moves;
  move mov;

  data->nodes++;
  if (depth <= 0) 
    return 0;
  moves = move_generator(pos);
  mov = pop(get_move_stack(pos),moves);
  while (mov!=NULL_MOVE){
    do_move(pos, mov); 
    if (!in_check_illegal(pos)) {
      value=-perft_doit(pos, depth-1, data);
    } else value=-100;
    undo_move(pos, mov);
    if (value > best) best=value;
    mov = pop(get_move_stack(pos),moves);
  }
  free_move_list(get_move_stack(pos),moves);
  if (best==-100 && /* NO legal moves */
      !draw_or_checkmate(pos)) {
    best=0; /* DRAW */
    data->nodes--;
  }
  return best;
}

/*-------------------------------------------------------.
 | function: perft(position pos, int depth) -> int       |
 | note: does a complete minmax search.                  |
  -------------------------------------------------------*/
int perft(position pos, int depth){
  search_data data;

  data = (search_data) malloc(sizeof(search_data_));
  data->nodes = -1;

  perft_doit(pos, depth, data);
  if (getToMove(pos)==WHITE) setValue(pos, 0, data->nodes);
  else setValue(pos, 0, data->nodes);
  return 1;
}

/*-------------------------------------------------------.
 | function: alphabeta(position pos, int depth, int alpha,| 
 |           int beta, pv *root_pv,  search_data data)    |
 | returns: the value for position pos for side on the    |
 |          move (positive=good).                         |
  -------------------------------------------------------*/
int alphabeta(position pos, int depth, int alpha, int beta, pv *root_pv,  search_data data){
  move_list moves;
  move mov, best_move=NULL_MOVE;
  int value=-INFINITY, flag=TT_ALPHA;
  U64 debug_key;
#ifdef SEARCH_PVS
  int pv_node=0;
#endif
#ifndef SEARCH_NO_HASH
  trans_table_entry tt_entry=NULL;
  int tt_val=0,tt_flag=0;
#endif
  pv node_pv; /* following "nodes" pv */
  
  data->nodes++;
  data->time++;
  node_pv.moves = 0;
  /* check for draw by 3-fold repetition */
  if (rep_search(pos) >= 1) return 0;

#ifndef SEARCH_NO_HASH
  /* Check the Transposition Table first. */
  if ((tt_entry=trans_table_search(pos)) != NULL) { /* We have searched this position before. */
   
    if ((trans_depth(tt_entry) >= depth) && ((tt_flag=trans_flag(tt_entry)) != TT_UNKNOWN)) {
      tt_val = trans_value(tt_entry);
      if (tt_flag == TT_EXACT)
	return tt_val;
      if ((tt_flag == TT_ALPHA) && (tt_val <= alpha)) 
	return alpha;
      if ((tt_flag == TT_BETA) && (tt_val >= beta)) 
	return beta;
    }
  }
#endif
 
  if (depth <= 0) {
    root_pv->moves=0;
    value = quiescent(pos, MAX_QDEPTH, alpha, beta, data);
    return value;
  }

  /* try to cut with a NullMove */
  debug_key = getZobristKey(pos);
#ifdef SEARCH_NULL_MOVE
  if (((depth>=SEARCH_NULL_MOVE_R+1) && (pieces(pos) >= SEARCH_ENDGAME )&& 
      !last_move_null(pos)) && (!in_check(pos))) {
    do_move(pos, NULL_MOVE_PRUNING);
    value = -alphabeta(pos, depth-1-SEARCH_NULL_MOVE_R, -beta, -beta+1, &node_pv, data);
    undo_move(pos, NULL_MOVE_PRUNING);
  }
#endif
  assert(debug_key==getZobristKey(pos));
  if (value >= beta) {
#ifndef SEARCH_NO_HASH
    trans_table_insert(pos, depth, NULL_MOVE, beta, TT_BETA); /* save position and move in the TT. */
#endif
    return beta;
  }

  moves = move_generator(pos);
  sort_move_scores(get_move_stack(pos),moves);
#ifndef SEARCH_NO_HASH
  if (tt_entry != NULL && trans_flag(tt_entry) != TT_UNKNOWN && 
      (trans_best_move(tt_entry)!=NULL_MOVE))
    mov=trans_best_move(tt_entry); /* try the move from the TT first. */
  else
#endif
    mov = pop(get_move_stack(pos),moves);

  while (mov!=NULL_MOVE){
    debug_key = getZobristKey(pos);
    do_move(pos, mov); 
    if (!in_check_illegal(pos)){
#ifdef SEARCH_PVS
      if (pv_node) {
	value = -alphabeta(pos, depth-1, -alpha-1, -alpha, &node_pv, data);
	if ((value>alpha) && (value <beta))
	  value = -alphabeta(pos, depth-1, -beta, -alpha, &node_pv, data);
      }
      else
#endif
      value = -alphabeta(pos, depth-1, -beta, -alpha, &node_pv, data);
    } else value=-INFINITY;
    undo_move(pos, mov);
    if (data->use_time && data->time >= OUT_OF_TIME) { /* test for out of time */
      if (out_of_time()) {
	free_move_list(get_move_stack(pos),moves);
	return INFINITY_PLUS_ONE;
      }
      data->time=0;
    }
    assert(debug_key==getZobristKey(pos));
    if (value >= beta) {
#ifndef SEARCH_NO_HASH
      trans_table_insert(pos, depth, mov, beta, TT_BETA); /* save position and move in the TT. */
#endif
      hist_inc(get_hist_heuristic(pos),mov, depth*depth); /* increase the history for this move */
      free_move_list(get_move_stack(pos),moves);
      return beta;
    }
    if (value > alpha) {
#ifdef SEARCH_PVS
      pv_node = 1;
#endif
      flag = TT_EXACT;
      alpha = value;
      best_move = mov;
      root_pv->pv[0]=best_move;
      memcpy(root_pv->pv+1, node_pv.pv, node_pv.moves * sizeof(move));
      root_pv->moves = node_pv.moves + 1;
    }
    mov = pop(get_move_stack(pos),moves);
  }

  if (alpha==-INFINITY && /* NO legal moves */
      !draw_or_checkmate(pos)) /* Draw or Checkmate? */
    alpha=0; /* DRAW */

  if (best_move != NULL_MOVE) /* if we have a best move ... */
    hist_inc(get_hist_heuristic(pos), best_move, depth*depth); /* increase the history for this move */

  /* Now save this position and best_move in the Transposition Table */
#ifndef SEARCH_NO_HASH
  trans_table_insert(pos, depth, best_move, alpha, flag); 
#endif  
  free_move_list(get_move_stack(pos),moves);
  return alpha;
}

/*-------------------------------------------------------.
 |               F A I L       S O F T                   |
 |                        MTD                            |
 | note: code for MTD, ignore if not using MTD.          |
  -------------------------------------------------------*/

int quiescent_failsoft(position pos, int depth,  int alpha, int beta, search_data data){
  move_list moves;
  move mov;
  int value, g=-INFINITY;
  pv not_used;
#ifndef SEARCH_NO_HASH
  trans_table_entry tt_entry=NULL;
  int tt_val=0,tt_flag=0;
#endif

  data->nodes++;
  data->q_nodes++;
#ifndef SEARCH_NO_HASH
  /* Check the Transposition Table first. */
  if ((tt_entry=trans_table_search(pos)) != NULL) { /* We have searched this position before. */
   
    if ((trans_depth(tt_entry) >= 0) && ((tt_flag=trans_flag(tt_entry)) != TT_UNKNOWN)) {
      tt_val = trans_value(tt_entry);
      if (tt_flag == TT_EXACT)
	return tt_val;
      if ((tt_flag == TT_ALPHA) && (tt_val <= alpha)) {
	return tt_val;
      }
      if ((tt_flag == TT_BETA) && (tt_val >= beta)) {
	return tt_val;
      }
    }
  }
#endif

  if (depth <= 0) 
    return eval(pos);
  if (in_check(pos)) 
    return alphabeta_failsoft(pos, 1, alpha, beta, &not_used, data);
  value = eval(pos);

  if (value >= beta) 
    return value; /* fail-soft */
  if (value > alpha)
    alpha = value;
  moves = move_generator(pos);
  sort_move_scores(get_move_stack(pos),moves);
  mov = pop(get_move_stack(pos),moves);
  if (value > g) g=value;
  if (mov==NULL_MOVE || !is_capture_move(mov)) { /* NO Captures */
    free_move_list(get_move_stack(pos),moves);
    return g; /* fail-soft */
  }
  if (value>g) g=value;
  while (mov!=NULL_MOVE && is_capture_move(mov)){
    do_move(pos, mov); 
    if (!in_check_illegal(pos))
      value = -quiescent_failsoft(pos,depth-1, -beta, -alpha, data);
    else value=-INFINITY;
    undo_move(pos, mov);
    if (value >= beta) { 
#ifndef SEARCH_NO_HASH
      trans_table_insert(pos, 0, mov, value, TT_BETA);
#endif 
      free_move_list(get_move_stack(pos),moves);
      return value; /* fail-soft */
    }
    if (value>g) g=value;
    if (value > alpha) alpha = value;
    mov = pop(get_move_stack(pos),moves);
  }
#ifndef SEARCH_NO_HASH
  if (g<=alpha) trans_table_insert(pos, 0, NULL_MOVE, g, TT_ALPHA); 
  if (g>alpha && g<beta) trans_table_insert(pos, 0, NULL_MOVE, g, TT_EXACT); 
#endif  
  free_move_list(get_move_stack(pos),moves);
  return g; /* fail-soft */
}


/*-----------------------------------------------------------------.
 | function: alphabeta_failsoft(position pos, int depth, int alpha,| 
 |           int beta, pv *root_pv,  search_data data)             |
 | returns: the value for position pos for side on the move.       |
 | note: fail-soft version (used if SEARCH_MTDF is defined).       |
  -----------------------------------------------------------------*/
int alphabeta_failsoft(position pos, int depth, int alpha, int beta, pv *root_pv,  search_data data){
  move_list moves;
  move mov, best_move=NULL_MOVE;
  int value=0, flag=TT_ALPHA, g=-INFINITY;
#ifndef SEARCH_NO_HASH
  trans_table_entry tt_entry=NULL;
  int tt_val=0,tt_flag=0;
#endif
  pv node_pv; /* following "nodes" pv */
  
  data->nodes++;
  data->time++;
  node_pv.moves = 0;

  /* check for draw by 3-fold repetition */
  if (rep_search(pos) >= 2) return 0;

#ifndef SEARCH_NO_HASH
  /* Check the Transposition Table first. */
  if ((tt_entry=trans_table_search(pos)) != NULL) { /* We have searched this position before. */
   
    if ((trans_depth(tt_entry) >= depth) && ((tt_flag=trans_flag(tt_entry)) != TT_UNKNOWN)) {
      tt_val = trans_value(tt_entry);
      if (tt_flag == TT_EXACT)
	return tt_val;
      if ((tt_flag == TT_ALPHA) && (tt_val <= alpha)) {
	return tt_val;
      }
      if ((tt_flag == TT_BETA) && (tt_val >= beta)) {
	return tt_val;
      }
    }
  }
#endif

  if (depth <= 0) {
    root_pv->moves=0;
    value = quiescent_failsoft(pos, MAX_QDEPTH, alpha, beta, data);
    return value;
  }
  
  moves = move_generator(pos);
  sort_move_scores(get_move_stack(pos),moves);
#ifndef SEARCH_NO_HASH
  if (tt_entry != NULL && trans_flag(tt_entry) != TT_UNKNOWN && 
      (trans_best_move(tt_entry)!=NULL_MOVE))
    mov=trans_best_move(tt_entry); /* try the move from the TT first. */
  else
#endif
    mov = pop(get_move_stack(pos),moves);

  while (mov!=NULL_MOVE){
    do_move(pos, mov); 
    if (!in_check_illegal(pos)){
      value = -alphabeta_failsoft(pos, depth-1, -beta, -alpha, &node_pv, data);
    } else value=-INFINITY;
    undo_move(pos, mov);
    if (data->use_time && data->time >= OUT_OF_TIME) { /* test for out of time */
      if (out_of_time()) {
	free_move_list(get_move_stack(pos),moves);
	return INFINITY_PLUS_ONE;
      }
      data->time=0;
    }
    if (value > g) g=value;
    if (value >= beta) {
#ifndef SEARCH_NO_HASH
      trans_table_insert(pos, depth, mov, value, TT_BETA);
#endif
      hist_inc(get_hist_heuristic(pos), mov, depth*depth); /* increase the history for this move */
      free_move_list(get_move_stack(pos),moves);
      return value; 
    }
    if (value > alpha) {
      flag = TT_EXACT;
      alpha = value;
      best_move = mov;
    }
    mov = pop(get_move_stack(pos),moves);
  }
  
  if (g==-INFINITY && /* NO legal moves */
      !draw_or_checkmate(pos)) /* Draw or Checkmate? */
    g=0; /* DRAW */

  if (best_move != NULL_MOVE) /* if we have a best move ... */
    hist_inc(get_hist_heuristic(pos), best_move, depth*depth); /* increase the history for this move */

  /* Now save this position and best_move in the Transposition Table */
#ifndef SEARCH_NO_HASH
  if (g<=alpha) trans_table_insert(pos, depth, best_move, g, TT_ALPHA); 
  if (g>alpha && g<beta) trans_table_insert(pos, depth, best_move, g, TT_EXACT); 
#endif  
  free_move_list(get_move_stack(pos),moves);
  return g;
}

/*-------------------------------------------------------.
 | function: mtdf(position pos, int f, int depth) -> int |
 | returns: the value of pos using the MTDF algorithm.   |
 |          f is the "first guess".                      |
  -------------------------------------------------------*/
int mtdf(position pos, int f, int depth, search_data data) {
  int lowerbound=-INFINITY, upperbound=INFINITY, beta, g;
  int l=0;
  pv not_used;

  g = f;
  printf("guess=%i\n", f);
  do {
    if (g==lowerbound) beta=g+1;
    else beta=g;
    g=alphabeta_failsoft(pos, depth, beta-1, beta, &not_used, data);
    printf("%i,", g);
    if (data->use_time && data->time >= OUT_OF_TIME) {  /* test for out of time */
      if (out_of_time()) {
	printf("\n");
	return INFINITY_PLUS_ONE;
      }
      data->time=0;
    }
    if (g < beta) upperbound=g;
    else lowerbound=g;
    l++;
  } while (lowerbound < upperbound);
  printf("Mtdf runs: %i\n", l);
  return g;
}


/*-------------------------------------------------------.
 | function: bestMove_(position pos, int depth) -> move  |
 | returns: the "best" move in position pos.             |
 | note: if there is a forced mate in position pos,      |
 |       the last legal move is returned, if the position|
 |       already is ended (stalmate, checkmate) 0 is     |
 |       returned.                                       |
  -------------------------------------------------------*/
move bestMove_(position pos, int depth, pv *root_pv, search_data data){
  move_list moves;
  move mov, badmove=NULL_MOVE, bestmove=NULL_MOVE;
  int value=0, alpha=-INFINITY, beta=INFINITY;
  U64 debug_key;
#ifndef SEARCH_NO_HASH
  // trans_table_entry tt_entry;
#endif
  int flag=TT_UNKNOWN;
  pv node_pv;
  
  node_pv.moves = 0;
  data->nodes=0;
  data->q_nodes=0;

#ifndef SEARCH_NO_HASH
  /* check transposition table */
  // if ((tt_entry=trans_table_search(pos)) != NULL) { /* We have searched this position before. */
  //  if ((trans_depth(tt_entry) >= depth) && (trans_flag(tt_entry) == TT_EXACT) && 
  //	(trans_best_move(tt_entry))!=NULL_MOVE) {
  //    return trans_best_move(tt_entry);
  //  }
  // }
#endif
  moves = move_generator(pos);
  sort_move_scores(get_move_stack(pos),moves);
  mov = pop(get_move_stack(pos),moves);
  while (mov!=NULL_MOVE){
    debug_key = getZobristKey(pos);
    do_move(pos, mov); 
    if (!in_check_illegal(pos)){
      value = -alphabeta(pos, depth-1, -INFINITY, -alpha, &node_pv, data);
      if (value!=INFINITY_PLUS_ONE) badmove=mov;
    } else value=-INFINITY;
    undo_move(pos, mov);
    if (data->use_time && data->time >= OUT_OF_TIME) { /* test for out of time */
      if (out_of_time()) {
	free_move_list(get_move_stack(pos),moves);
	return NULL_MOVE;
      }
      data->time=0;
    }
    assert(debug_key==getZobristKey(pos));
    if (value >= beta) { /* meaning value >= INFINITY : forced mate */
      //trans_table_insert(pos, depth, mov, beta, TT_BETA);
      hist_inc(get_hist_heuristic(pos), mov, depth*depth); /* increase the history for this move*/
      free_move_list(get_move_stack(pos),moves);
      return mov;
    }
    if (value > alpha) {
      flag = TT_EXACT;
      alpha = value;
      bestmove=mov;
      root_pv->pv[0]=bestmove;
      memcpy(root_pv->pv+1, node_pv.pv, node_pv.moves * sizeof(move));
      root_pv->moves = node_pv.moves + 1;
    }
    mov = pop(get_move_stack(pos),moves);
  }

  if (bestmove!=NULL_MOVE) /* if we have a best move ... */
    hist_inc(get_hist_heuristic(pos), bestmove, depth*depth); /* increase the history for this move */
  if (bestmove!=NULL_MOVE) trans_table_insert(pos, depth, bestmove, alpha, flag);

  free_move_list(get_move_stack(pos),moves);

  if (getToMove(pos)==WHITE) setValue(pos, alpha, data->nodes);
  else setValue(pos, -alpha, data->nodes);
  if (bestmove==NULL_MOVE) return badmove; /* forced checkmate */
  return bestmove;
}

int print_pv(position pos, float value, float time, float nps, int depth, pv root_pv, search_data data){
  int i, j, fullmove;
  printf("tellothers Depth[%i] ", depth);
  if (value >= 0.0) printf("+");
  printf("%.2f ", value);
  
  j=0;
  for (i=0; i < root_pv.moves; i++) {
    if (!j) {
      if ((getPly(pos)+1) % 2 == 0) fullmove = ((getPly(pos)+1)/2);
      else fullmove = (getPly(pos)+1)/2+1;
      fullmove = fullmove + i/2;
      printf("%i", fullmove);
      if (getCompSide(pos) == BLACK && i==0)
	printf("...");
      else printf(".");
    }
    print_move(root_pv.pv[i]);
    printf(" ");
    j++;
    if (j==2) j=0;
  }
  printf("time: %.2f avg nps: %.2f nodes: %i\n", time, nps, data->nodes);
  return 1;
}

move bestMove_mtdf(position pos, int depth, pv *root_pv, search_data data){
  move_list moves;
  move mov, badmove=NULL_MOVE, bestmove=NULL_MOVE;
  int value=0, f=-INFINITY, guess=0;
  pv node_pv;
#ifndef SEARCH_NO_HASH
  trans_table_entry tt_entry=NULL;
  int tt_val=0,tt_flag=0;
#endif
  
  node_pv.moves = 0;
  data->nodes=0;
  data->q_nodes=0;

  if (getToMove(pos)==WHITE) guess=getValue(pos);
  else guess=-getValue(pos);

  moves = move_generator(pos);
  sort_move_scores(get_move_stack(pos),moves);
  mov = pop(get_move_stack(pos),moves);
  while (mov!=NULL_MOVE){
    do_move(pos, mov); 
    if (!in_check_illegal(pos)){
#ifndef SEARCH_NO_HASH
  /* Get a good guess */
  if ((tt_entry=trans_table_search(pos)) != NULL) { /* We have searched this position before. */
   
    if (((tt_flag=trans_flag(tt_entry)) != TT_UNKNOWN)) {
      tt_val = trans_value(tt_entry);
      if (tt_flag == TT_EXACT)
	guess = tt_val;
    }
  }    
#endif
      value = -mtdf(pos, guess, depth-1, data);
      if (value!=INFINITY_PLUS_ONE) badmove=mov;
    } else value=-INFINITY;
    undo_move(pos, mov);
    if (data->use_time && data->time >= OUT_OF_TIME) { /* test for out of time */
      if (out_of_time()) {
	free_move_list(get_move_stack(pos),moves);
	return NULL_MOVE;
      }
      data->time=0;
    }
    if (value >= INFINITY) { /* meaning value >= INFINITY : forced mate */
      hist_inc(get_hist_heuristic(pos), mov, depth*depth); /* increase the history for this move*/
      free_move_list(get_move_stack(pos),moves);
      return mov;
    }
    if (value > f) {
      f = value;
      bestmove=mov;
    }
    mov = pop(get_move_stack(pos),moves);
  }

  if (bestmove!=NULL_MOVE) /* if we have a best move ... */
    hist_inc(get_hist_heuristic(pos), bestmove, depth*depth); /* increase the history for this move */
  free_move_list(get_move_stack(pos),moves);

  if (getToMove(pos)==WHITE) setValue(pos, f, data->nodes);
  else setValue(pos, -f, data->nodes);

  if (bestmove==NULL_MOVE) return badmove; /* forced checkmate */
  return bestmove;
}
/*-------------------------------------------------------.
 | function:  bestMove(position pos, int max_depth,      |
 |                     int time_ply) -> move             |
 | returns: the best move in pos, searching to max_depth |
 |          if there is enough time, will always search  |
 |          at least to time_ply.                        |
  -------------------------------------------------------*/
move bestMove(position pos, int max_depth, int time_ply){
  int depth, ms, seconds, timecenti, i;
  move best_move=NULL_MOVE, book, mov=NULL_MOVE;
  float time=0, nps=0, value=0;
  search_data data;
  pv root_pv;
  struct timeb before, after;

  memset(root_pv.pv, 0, sizeof(move) * MAX_PV);
  data = (search_data) malloc(sizeof(search_data_));
  data->nodes=0;
  data->q_nodes=0;
  data->use_time=0;
  data->time=0;

  /* check if pos still is book */
  if (!getOutOfBook(pos)){
    if ((book=opening_move(pos))!=NULL_MOVE) 
      return book;
  }

  /* Iterative Deepening */
  initHistory(get_hist_heuristic(pos));

  for (depth=1; depth<=max_depth; depth++) {
    root_pv.moves=0;
    ftime(&before);
    if (depth<=time_ply)  {
      data->use_time=0; /* don't use out of time */
#ifndef SEARCH_MTDF
      best_move = bestMove_(pos, depth, &root_pv, data);
#endif
#ifdef SEARCH_MTDF
      best_move =  bestMove_mtdf(pos, depth, &root_pv, data);
#endif
      data->use_time=1;
    }
    else {
#ifndef SEARCH_MTDF
      best_move = bestMove_(pos, depth, &root_pv, data);
#endif
#ifdef SEARCH_MTDF
      best_move =  bestMove_mtdf(pos, depth, &root_pv, data);
#endif
    }
    if (depth>time_ply && out_of_time() && depth > 1) return mov;
    mov = best_move;

    ftime(&after);
    seconds = (int) difftime(after.time, before.time);
    ms = after.millitm-before.millitm;
    time = seconds + ms/1000.0;
    timecenti = time*100;
    value = getValue(pos) / 100.0;
    if (time>0) nps = data->nodes / time;
    printf("%i %i %i %i ", depth, getValue(pos), timecenti, data->nodes);
    for (i=0; i < root_pv.moves; i++) {
      print_move(root_pv.pv[i]);
      printf(" ");
    }
    printf("\n");
  }
  return best_move;
}
