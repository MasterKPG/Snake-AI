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

/*
  Path structure for backtracking
*/
typedef struct {
    int *path;           /* Array of node indices */
    bool *visited;       /* Visited array */
    int path_length;     /* Current path length */
    bool bonus_visited;  /* Flag to track if bonus was visited */
} PathState;

// prototypes of the local/private functions
static void printAction(action);
static void printBoolean(bool);
static bool actionValid(action, char **, int, int);
static bool isSnakeBody(snake_list, int, int);
static graph* createGraph(char **, snake_list, int, int);
static int countFreePixels(char **, snake_list, int, int);
static int findNode(graph *, int, int);
static int adjacentPos(int, int, Position *);
static Position getHeadPos(snake_list);
static Position getTailPos(snake_list);
static int getHeadAdjacentIndices(graph *, Position, int *);
static int getTailAdjacentIndices(graph *, Position, int *);
static bool findHamiltonianRec(graph *, PathState *, int, int *, int, int);
static bool findHamiltonianPath(graph *, int *, int, int *, int, int, int **, int *);
static action getDirection(Position, Position);
static void freeGraph(graph *);

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

  //Coordinates of the snake's head---------------------------------------------------------------------------
  Position headPos;
  headPos = getHeadPos(s);

  if (DEBUG){//Print the coordinates of the of the head
    printf("X coordinates of the head = %d\nY coordinates of the head = %d\n", headPos.x, headPos.y);
  }
  //----------------------------------------------------------------------------------------------------------

  //Coordinates of the snake's tail---------------------------------------------------------------------------
  Position tailPos;

  //Use the function getTailPos to assign the coordinates of the tail to tailPos.x and tailPos.y
  tailPos = getTailPos(s);

  if (DEBUG){//Print the coordinates of the tail
    printf("X coordinates of the tail = %d\nY coordinates of the tail = %d\n", tailPos.x, tailPos.y);
  }
  //----------------------------------------------------------------------------------------------------------

  //Coordinates of the Bonus----------------------------------------------------------------------------------
  Position bonusPos;

  //Look for the Bonus (we start from 1 and subtract 1 to not waste time looking in the walls)
  bool found = false;
  for (int row = 1; row < mapysize - 1 && !found; row++)
    for (int col = 1; col < mapxsize - 1 && !found; col++)
      if (map[row][col] == BONUS){
        bonusPos.x = col;
        bonusPos.y = row;
        found = true;
      }
  
  if (DEBUG){ //Print the coordinates if the bonus has been found
    if (found){
      printf("X coordinates of the bonus = %d\nY coordinates of the bonus = %d\n", bonusPos.x, bonusPos.y);
    }
    printf("The bonus has been found: ");
    printBoolean(found);
    printf("\n");
  }
  //-----------------------------------------------------------------------------------------------------------
  
  do {
    //a=rand()%4; // ramdomly select one of the 4 possible actions: 0=NORTH, 1=EAST, 2=SOUTH, 3=WEST

    //Build graph from map (excluding snake body)
    graph *g = createGraph(map, s, mapxsize, mapysize);
    
    if (g == NULL){//Failed to make the graph, return a random move
      a = rand()%4;
    }

    //Find bonus index in the graph
    int bonus_idx = findNode(g, bonusPos.x, bonusPos.y);

    if (bonus_idx == -1){//Failed to find the bonus, free the graph and choose a random move
      freeGraph(g);
      a = rand()%4;
    }

    //Get cells and counts of cells adjacent to head and tail
    int headAdjacentIndices[4];
    int tailAdjacentIndices[4];

    int headAdjacentCount = getHeadAdjacentIndices(g, headPos, headAdjacentIndices);
    int tailAdjacentCount = getTailAdjacentIndices(g, tailPos, tailAdjacentIndices);

    if (headAdjacentCount == 0 || tailAdjacentCount == 0){//no adjacent cells to neither, free the graph and choose a random move
      freeGraph(g);
      a = rand()%4;
    }

    //Find hamiltonian path from a cell adjacent to the head to a cell adjacent to a cell adjacent to the tail
    int *path = NULL;
    int path_length = 0;
    bool found = findHamiltonianPath(g, headAdjacentIndices, headAdjacentCount, tailAdjacentIndices, tailAdjacentCount, bonus_idx, &path, &path_length); 

    if (found && path_length >= 1){//We found a path of at least 1 node
      //Get the first position in the path (adjacent  to the head)
      Position next = g->nodes[path[0]];

      //Calculate the direction from the head to next (the adjacent cell)
      a = getDirection(headPos, next);

      free(path); //free the path since we already got the action we're looking for 
    }

    freeGraph(g);

    if(DEBUG) { // print the randomly selected action, only in DEBUG mode
      printf("Candidate action is: ");
      printAction(a);
      printf("\n");
    }

    ok = actionValid(a, map, headPos.x, headPos.y);

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
        g->adjacency[i][j] = 1; //The nodes are adjacent 
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

  return count;
}

