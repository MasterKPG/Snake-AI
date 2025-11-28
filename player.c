// compiler's header files
#include <stdbool.h> // bool, true, false
#include <stdlib.h> // rand
#include <stdio.h> // printf

// main program's header file
#include "snake_def.h"
#include "snake_dec.h"

// student name goes here
char * student="Mohammad Amara"; 

// prototypes of the local/private functions
static void printAction(action);
static void printBoolean(bool);
static bool actionValid(action, char **, int, int);

/*
  snake function called from the main program
  This function randomly selects a valid move for Snake based on its current position on the game map
*/
action snake(
	     char * * map, // array of chars modeling the game map
	     int mapxsize, // x size of the map
	     int mapysize, // y size of the map
	     snake_list s, // snake coded as a linked list
	     action last_action // last action made, set to -1 in the beginning 
	     ) {
  action a; // action to choose and return
  
  bool ok=false; // ok will be set to true as soon as a randomly selected action is valid

  //coordinates of the snake's head
  int x = s->x;
  int y = s->y;
  
  do {
    a=rand()%4; // ramdomly select one of the 4 possible actions: 0=NORTH, 1=EAST, 2=SOUTH, 3=WEST

    if(DEBUG) { // print the randomly selected action, only in DEBUG mode
      printf("Candidate action is: ");
      printAction(a);
      printf("\n");
    }

    ok = actionValid(a, map, x, y);

    if(DEBUG) { // print whether the randomly selected action is valid, only in DEBUG mode
      printf("Is this candidate action valid? ");
      printBoolean(ok);
      printf("\n");
    }
  // while the selected action is not valid and there exists a valide move
  } while(!ok && (map[s->y-1][s->x]==PATH || map[s->y-1][s->x]==BONUS  
		  || map[s->y][s->x+1]==PATH || map[s->y][s->x+1]==BONUS
		  || map[s->y+1][s->x]==PATH || map[s->y+1][s->x]==BONUS
		  || map[s->y][s->x-1]==PATH || map[s->y][s->x-1]==BONUS));

  return a; // answer to the game engine
}

/*
  printAction function:
  This function prints the input action name on screen.
*/
static void printAction(action a) {
  switch(a) {
  case NORTH:
    printf("NORTH");
    break;
  case EAST:
    printf("EAST");
    break;
  case SOUTH:
    printf("SOUTH");
    break;
  case WEST:
    printf("WEST");
    break;
  }
}

/*
  printBoolean funtion:
  This function prints the input boolan value on screen.
*/
static void printBoolean(bool b) {
  if(b) {
    printf("true");
  }
  else {
    printf("false");
  }
}

/*
  actionValid funtion:
  This function checks if the action is valid or not, then changes the ok variable accordingly.
*/
static bool actionValid(action a, char ** map, int x, int y){
  switch(a) { // check whether the randomly selected action is valid, i.e., if its preconditions are satisfied 
  case NORTH: // going toward this direction does not put snake's head into
    if(map[y-1][x]!=WALL // a wall
	    && map[y-1][x]!=SNAKE_BODY // snake's body
	    && map[y-1][x]!=SNAKE_TAIL) // snake's tail
	    return true; // this direction is safe, hence valid!
    break;
  case EAST: // going toward this direction does not put snake's head into
    if(map[y][x+1]!=WALL // a wall
	    && map[y][x+1]!=SNAKE_BODY // snake's body
	    && map[y][x+1]!=SNAKE_TAIL) // snake's tail
	    return true; // this direction is safe, hence valid!
    break;
  case SOUTH: // going toward this direction does not put snake's head into
    if(map[y+1][x]!=WALL // a wall
	    && map[y+1][x]!=SNAKE_BODY // snake's body
	    && map[y+1][x]!=SNAKE_TAIL) // snake's tail
	  return true; // this direction is safe, hence valid!
    break;
  case WEST: // going toward this direction does not put snake's head into
    if(map[y][x-1]!=WALL // a wall
	    && map[y][x-1]!=SNAKE_BODY // snake's body
	    && map[y][x-1]!=SNAKE_TAIL) // snake's tail
	    return true; // this direction is safe, hence valid!
    break;
  }
}