// compiler's header files
#include <stdbool.h> // bool, true, false
#include <stdlib.h> // rand
#include <stdio.h> // printf

// main program's header file
#include "snake_def.h"
#include "snake_dec.h"

// student name goes here
char * student="Mohammad Amara"; 

/*
  Position struct, this has the coordinates of a node in the map
*/
typedef struct {//Position in the map (this will be used to define graph nodes)
    int x;
    int y;
} Position;

/*
  Graph struct, this will be used to define a graph in the map based on the free nodes (PATH/BONUS)
*/
typedef struct {
    Position *nodes;    //Array of free nodes (for example node[O] = {1,2} means the node 0 is the pixel at x=1 and y=2)
    int node_count;     //Number of free nodes/pixels in the map/graph
    int **adjacency;    //Adjacency matrix
} graph;

// prototypes of the local/private functions
static void printAction(action);
static void printBoolean(bool);
static bool actionValid(action, char **, int, int);
static void snakeTail(snake_list, int *, int *);
static bool isSnakeBody(snake_list, int, int);
static graph* createGraph(char **, snake_list, int, int);
static int countFreePixels(char **, snake_list, int, int);
static int findNode(graph *, int, int);
static int adjacentPos(int, int, Position *);

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

  //Coordinates of the snake's head
  int hx = s->x;
  int hy = s->y;

  if (DEBUG){//Print the coordinates of the of the head
    printf("X coordinates of the head = %d\nY coordinates of the head = %d\n", hx, hy);
  }

  //Coordinates of the snake's tail
  int tx;
  int ty;

  //Use the function snakeTail to assign the coordinates of the tail to tx and ty
  snakeTail(s, &tx, &ty);

  if (DEBUG){//Print the coordinates of the tail
    printf("X coordinates of the tail = %d\nY coordinates of the tail = %d\n", tx, ty);
  }

  //Coordinates of the Bonus
  int bx;
  int by;

  //Look for the Bonus (we start from 1 and subtract 1 to not waste time looking in the walls)
  bool found = false;
  for (int row = 1; row < mapysize - 1 && !found; row++)
    for (int col = 1; col < mapxsize - 1 && !found; col++)
      if (map[row][col] == BONUS){ bx = col; by = row; found = true; }
  
  if (DEBUG){ //Print the coordinates if the bonus has been found
    if (found){
      printf("X coordinates of the bonus = %d\nY coordinates of the bonus = %d\n", bx, by);
    }
    printf("The bonus has been found: ");
    printBoolean(found);
    printf("\n");
  }

  do {
    a=rand()%4; // ramdomly select one of the 4 possible actions: 0=NORTH, 1=EAST, 2=SOUTH, 3=WEST

    if(DEBUG) { // print the randomly selected action, only in DEBUG mode
      printf("Candidate action is: ");
      printAction(a);
      printf("\n");
    }

    ok = actionValid(a, map, hx, hy);

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
  return false;
}

/*
  snakeTail funtion:
  This function goes through the snake linked list to get the coordinates of the tail
  then changes the tx and ty variables accordingly.
*/
static void snakeTail(snake_list s, int *ptx, int *pty){
  if (s->c == SNAKE_HEAD && s->next == NULL ){// if the snake's length is 1, meaning it's the start of the game
    //Assign the head's coordinates to the coordinates of the tail 
    //since they're technically overlapping and they're at the same node of the map
    *ptx = s->x;
    *pty = s->y;
    return;
  } else if (s->c == SNAKE_TAIL){//We found the tail, we assign the coordinates to the variables
    *ptx = s->x;
    *pty = s->y;
    return;
  } else {//We need to go deeper since we're still at the body or the head of the snake
    snakeTail(s->next, ptx, pty);
  }
}

/*
  isSnakeBody funtion:
  This function checks if the coordinates passed in the arguments are occupied by the snake's body 
  (including head and tail).
*/
static bool isSnakeBody(snake_list s, int x, int y){
  snake_list current = s;
  while(current != NULL){
    if (current->x == x && current->y == y){
      return true;
    }
    current = current->next;
  }
  return false;
}

