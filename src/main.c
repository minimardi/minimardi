#include "main.h"
static char input[MAX_INPUT];
static move opp_move;
static position pos=NULL;
static int GameOver=0;
static int force_mode=0;
static int printboard=0;

static void cmd_quit(){
  if (pos) free_position(pos);
  exit(0);
}

static void cmd_move(){
  char temp[MAX_INPUT];
  if (!strncmp(input, "usermove", 8)){
    strcpy(temp,(input+9));
    opp_move = alg_to_move(temp, getToMove(pos));
    opp_move = legal_opponent_move(pos, opp_move);
    if (opp_move==NULL_MOVE){
      printf("Illegal move: \n");
      return;
    } else {
      do_move(pos, opp_move);
    }
  } 

  if (!strncmp(input, "result",6)){ /* the game is over for diff. reasons. */
    GameOver=1;
    if (pos) free_position(pos);
    pos = new_chess_position(); 
    return;
  }

  if (!strncmp(input, "ping", 4)){
    strcpy(temp, input);
    temp[1]='o';
    printf(temp);
    return;
  }
  
  if (!strncmp(input, "level", 5)){ /* set time limit */
    int base=0, increment=0, i=0;

    if (input[6] == '0') {/* ICS standard time control */
      base = atoi(input+8);
      i = 8;
      while (input[i] != ' ') i++;
      increment = atoi(input+i);
      set_time_control(base, increment);
    }
    else {
      set_time_control(3, 0);
    }
    return;
  }

  if (!strncmp(input, "time", 4)){ /* computer clock  */
    set_comp_time(atoi(input+5));
    return;
  }

  if (!strncmp(input, "otim", 4)){ /* opponent clock */
    set_opp_time(atoi(input+5));
    return;
  }

  if (!strncmp(input, "setboard", 8)){ /* opponent clock */
    if (pos) {
      free_position(pos);
      pos = new_chess_position_fen(input+9);
    }
    else  pos = new_chess_position_fen(input+8);
    return;
  }

  /* maybe a move */

  if (!strncmp(input, "o-o-o", 5)) {
    if (getToMove(pos)==WHITE) {
      opp_move = alg_to_move("e1c1\n", getToMove(pos));
      opp_move = legal_opponent_move(pos, opp_move);
      if (opp_move==NULL_MOVE){
	printf("Illegal move: o-o-o\n");
	return;
      } 
      else {
	do_move(pos, opp_move);
	return;
      }
    }
    else { /* BLACK */
      opp_move = alg_to_move("e8c8\n", getToMove(pos));
      opp_move = legal_opponent_move(pos, opp_move);
      if (opp_move==NULL_MOVE){
	printf("Illegal move: o-o-o\n");
	return;
      } 
      else {
	do_move(pos, opp_move);
	return;
      } 
    }
  }
 
  else  
    if (!strncmp(input, "o-o", 3)) {
      if (getToMove(pos)==WHITE) {
	opp_move = alg_to_move("e1g1\n", getToMove(pos));
	opp_move = legal_opponent_move(pos, opp_move);
	if (opp_move==NULL_MOVE){
	  printf("Illegal move: o-o\n");
	  return;
	} 
	else {
	  do_move(pos, opp_move);
	  return;
	}
      }
      else { /* BLACK */
	opp_move = alg_to_move("e8g8\n", getToMove(pos));
	opp_move = legal_opponent_move(pos, opp_move);
	if (opp_move==NULL_MOVE){
	  printf("Illegal move: o-o\n");
	  return;
	} 
	else {
	  do_move(pos, opp_move);
	  return;
	} 
      }
    }

  if (input[0] >= 97 && input[0] <= 104 && input[1] >=49 && input[1] <= 56 &&
      input[2] >= 97 && input[2] <= 104 && input[3] >=49 && input[3] <= 56 )
       
    {
    strcpy(temp,(input));
    opp_move = alg_to_move(temp, getToMove(pos));
    opp_move = legal_opponent_move(pos, opp_move);
    if (opp_move==NULL_MOVE){
      printf("Illegal move: %s\n", temp);
      return;
    } else {
      do_move(pos, opp_move);
    }

  }
  /* command not implemented => do nothing */
}

static void cmd_xboard(){
  printf("xboard mode on.\n");
}
static void cmd_new(){
  GameOver=0;
  force_mode=0;
  if (pos) free_position(pos);
  pos = new_chess_position(); 
}
static void cmd_white(){
  setSideBlack(pos);
  setToMove(pos, WHITE);
}

static void cmd_black(){
  setSideWhite(pos);
  setToMove(pos,BLACK);
}

static void cmd_protover(){
  printf("feature usermove=1 ping=1 colors=1 setboard=1 done=1 time=1\n");
}

static void cmd_go(){
  force_mode = 0;
  if (getToMove(pos)==WHITE)
    setSideWhite(pos);
  else setSideBlack(pos);
}

static void cmd_force(){
  force_mode = 1;
}

