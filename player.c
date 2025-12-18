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
} PathState;

// prototypes of the local/private functions
static void printAction(action);
static bool actionValid(action, char **, int, int);
//static bool isSnakeBody(snake_list, int, int);
//static graph* createGraph(char **, snake_list, int, int);
//static int countFreePixels(char **, snake_list, int, int);
//static int findNode(graph *, int, int);
//static int adjacentPos(int, int, Position *);
static Position getHeadPos(snake_list);
static Position getTailPos(snake_list);
//static int getHeadAdjacentIndices(graph *, Position, int *);
//static int getTailAdjacentIndices(graph *, Position, int *);
//static bool findHamiltonianRec(graph *, PathState *, int, int *, int);
//static bool findHamiltonianPath(graph *, int *, int, int *, int, int **, int *);
//static action getDirection(Position, Position);
//static void freeGraph(graph *);
static action followTailStrategy(char **, int, int, Position, Position, Position, snake_list);
static int countValidMoves(char **, int, int);
static int getSnakeLength(snake_list);
static action zigzagStrategy(char **, int, int, Position, Position, Position);
static action aggressiveStrategy(char **, int, int, Position, Position);
static action smartStrategy(char **, int, int, Position, Position, Position, snake_list)


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
  
  //Coordinates of the snake's head---------------------------------------------------------------------------
  Position headPos = getHeadPos(s);

  if (DEBUG){//Print the coordinates of the of the head
    printf("X coordinates of the head = %d\nY coordinates of the head = %d\n", headPos.x, headPos.y);
  }
  //----------------------------------------------------------------------------------------------------------

  //Coordinates of the snake's tail---------------------------------------------------------------------------
  Position tailPos = getTailPos(s);

  if (DEBUG){//Print the coordinates of the tail
    printf("X coordinates of the tail = %d\nY coordinates of the tail = %d\n", tailPos.x, tailPos.y);
  }
  //----------------------------------------------------------------------------------------------------------

  //Coordinates of the Bonus----------------------------------------------------------------------------------
  Position bonusPos;

  //Look for the Bonus (we start from 1 and subtract 1 to not waste time looking in the walls)
  for (int row = 1; row < mapysize - 1; row++)
    for (int col = 1; col < mapxsize - 1; col++)
      if (map[row][col] == BONUS){
        bonusPos.x = col;
        bonusPos.y = row;
        break; //Bonus found, break the loop
      }
  
  if (DEBUG){ //Print the coordinates of the bonus
    printf("X coordinates of the bonus = %d\nY coordinates of the bonus = %d\n", bonusPos.x, bonusPos.y);
  }
  //-----------------------------------------------------------------------------------------------------------

  a = followTailStrategy(map, mapxsize, mapysize, headPos, tailPos, bonusPos, s);

  if (DEBUG){
    printf("Follow tail strategy, moving: ");
    printAction(a);
    printf("\n");
  }

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
/*
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
*/

/*
  createGraph function:
  This function takes in the map and the snake list, makes a graph based on the free pixels (paths not occupied by the snake body)
  making the nodes array which contains the nodes with their indices and coordinates and then makes the adjacency matrix of the graph
  Returns the graph variable containing the nodes array, node_count and the adjacency matrix
*/
/*
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
      return NULL;
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
*/

/*
  countFreeCells function:
  This function counts the free cells in the map excluding the entire snake from head to tail.
*/
/*
static int countFreePixels(char **map, snake_list s, int mapxsize, int mapysize){
  int count = 0; //Local variable to count the free cells
  for (int i = 0; i < mapxsize; i++){
    for (int j = 0; j < mapysize; j++){
      //For a pixel to be free it has to be inside the map and not occupied by the snake's body
      if (map[j][i] != WALL && !isSnakeBody(s,i,j)) count++;
    }
  }
  return count;
}
*/

