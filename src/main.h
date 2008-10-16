#ifndef MAIN
#define MAIN
#define MAXDEPTH 20
#define MAX_INPUT 256
#include "position.h"
#include "search.h"
#include "book.h"
#include <string.h>
#include <signal.h>
#include <stdio.h>

static char intro_msg[] = 
".--------------------------------------------.\n"
"| miniMardi 1.3 - Messy Chess Program.       |\n"    
"| Copyright (C) 2003 Juan Pablo Fernandez.   |\n"
"| jufe1771@student.uu.se                     |\n"
" -------------------------------------------- \n";

static char help_msg[] =
"xboard           - xboard mode\n"
"new              - sets the board to the initial position\n" 
"quit             - exit program\n"
"white            - sets white on the move, comp plays black\n"
"black            - sets black on the move, comp plays white\n" 
"go               - comp plays the color on the move\n"
"force            - enter force mode\n"
"perft5           - run perft 5\n"
"perft6           - run perft 6\n"
"bench            - run bench\n"
"bench2           - run bench2 \n"
"playother        - sets comp to play color not on move\n"
"printboard       - prints the position after each move\n"
"level 0 <X> <Y>  - set the time control to X with inc Y\n"
"setboard <FEN>   - set the board to the position FEN\n"
"help/?           - this message\n";


typedef void function_ptr ();
typedef struct cmd_table {
  char *cmd;
  function_ptr *fun_ptr;
} cmd_table;
int poll_input();
#endif 