static void cmd_playother(){
  force_mode = 0;
  if (getToMove(pos)==WHITE)
    setSideBlack(pos);
  else setSideWhite(pos);
}
/*
Total # nodes, depth = 1: 20
Total # nodes, depth = 2: 420
Total # nodes, depth = 3: 9322
Total # nodes, depth = 4: 206603
Total # nodes, depth = 5: 5072212
Total # nodes, depth = 6: 124132536
 */
static void cmd_perft5(){
  int i;
  cmd_new();
  for (i=1; i <= 5; i++){
    perft(pos, i);
  printf("Total # nodes, depth = %i: %i\n", i, getNodes(pos));
  }
  free_position(pos);
}

static void cmd_perft6(){
  int i;
  cmd_new();
  for (i=1; i <= 6; i++){
    perft(pos, i);
  printf("Total # nodes, depth = %i: %i\n", i, getNodes(pos));
  }
  free_position(pos);
}
/*
Total # nodes, depth = 1: 48
Total # nodes, depth = 2: 2087
Total # nodes, depth = 3: 99949
Total # nodes, depth = 4: 4185552
 */
static void cmd_bench(){
  int depth=4, time=0, nps=0, nodes=0;
  
  pos=new_chess_position_fen("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -");
  print_position(pos);
  time=clock();
  perft(pos, depth);
  nodes = getNodes(pos);
  time = clock() / CLOCKS_PER_SEC;
  if (time>0) nps = nodes / time;
  printf("Depth:%i, Nodes:%i, Time:%i sec, Nps:%i, Value:%i ", depth, nodes, time, nps, getValue(pos)); 
  free_position(pos);
}
static void cmd_bench2(){
  int depth=5;
  move best_move;
  int time=0, nps=0, nodes=0;
  
  pos = new_chess_position();
  time=clock();
  best_move=perft(pos, depth);
  nodes = getNodes(pos);
  time = clock() / CLOCKS_PER_SEC;
  if (time>0) nps = nodes / time;
  printf("Depth:%i, Nodes:%i, Time:%i sec, Nps:%i, Value:%i ", depth, nodes, time, nps, getValue(pos)); 
  free_position(pos);
}

static void cmd_printboard(){
  printboard=1;
}

static void cmd_help() {
  printf(help_msg);
}

static cmd_table commands[] = {
  {"xboard\n", cmd_xboard},
  {"new\n", cmd_new},
  {"protover 2\n", cmd_protover},
  {"quit\n", cmd_quit},
  {"white\n", cmd_white},
  {"black\n", cmd_black},
  {"go\n", cmd_go},
  {"force\n", cmd_force}, 
  {"perft5\n", cmd_perft5},
  {"perft6\n", cmd_perft6},
  {"bench\n", cmd_bench},
  {"bench2\n", cmd_bench2},
  {"playother\n", cmd_playother},
  {"printboard\n", cmd_printboard},
  {"help\n", cmd_help},
  {"?\n", cmd_help},
  {NULL, cmd_move }  
};

static function_ptr *get_fun(char *in_cmd){
  int i;
  for (i=0; commands[i].cmd != NULL; i++)
    if (!strcmp(commands[i].cmd, in_cmd)) 
      return commands[i].fun_ptr;
  return commands[i].fun_ptr;
}

static void comp_ended(){
  /* comp move ended the game so either we won or drew */
  if (draw_or_checkmate(pos)){ /* CHECKMATE */
    if (isCompBlack(pos))
      printf("0-1 {Black mates}\n");
    else printf("1-0 {White mates}\n"); 
  } 
  else /* DRAW */
    printf("1/2-1/2\n");
  GameOver=1;
  if (pos) free_position(pos);
  pos = NULL;
}

static void opponent_ended(){
  /* opp move ended the game so either he won or drew */
  if (draw_or_checkmate(pos)){ // CHECKMATE
    if (isCompBlack(pos))
      printf("1-0 {White mates}\n");
    else printf("0-1 {Black mates}\n"); 
  } 
  else /* DRAW */
    printf("1/2-1/2\n");
  GameOver=1;
  if (pos) free_position(pos);
  pos = NULL;
}

int main() {
  move me_mov;
  function_ptr *funptr;
  
  signal(SIGINT, SIG_IGN);
  signal(SIGTERM, SIG_IGN);
  
  setbuf(stdout, NULL);
  setbuf(stdin, NULL);
  setvbuf(stdout, NULL, _IONBF, 0);
  setvbuf(stdin, NULL, _IONBF, 0);
  fflush(NULL);
  printf(intro_msg);
  pos = new_chess_position();

  while (1) {

    fgets(input, sizeof(input), stdin);
    funptr = get_fun(input);
    if (funptr) funptr();
    funptr = NULL;
       
    if (!force_mode && !GameOver && getToMove(pos)==getCompSide(pos)) {
      allocate_time();
      me_mov = bestMove(pos, MAXDEPTH, 4);
      if (me_mov==NULL_MOVE) opponent_ended(); /* Opponent move ended the Game */
      else {
	do_move(pos, me_mov);
	print_move_alg(me_mov);
	if (isGameOver(pos)) comp_ended(); /* Comp move ended the Game */
	else {
	  if (printboard) print_position(pos);
	}
      }
    }
  }
  return 1;
}



