#ifndef PLAYER_H
#define PLAYER_H

#include "snake_def.h" // snake_list

extern char * student; // student name here

action snake(
	     char * * map, // array of char modeling the game map
	     int mapxsize, // x size of the map
	     int mapysize, // y size of the map
	     snake_list s, // snake
	     action last_action // last action made, -1 in the beginning 
	     );

#endif