/*
  findNode function:
  This function goes through the nodes array and returns the index of the node in the coordinates x and y
*/
/*
static int findNode(graph *g, int x, int y){
  for (int i = 0; i < g->node_count; i++){//Go through the nodes until the last one, look for a match in coordinates 
    if (g->nodes[i].x == x && g->nodes[i].y == y) return i;//If the coordinates match, return the index
  }
  return -1;//Else return -1
}
*/

/*
  adjacentPos function:
  This function takes in coordinates of a cell, and returns the count of possible adjacent positions,
  aswell as returning their coordinates in the adjacent array passed in the arguments.
*/
/*
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
*/

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
/*
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
*/

/*
  getTailAdjacentIndices function:
  This function takes in the graph and position of the tail and returns indices of free free cells adjacent to tail
*/
/*
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
*/

/*
  findHamiltonianRec function:
  This function uses recursive backtracking to find hamiltonian path
*/
/*
static bool findHamiltonianRec(graph *g, PathState *state, int current_idx, int *tailAdjacentIndices, int tailAdjacentCount){
  //We take in the graph where we do the search, the pathstate pointer where we return the path,
  //the current node index for recursion, the tail adjacent indices and their count to know where we stop for the search,
  

  state->path[state->path_length++] = current_idx; //Add current node to the path
  state->visited[current_idx] = true; //We visited this node because we're on it right now

  //Check if we found a complete hamiltonian path
  if (state->path_length == g->node_count){//We went trhough all nodes 
    //Must end adjacent to tail
    //Check if we're adjacent to tail
    for (int i = 0; i < tailAdjacentCount; i++){
      if (current_idx == tailAdjacentIndices[i]){
        return true;
      }
    }
  
    //Backtrack if not adjacent to tail
    //We take out the current index from the path
    state->path_length--;
    state->visited[current_idx] = false;
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

      if (findHamiltonianRec(g, state, next_idx, tailAdjacentIndices, tailAdjacentCount)){
        //Recall the function on the next index, if the path is complete, return true, otherwise backtrack
        return true;
      }
    }
  }

  //Backtrack
  state->path_length--;
  state->visited[current_idx] = false;

  return false; //Not a hamiltonian path
}
*/

/*
  findHamiltonianPath function:
  This function initializes and finds a Hamiltonian path from head-adjacent to tail-adjacent
*/
/*
static bool findHamiltonianPath(graph *g, int *headAdjacentIndices, int headAdjacentCount, int *tailAdjacentIndices, int tailAdjacentCount, int **result_path, int *path_length){
  
  //We take in the graph where we're looking for the hamiltonian path,
  //the head and tail adjacent indices and count to know from where to start the path and where to end it,
  //a pointer to the result path where we store the final result path, which starts adjacent to the head and ends adjacent to the tail and passes through the bonus,
  //and a pointer to store the path's length, which should match the node count of the graph.
  
  
  //Try starting from nodes adjacent to the head
  for (int i = 0; i < headAdjacentCount; i++){
    int start_idx = headAdjacentIndices[i];

    //Initialize path state
    PathState state;
    state.path = (int*)malloc(sizeof(int) * g->node_count);//Allocate memory for the path array
    state.visited = (bool*)calloc(g->node_count, sizeof(bool));//Allocate memory and initialize with false
    state.path_length = 0;

    if (state.path == NULL || state.visited == NULL){//Memory allocation failed on one of them
      free(state.path);
      free(state.visited);
      return false;
    }

    //Find path using recursive backtracking 
    bool found = findHamiltonianRec(g, &state, start_idx, tailAdjacentIndices, tailAdjacentCount);

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
*/

/*
  getDirection function:
  This function takes in 2 positions next to eachother and gives back the direction to go from the first to the second
*/
/*
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
*/

/*
  freeGraph function:
  This function frees the memory allocated to the graph
*/
/*
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
*/

