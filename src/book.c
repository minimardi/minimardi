#include "book.h"
/*-------------------------------------------------------.
 | file: book.c                                          |
 | contains: functions for getting book moves.           |
  -------------------------------------------------------*/

/*-------------------------------------------------------.
 | function: cp_string(char *s1, char *s2, int start)    |
 | returns: copies string s1 to s2+start.                |
  -------------------------------------------------------*/
int cp_string(char *s1, char *s2, int start){
  int i = 0;
  while(s1[i] != '\n'){
    s2[start+i]=s1[i];
    i++;
  }
  return i;
}

/*-------------------------------------------------------.
 | function: get_opening(char *hist, char *opening,      |
 |                       position pos) -> move           |
 | returns: the book move given the two strings.         |
  -------------------------------------------------------*/
move get_opening(char *hist, char *opening, position pos){
  int i=0, j=0;
  char tmp[5];
  move mov;

  while(hist[i] != '\n') {
    if (hist[i] != opening[i])
      return NULL_MOVE;
    i++;
  }
  if (opening[i] != '\n'){ /* found a book move */
    tmp[j] = opening[i];
    tmp[j+1] = opening[i+1];
    tmp[j+2] = opening[i+2];
    tmp[j+3] = opening[i+3];
    mov = alg_to_move(tmp, getToMove(pos));
    mov = legal_opponent_move(pos, mov);
    return mov;
  }
  return NULL_MOVE; /* No book move found  */
}

/*-------------------------------------------------------.
 | function: opening_move(position pos) -> move          |
 | returns: the book move (randomized) if any.           |
 | note: NULL_MOVE is returned if no move is in the file |
  -------------------------------------------------------*/
move opening_move(position pos){
  int i=0, j=0, lines=0, x=0;
  char alg_mov[10];
  char hist[2024]; 
  char opening[2048];
  move_list history;
  move mov=NULL_MOVE, oldmov;
  move candidates[1024];
  FILE *thebook;

  thebook = getBook(pos);
  if (!thebook) return NULL_MOVE;
  srand(time(NULL));
  history=getPosMoves(pos);
  mov=look_at_hist(get_move_stack(pos),history, i);
  while(mov!=NULL_MOVE){
    move_to_alg(mov, alg_mov);
    j=j+cp_string(alg_mov, hist, j);
    i++;
    mov=look_at_hist(get_move_stack(pos),history, i);   
  }
  hist[j]='\n';

  fseek(thebook, 0, SEEK_SET);
  mov=NULL_MOVE;
  while(mov==NULL_MOVE && fgets(opening, 256, thebook)) 
    mov = get_opening(hist, opening, pos);
  if (mov!=NULL_MOVE) { /* found a book move */
    lines=0;
    candidates[0]=mov;
    oldmov = mov;
    while(fgets(opening, 256, thebook) && 
	  (mov = get_opening(hist, opening, pos)) != NULL_MOVE) {
      if (mov != oldmov) {
	lines++;
	candidates[lines]=mov;
	oldmov = mov;
      }
    }
    if (lines==0) x=0;
    else x = rand() % (lines+1);
    if (x<0 || x>lines || lines>1023) { printf("Book: Error\n"); exit(1); }
    mov = candidates[x];
  }
  if (mov == NULL_MOVE) setOutOfBook(pos);
  return mov;
}

