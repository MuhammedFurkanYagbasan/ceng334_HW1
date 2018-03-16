#include <stdio.h>
#include <sys/types.h>

typedef struct coordinate {
	int x;
	int y;
} coordinate;

typedef struct server_message {
	coordinate pos;
	coordinate adv_pos;
	int object_count;
	coordinate object_pos[4];
} server_message;

typedef struct obstacle {
    coordinate pos;
} obstacle;

typedef struct player {
	int fd;
	pid_t pid;
	int isAlive;
    coordinate pos;
    int energy;
} player;

typedef struct ph_message {
	coordinate move_request;
} ph_message;

int MD(coordinate c1, coordinate c2, int c2OffSetX, int c2OffSetY) {
    return abs(c1.x - (c2.x + c2OffSetX)) + abs(c1.y - (c2.y + c2OffSetY));
}

int isEquelCoordinates(coordinate c1, coordinate c2, int c2OffSetX, int c2OffSetY){
	if(c1.x == (c2.x + c2OffSetX) && c1.y == (c2.y + c2OffSetY))
		return 1;
	else
		return 0;
}