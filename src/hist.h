#ifndef HIST
#define HIST
#include "move.h"
#include "string.h"
#include "board.h"

typedef struct history_heuristic_ *history_heuristic;
typedef struct history_heuristic_ {
  int history[BOARD_SIZE][BOARD_SIZE];
} history_heuristic_;

history_heuristic new_history_heuristic();
void free_history_heuristic(history_heuristic hist);
void hist_inc(history_heuristic hist, move mov, int score);
int hist_score(history_heuristic hist, move mov);
int hist_score2(history_heuristic hist, int src, int dst);
void clearHistory(history_heuristic hist);
void initHistory(history_heuristic hist);
#endif
