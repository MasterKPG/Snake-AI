#ifndef SNAKE_DEF_H
#define SNAKE_DEF_H

// global definitions

enum actions {NORTH, EAST, SOUTH, WEST};
typedef enum actions action;

struct snake_link {char c; int x; int y; struct snake_link * next;};
typedef struct snake_link * snake_list;

action snake(char * *, int, int, snake_list, action);

#endif
