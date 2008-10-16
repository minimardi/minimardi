#ifndef UTILS
#define UTILS
#include <sys/timeb.h>
#include <stdio.h>
#include <time.h>

typedef struct chess_clock {
  int base; /* time control */
  int increment;
 
  int comp_time; /* centi sec */
  int opp_time; 
  int speed;
  int alloc_time;
  struct timeb allocated; /* in milli sec */
} chess_clock;

int set_time_control(int base, int increment);
int set_comp_time(int time);
int set_opp_time(int time);
int out_of_time();
int allocate_time();
#endif