//Functions for the new strategy (follow tail and grab bonus if on the same path as the tail or close to the path)
/*
  countValidMoves function:
  This function counts the valid moves possible, based on the position given by x and y coordinates
*/
static int countValidMoves(char **map, int x, int y){
  int count = 0; //Counter to return the value

  if (map[y-1][x] != WALL && map[y-1][x] != SNAKE_BODY && map[y-1][x] != SNAKE_TAIL) count++; // NORTH
  if (map[y][x+1] != WALL && map[y][x+1] != SNAKE_BODY && map[y][x+1] != SNAKE_TAIL) count++; // EAST
  if (map[y+1][x] != WALL && map[y+1][x] != SNAKE_BODY && map[y+1][x] != SNAKE_TAIL) count++; // SOUTH
  if (map[y][x-1] != WALL && map[y][x-1] != SNAKE_BODY && map[y][x-1] != SNAKE_TAIL) count++; // WEST

  return count;
}

/*
  followTailStrategy function:
  This function chooses the best move possible, one that will not trap us by following the tail
  and help us take the bonus if it's close to the path to the tail.
*/
static action followTailStrategy(char **map, int mapxsize, int mapysize, Position headPos, Position tailPos, Position bonusPos, snake_list s){
  
  action moves[4] = {NORTH, EAST, SOUTH, WEST}; //Array to iterate through the moves without naming them everytime
  //We store the coordinates changes in these arrays, they represent the how the position changes when moving in each direction,
  //Example: moving NORTH, means the difference between x before and after the move is dx[0] and same for y, dy[0], same goes for EAST with index 1 and so on...
  int dx[4] = {0, 1, 0, -1};
  int dy[4] = {-1, 0, 1, 0};

  int snakeLength = getSnakeLength(s);//Get snake length

  //Calculate dynamic tolerance (ignore getting trapped) based on snake length and map size
  //Longer snake need to have minimal tolerance, can easily get trapped
  //Shorter snake we can go for the bonus without checking because the risk is minimal
  int mapSize = (mapxsize + mapysize)/2; //Calculate the average dimension of the map
  int tolerance = mapSize/2 - snakeLength/3; //To optimize the division constants later with tests (zid bash code test_sequence bach tshl 3lina test)

  //Ensure minimum tolerance of 5
  if (tolerance < 5) tolerance = 5;

  //Here we will store the target position, it will be either tail or bonus
  Position target;

  //Here we will check for the best target between the bonus and the tail, to stay safe and be greedy (based on the tolenrance calculated)
  if (snakeLength <= 5){//Small snake, very low risk of getting trapped (in big maps)
    target = bonusPos;
  } else {//Big snake, be careful and calculate the optimal target between the bonus and tail to stay safe and progress
    //Calculate the distances:
    int distHeadToBonus = abs(headPos.x - bonusPos.x) + abs(headPos.y - bonusPos.y);//Head to bonus distance
    int distHeadToTail = abs(headPos.x - tailPos.x) + abs(headPos.y - tailPos.y); // Head to tail distance
    int distBonusToTail = abs(bonusPos.x - tailPos.x) + abs(bonusPos.y - tailPos.y);// Bonus to tail distance

    //If the bonus is in the path to the tail (or close (tolerance as max number of cells)), go to the bonus directly
    //Otherwise go to tail to stay safe and not get trapped
    if (distHeadToBonus + distBonusToTail <= distHeadToTail + tolerance){
      target = bonusPos; //The bonus is on our path or very close so we go for it
    } else {
      target = tailPos; //Otherwise chase the tail to stay safe
    }
  }

  //Here wee find the best move to reach the target we set, based on a score we will calculate for each move
  action best_move = rand()%4; //We initialize the best move to choose with a random move
  int best_score = -999999; //We calculate the scores of the best 

  for (int i = 0; i < 4; i++){//We go through all the moves possible in the array moves[4]
    //Calculate the new head coordinates after the move
    int newX = headPos.x + dx[i];
    int newY = headPos.y + dy[i];

    //Check if this move is valid
    if (!actionValid(moves[i], map, headPos.x, headPos.y)){//If action not valid we skip this move
      continue;
    }

    int score = 0; //Store the score of the current move based on:
                  // distance to the target (close is better)
                  // space around this position (free cells) (the more the better)
                  // distance to the center to avoid edges (the closer the better)

    // First: distance to target
    int distToTarget = abs(newX - target.x) + abs(newY - target.y);
    score -= distToTarget * 100; // 100 coefficent to prioritize getting closer to the target

    // Second: Space around the position
    int freeNeighbors = countValidMoves(map, newX, newY);
    score += freeNeighbors * 10;

    // Third Avoid edges and corners
    int distToCenter = abs(newX - mapxsize/2) + abs(newY - mapysize/2);
    score -= distToCenter;
    
    if (score > best_score){ //If the current score is better, take this move and update the best_score
      best_score = score;
      best_move = moves[i];
    }
  }

  return best_move;
}

