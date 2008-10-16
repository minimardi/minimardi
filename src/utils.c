#include "utils.h"

/* the chess clock */
static chess_clock the_clock;

/*-------------------------------------------------------.
 | function: set_time_control(int base, int increment)   |
 | note: sets the time control, only ICS controls.       |
 | note: for standard, default 2 sec for every move.     |
  -------------------------------------------------------*/
int set_time_control(int base, int increment){
  the_clock.base = base*100;
  the_clock.increment = increment*100;
  the_clock.speed = 35;
  return 1;
}

/*-------------------------------------------------------.
 | function: set_comp_time(int time) -> int              |
 | note: sets the computers clock to time.               |
  -------------------------------------------------------*/
int set_comp_time(int time){
  the_clock.comp_time = time;
  return 1;
}

/*-------------------------------------------------------.
 | function: set_opp_time(int time) -> int               |
 | note: sets the opponents clock to time.               |
  -------------------------------------------------------*/
int set_opp_time(int time){
  the_clock.opp_time = time;
  return 1;
}

/*-------------------------------------------------------.
 | function: out_of_time() -> int                        |
 | returns: 1 if the computer has searched for more than |
 |          the allocated time for this move. Else 0.    |
  -------------------------------------------------------*/
int out_of_time(){
  struct timeb current;
  int time, second, ms;
  ftime(&current);

  second = (int) difftime(current.time, the_clock.allocated.time);
  ms = current.millitm-the_clock.allocated.millitm;
  time = second*1000 + ms;
  if (time < the_clock.alloc_time)
    return 0;
  return 1;
}

/*-------------------------------------------------------.
 | function: allocated_time()                            |
 | note: allocates time for a move.                      |
  -------------------------------------------------------*/
int allocate_time(){
  int time = 0, speed = 0;
  struct timeb current;
  
  ftime(&current);
  speed = the_clock.speed;
  if ((the_clock.opp_time < the_clock.comp_time) && 
      (speed > 10))
    speed -= 2;
  else if (speed < 50) speed += 5;
  the_clock.speed = speed;
  if (the_clock.increment > 0)
    time = (the_clock.increment-1) + (the_clock.comp_time/speed);
  else time = the_clock.comp_time/speed;
  the_clock.allocated = current;
  the_clock.alloc_time = time*10;  
  return time;
}
