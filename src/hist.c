#include "hist.h"
/*-------------------------------------------------------.
 | file: hist.c                                          |
 | contains: functions for the History Heuristic.        |
  -------------------------------------------------------*/

/*-------------------------------------------------------.
 | function: new_history_heuristic() -> history_heuristic|
 | returns: a history heurstic                           |
  -------------------------------------------------------*/
history_heuristic new_history_heuristic(){
  history_heuristic hist;
  hist = (history_heuristic) malloc(sizeof(history_heuristic_));
  return hist;
}

/*-------------------------------------------------------.
 | function: free_history_heuristic() -> void            |
  -------------------------------------------------------*/
void free_history_heuristic(history_heuristic hist){
  free(hist);
}

/*-------------------------------------------------------.
 | function: hist_inc(hist, move mov, int score) -> void |
 | effects: increases the history score for move mov.    |
  -------------------------------------------------------*/
void hist_inc(history_heuristic hist, move mov, int score){
  hist->history[get_src(mov)][get_dst(mov)] += score;
}

/*-------------------------------------------------------.
 | function: hist_score(hist, move mov) -> int           |
 | returns: the score for move mov.                      |
  -------------------------------------------------------*/
int hist_score(history_heuristic hist, move mov){
  return hist->history[get_src(mov)][get_dst(mov)];
}

/*-------------------------------------------------------.
 | function: hist_score2(hist, int src, int dst) -> int  |
 | returns: the score for a move with given src and dst. |
  -------------------------------------------------------*/
int hist_score2(history_heuristic hist, int src, int dst){
  return hist->history[src][dst];
}

/*-------------------------------------------------------.
 | function: clearHistory(hist) -> void                  |
 | effects: sets scores for all moves to zero.           |
  -------------------------------------------------------*/
void clearHistory(history_heuristic hist){
  memset(hist->history,0,sizeof(hist->history));
}

/*-------------------------------------------------------.
 | function: initHistory(hist) -> void                   |
 | effects: decreases scores for all moves.              |
 | note: this fun should be called sometimes, else the   |
 |       scores will get really big!                     |
  -------------------------------------------------------*/
void initHistory(history_heuristic hist){
  int i=0,j=0;
  for (i=0; i<128;i++)
    for (j=0; j<128;j++)
      hist->history[i][j] = hist->history[i][j] >> 8;
}