/*
  getSnakeLength function:
  This function returns the length of the snake
*/
static int getSnakeLength(snake_list s){
  int length = 0;
  snake_list current = s;
  while(current != NULL){ //Not the end of the snake
    length++;
    current = current->next;
  }
  return length;
}

/*
  zigzagStrategy function:
  This function returns the action needed to go in a zigzag pattern while leaving a path on the bottom of the map
  so the snake doesn't get trapped.
*/
static action zigzagStrategy(char **map, int mapxsize, int mapysize, Position headPos, Position tailPos, Position bonusPos){
  
  action moves[4] = {NORTH, EAST, SOUTH, WEST}; //Array to iterate through the moves without naming them everytime
  //We store the coordinates changes in these arrays, they represent the how the position changes when moving in each direction,
  //Example: moving NORTH, means the difference between x before and after the move is dx[0] and same for y, dy[0], same goes for EAST with index 1 and so on...
  int dx[4] = {0, 1, 0, -1};
  int dy[4] = {-1, 0, 1, 0};

  //zigzag pattern: move right, go down at the edge of the map, move left, go down, repeat
  //With this we can sweep the map and get the bonus in case it's too far away without getting trapped

  //Check if we're at the right edge (near the right wall)
  bool atRightEdge = (headPos.x >= mapxsize - 3); // Leave path for the snake to comeback -1 for the wall, -1 for the free path and -1 where the snake should be
  //Check if we're at the left edge (near the right wall)
  bool atLeftEdge = (headPos.x <= 2); //same logic because the map starts at 0

  //Determine if we should be moving right or left depending on the y coordinates (lines)
  //we choose:
  //Even lines (from top): move right
  //Odd lines: move left
  bool shouldMoveRight = ((headPos.y - 1)%4 < 2 );

  action preferred_move; //The move that will get us in zigzag path
  action secondary_move; //Move to use incase the zigzag is not possible

  if (shouldMoveRight){// We should move right
    
    if (atRightEdge){// We're close to the right edge, we should move down
      preferred_move = SOUTH; //Zigzag path
      secondary_move = EAST; // Continue right if not possible
    } else { //We're far from the the right edge, we should continue moving right to make the zigzag pattern
      preferred_move = EAST; //Zigzag path
      secondary_move = SOUTH; // Go down if not possible
    }

  } else {// We should move left and follow same logic
    if (atLeftEdge){ //Move down
      preferred_move = SOUTH;
      secondary_move = WEST;
    } else { //Move left
      preferred_move = WEST;
      secondary_move = SOUTH;
    }
  }

  //Try the preferred move first 
  if (actionValid(preferred_move, map, headPos.x, headPos.y)){
    return preferred_move;
  }
  //Else try secondary move
  if (actionValid(secondary_move, map, headPos.x, headPos.y)){
    return secondary_move;
  }

  //In case both not valid, try any valid move with priority to moving towards bonus
  action best_move = rand()%4;
  int best_score = -999999;

  for (int i = 0; i < 4; i++){
    if (actionValid(moves[i], map, headPos.x, headPos.y)){
      int newX = headPos.x + dx[i];
      int newY = headPos.y + dy[i];

      int score = 0;

      //Distance to bonus 
      int distToBonus = abs(newX - bonusPos.x) + abs(newY - bonusPos.y);
      score -= distToBonus * 10;

      //Free space
      int freeNeighbors = countValidMoves(map, newX, newY);
      score += freeNeighbors * 50;

      if (score > best_score){
        best_score = score;
        best_move = moves[i];
      }

    }
  }

  return best_move;
}