/*
  createGraph function:
  This function takes in the map and the snake list, makes a graph based on the free pixels (paths not occupied by the snake body)
  making the nodes array which contains the nodes with their indices and coordinates and then makes the adjacency matrix of the graph
  Returns the graph variable containing the nodes array, node_count and the adjacency matrix
*/
static graph* createGraph(char **map, snake_list s, int mapxsize, int mapysize){
  graph *g = (graph*)malloc(sizeof(graph));
  if (g == NULL) return NULL; //Failed memory allocation

  //Counting the free pixels
  g->node_count = countFreePixels(map, s, mapxsize, mapysize);

  //Allocating the nodes array
  g->nodes = (Position*)malloc(sizeof(Position)*g->node_count); 
  if (g->nodes == NULL){//The allocation has failed
    free(g);
    return NULL;
  }

  //Filling the nodes array with the free pixels that can be used
  int i = 0; //Index for counting graph nodes
  for (int y = 1; y < mapysize - 1; y++){
    for (int x = 1; x < mapxsize -1; x++){
      if (map[y][x] != WALL && !isSnakeBody(s, x, y)){
        //The node i contains it's position in the map given by the coordinates x and y
        //We increment the index i to go to the next pixel/node
        g->nodes[i].x = x;
        g->nodes[i].y = y;
        i++;
      }
    }
  }

  //Allocating adjacency matrix
  g->adjacency = (int**)malloc(sizeof(int *)*g->node_count);//Matrix with node_count rows, each row is a pointer to a pointer to an array of integers
  if (g->adjacency == NULL){//The allocation has failed
    free(g->nodes);
    free(g);
    return NULL;
  }

  for (int i = 0; i < g->node_count; i++){
    g->adjacency[i] = (int*)calloc(g->node_count,sizeof(int));//Matrix with node_count columns, with int elements (1 or 0) initialized with zeros
    if (g->adjacency[i] == NULL){
      for (int j = 0; j < i; j++){//The allocation has failed
        free(g->adjacency[j]);
      }
      free(g->adjacency);
      free(g->nodes);
      free(g);
    }
  }
  
  //Fill the adjacency matrix 1s where the pixels are adjacent (the matrix is symmetrical so we can optimize later and do the calculations 1/2 times)
  for (int i = 0; i < g->node_count; i++){
    for(int j = 0; j < g->node_count; j++){//We go through the matrix and calculate the distance between node i and j coordinates which can tell us if they're adjacent or not (1 = adjacent)
      int dx = abs(g->nodes[i].x - g->nodes[j].x);//Distance between nodes on the x axis 
      int dy = abs(g->nodes[i].y - g->nodes[j].y);//Distance between nodes on the y axis
      //The nodes are adjacent if the distance between them is 1 on one of the axis so either dx=1 or dy=1 and the distance to the other is 0
      if ( (dx == 1 && dy == 0) || (dx == 0 && dy == 1) ){
        g->adjacency[i][j] == 1; //The nodes are adjacent 
      }
    }
  }
  return g; //The graph is ready, with the nodes array and the adjacency matrix full
}

/*
  countFreeCells function:
  This function counts the free cells in the map excluding the entire snake from head to tail.
*/
static int countFreePixels(char **map, snake_list s, int mapxsize, int mapysize){
  int count; //Local variable to count the free cells
  for (int i = 0; i < mapxsize; i++){
    for (int j = 0; j < mapysize; j++){
      //For a pixel to be free it has to be inside the map and not occupied by the snake's body
      if (map[j][i] != WALL && !isSnakeBody(s,i,j)) count++;
    }
  }
  return count;
}

/*
  findNode function:
  This function goes through the nodes array and returns the index of the node in the coordinates x and y
*/
static int findNode(graph *g, int x, int y){
  for (int i = 0; i < g->node_count; i++){//Go through the nodes until the last one, look for a match in coordinates 
    if (g->nodes[i].x == x && g->nodes[i].y == y) return i;//If the coordinates match, return the index
  }
  return -1;//Else return -1
}

/*
  adjacentPos function:
  This function takes in coordinates of a cell, and returns the count of possible adjacent positions,
  aswell as returning their coordinates in the adjacent array passed in the arguments.
*/
static int adjacentPos(int x, int y, Position *adjacent){
  int count = 0;

  //North of the cell
  adjacent[count].x = x;
  adjacent[count].y = y - 1;
  count++;

  //South of the cell
  adjacent[count].x = x;
  adjacent[count].y = y + 1;
  count++;

  //East of the cell
  adjacent[count].x = x + 1;
  adjacent[count].y = y;
  count++;

  //West of the cell
  adjacent[count].x = x - 1;
  adjacent[count].y = y;
  count++;
}