#ifndef SEARCH
#define SEARCH
//#define SEARCH_NO_HASH 
//#define SEARCH_MTDF /* Use MTDF instead of "common" alphabeta */
//#define SEARCH_FAIL_SOFT /* MTDF must have this defined */
//#define SEARCH_PVS /* Use Principal Variation Search */
#define SEARCH_NULL_MOVE
#define SEARCH_NULL_MOVE_R 2
#define SEARCH_ENDGAME 5 /* pieces (including Kings) defined as endgame (no nullmove pruning) */
#include "piece.h"
#include "position.h"
#include "eval.h"
#include "book.h"
#include "trans.h"
#include "move.h"
#include <time.h>
#include "utils.h"
#include <assert.h>
#define MAX_PLY 40 /* the search never exceeds this value */
#define MAX_PV MAX_PLY/* the max pv */
#define MAX_QDEPTH 32 /* max depth in the qsearch */  
#define OUT_OF_TIME 512 /* nodes until check of out_of_time */
#define INFINITY 99999
#define INFINITY_PLUS_ONE 100000
/*
  struct to hold the principal variation of
  each node in the search.
 */
typedef struct pv {
  int moves; /* length of pv */
  move pv[MAX_PV]; /* the pv */
} pv;

/*
  struct for data in the search, like nodes
  searched, boolean variables for time management,
  when to check for out of time.
*/
typedef struct search_data_ *search_data;
typedef struct search_data_ {
  int nodes; /* nodes searched */
  int q_nodes; /* quiet nodes searched */
  int use_time; /* boolean:check for out of time/don't check for out time*/
  int time; /* it's time for checkup, every 1024 nodes. */
} search_data_;
/* FUNCTIONS */
move bestMove(position pos, int depth, int time_ply);
int perft(position pos, int depth);
#endif