/*
  getHeadPos function:
  This function takes in the snake list and returns the position of the head in the Position type variable.
*/
static Position getHeadPos(snake_list s){
  Position HeadPos;
  HeadPos.x = s->x;
  HeadPos.y = s->y;
  return HeadPos;
}

/*
  getHeadPos function:
  This function takes in the snake list and returns the position of the tail in a Position type variable.
*/
static Position getTailPos(snake_list s){
  Position tailPos;
  snake_list current = s;

  while(current->next != NULL){
    current = current->next;
  }

  tailPos.x = current->x;
  tailPos.y = current->y;
  return tailPos;
}

/*
  getHeadAdjacentIndices function:
  This function takes in the graph and position of the head and returns indices of free free cells adjacent to head
*/
static int getHeadAdjacentIndices(graph *g, Position headPos, int *indices){
  Position adjacent[4]; //Array of adjacent positions
  int adj_count = adjacentPos(headPos.x, headPos.y, adjacent); //Count of adjacent positions

  int found_count = 0;
  for (int i = 0; i < adj_count; i++){ //We go through the nodes and check if they exist inside the map, if so we add 1 to found_count and add it to the indices array
    int index = findNode(g, adjacent[i].x, adjacent[i].y);
    if (index != -1){//The node exists
      indices[found_count++] = index;
    }
  }
  return found_count;
}

/*
  getTailAdjacentIndices function:
  This function takes in the graph and position of the tail and returns indices of free free cells adjacent to tail
*/
static int getTailAdjacentIndices(graph *g, Position tailPos, int *indices){
  Position adjacent[4]; //Array of adjacent positions
  int adj_count = adjacentPos(tailPos.x, tailPos.y, adjacent); //Count of adjacent positions

  int found_count = 0;
  for (int i = 0; i < adj_count; i++){ //We go through the nodes and check if they exist inside the map, if so we add 1 to found_count and add it to the indices array
    int index = findNode(g, adjacent[i].x, adjacent[i].y);
    if (index != -1){//The node exists
      indices[found_count++] = index;
    }
  }
  return found_count;
}