/*
  aggressiveStrategy function:
  This function returns the action to move towards the bonus, in an aggressive way,
  meaning it doesn't take into account getting trapped, only checks if the move is valid.
  (The same as the code snippet in the followTailStrategy and zigzagStrategy)
*/
static action aggressiveStrategy(char **map, int mapysize, int mapxsize, Position headPos, Position bonusPos){
  action moves[4] = {NORTH, EAST, SOUTH, WEST};
  int dx[4] = {0, 1, 0, -1};
  int dy[4] = {-1, 0, 1, 0};

  action best_move = rand()%4;
  int best_score = -999999; 

  for (int i = 0; i < 4; i++){
    if (!actionValid(moves[i], map, headPos.x, headPos.y)){
      continue;
    }

    int newX = headPos.x + dx[i];
    int newY = headPos.y + dy[i];

    int score = 0;

    //More aggressive towards bonus (coefficient 200 (checki f test!!!!!!!!))
    int distToBonus = abs(newX - bonusPos.x) + abs(newY - bonusPos.y);
    score -= distToBonus * 200;

    //Add a bit of safety to not make it too risky by taking moves with better escape possibilities
    int freeNeighbors = countValidMoves(map, newX, newY);
    score += freeNeighbors * 30;

    if (score > best_score){
      best_score = score; 
      best_move = moves[i];
    }

  }

  return best_move;
}

/*
  smartStrategy function:
  This function chooses which strategy to go with, between follow tail, zigzag and aggressive based on the situation
  We choose the aggressive strategy if:
    -The snake is too small (length under 5)
    -The snake's head is very close to the bonus (5 cells)
  We choose the zigzag strategy if:
    -The snake is very large (fills up 60% of the map at least)
    -The bonus is very far away in comparison to the tail (over 1,5 x distance to the tail)
  We choose the follow tail strategy if:
    None of the cases before apply to the snake, so we use the default strategy
    and either go for the bonus or the tail to not get trapped
*/
static action smartStrategy(char **map, int mapxsize, int mapysize, Position headPos, Position tailPos, Position bonusPos, snake_list s){
  int snakeLength = getSnakeLength(s);
  int mapSize = (mapxsize + mapysize) / 2;

  //Calculate distances
  int distHeadToBonus = abs(headPos.x - bonusPos.x) + abs(headPos.y - bonusPos.y);
  int distHeadToTail = abs(headPos.x - tailPos.x) + abs(headPos.y - tailPos.y);

  //Snake is small (length <= 5) => aggressive
  if (snakeLength <= 5){
    return aggressiveStrategy(map, mapysize, mapxsize, headPos, bonusPos);
  }

  //Snake's head is close to the bonus (under 5 cells) => aggressive (khasni noptimizi b test sequence !!!!!)
  if (distHeadToBonus <= 5){
    return aggressiveStrategy(map, mapysize, mapxsize, headPos, bonusPos);
  }

  //Snake is big (fills up 60% of the map at least) => zigzag (can be brought down to minimize snake chasing tail)
  int totalCells = (mapxsize - 2) * (mapysize - 2); //No walls
  if (snakeLength > totalCells * 0.6){
    return zigzagStrategy(map, mapxsize, mapysize, headPos, tailPos, bonusPos);
  }

  //Snake's head is too far away from the bonus (> 1,5x distance to tail) => zigzag
  //This prevents the snake from chasing his tail in circles when the bonus is far
  if (distHeadToBonus > distHeadToTail * 1.5 && snakeLength > 15){
    return zigzagStrategy(map, mapxsize, mapysize, headPos, tailPos, bonusPos);
  }

  //None of the cases above fit for the current situation => default, follow tail strategy
  return followTailStrategy(map, mapxsize, mapysize, headPos, tailPos, bonusPos, s);

}