/*
  findHamiltonianRec function:
  This function uses recursive backtracking to find hamiltonian path
*/
static bool findHamiltonianRec(graph *g, PathState *state, int current_idx, int *tailAdjacentIndices, int tailAdjacentCount, int bonus_idx){
  /*we take in the graph where we do the search, the pathstate pointer where we return the path,
  the current node index for recursion, the tail adjacent indices and their count to know where we stop for the search,
  and the bonus index to see if we pass through it (we will surely do because it's a hamiltonian path and this is added for eventual optimizations)
  */

  state->path[state->path_length++] = current_idx; //Add current node to the path
  state->visited[current_idx] = true; //We visited this node because we're on it right now

  //Check if it's the bonus
  if (current_idx == bonus_idx){
    state->bonus_visited = true;
  }

  //Check if we found a complete hamiltonian path
  if (state->path_length == g->node_count){//We went trhough all nodes 
    //Must end adjacent to tail and pass by the bonus
    if (state->bonus_visited){
      //Check if we're adjacent to tail
      for (int i = 0; i < tailAdjacentCount; i++){
        if (current_idx == tailAdjacentIndices[i]){
          return true;
        }
      }
    }
  
    //Backtrack if conditions not met
    //We take out the current index from the path
    state->path_length--;
    state->visited[current_idx] = false;
    if (current_idx == bonus_idx){//The current index is the bonus, set the visited variable to false since it got taken out of the path
      state->bonus_visited = false;
    }
    return false;
  }

  //Try all adjacent unvisited nodes
  for (int next_idx = 0; next_idx < g->node_count; next_idx++){
    if (g->adjacency[current_idx][next_idx] && !state->visited[next_idx]){

      //If the next index is adjacent and not visited

      //OPTIMIZATION : if this is the last cell, check if it's adjacent to tail
      if (state->path_length == g->node_count - 1){
        bool is_tail_adjacent = false;
        for (int i = 0; i < tailAdjacentCount; i++){
          if (next_idx == tailAdjacentIndices[i]){//It's adjacent to the tail
            is_tail_adjacent = true;
            break;
          }
        }

        if (!is_tail_adjacent){//If it's not adjacent to the tail, skip
          continue;
        }
      }

      if (findHamiltonianRec(g, state, next_idx, tailAdjacentIndices, tailAdjacentCount, bonus_idx)){
        //Recall the function on the next index, if the path is complete, return true, otherwise backtrack
        return true;
      }
    }
  }

  //Backtrack
  state->path_length--;
  state->visited[current_idx] = false;
  if (current_idx == bonus_idx){
    state->bonus_visited = false;
  }

  return false; //Not a hamiltonian path
}

/*
  findHamiltonianPath function:
  This function initializes and finds a Hamiltonian path from head-adjacent to tail-adjacent
*/
static bool findHamiltonianPath(graph *g, int *headAdjacentIndices, int headAdjacentCount, int *tailAdjacentIndices, int tailAdjacentCount, int bonus_idx, int **result_path, int *path_length){
  /*
  We take in the graph where we're looking for the hamiltonian path,
  the head and tail adjacent indices and count to know from where to start the path and where to end it,
  the bonus index to see if we go through the bonus (this will be useful for optimization),
  a pointer to the result path where we store the final result path, which starts adjacent to the head and ends adjacent to the tail and passes through the bonus,
  and a pointer to store the path's length, which should match the node count of the graph.
  */
  
  //Try starting from nodes adjacent to the head
  for (int i = 0; i < headAdjacentCount; i++){
    int start_idx = headAdjacentIndices[i];

    //Initialize path state
    PathState state;
    state.path = (int*)malloc(sizeof(int) * g->node_count);//Allocate memory for the path array
    state.visited = (bool*)calloc(g->node_count, sizeof(bool));//Allocate memory and initialize with false
    state.path_length = 0;
    state.bonus_visited = false;

    if (state.path == NULL || state.visited == NULL){//Memory allocation failed on one of them
      free(state.path);
      free(state.visited);
      return false;
    }

    //Find path using recursive backtracking 
    bool found = findHamiltonianRec(g, &state, start_idx, tailAdjacentIndices, tailAdjacentCount, bonus_idx);

    if (found){//We found a hamiltonian path
      *result_path = state.path;
      *path_length = state.path_length;
      free(state.visited);
      return true;
    }

    //Clean up if not found with this start
    free(state.path);
    free(state.visited);
  }

  //No paths have been found
  return false;
}

/*
  getDirection function:
  This function takes in 2 positions next to eachother and gives back the direction to go from the first to the second
*/
static action getDirection(Position from, Position to){
  //Distance between them
  int dx = to.x - from.x;
  int dy = to.y - from.y;

  if (dx == 1 && dy == 0) return EAST;
  if (dx == -1 && dy == 0) return WEST;
  if (dx == 0 && dy == 1) return SOUTH;
  if (dx == 0 && dy == -1) return NORTH;

  return rand()%4; //return random action (to fix error since we need to have a return otherwise it bugs)
}

/*
  freeGraph function:
  This function frees the memory allocated to the graph
*/
static void freeGraph(graph *g){
  if (g == NULL) return;

  if (g->adjacency != NULL){
    for (int i = 0; i < g->node_count; i++){
      free(g->adjacency[i]);
    }
    free(g->adjacency);
  }

  free(g->nodes);
  free(g);

}