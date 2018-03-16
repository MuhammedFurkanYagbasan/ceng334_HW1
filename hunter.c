#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include "model.h"

server_message msg;
ph_message msgToSend;
coordinate currentPos;
int mapWidth, mapHeight;
int minDistance;

void updateMsgToSend(int xOffset, int yOffset) {
	int newDidtance = MD(msg.adv_pos, currentPos, xOffset, yOffset);
	if( newDidtance < minDistance){
		minDistance = newDidtance; 
		msgToSend.move_request.x = currentPos.x + xOffset;
		msgToSend.move_request.y = currentPos.y + yOffset;
	}
}

int checkDirection(int xOffset, int yOffset) {

	for(int i=0; i<msg.object_count; i++) {
		if( isEquelCoordinates(msg.object_pos[i], currentPos, xOffset, yOffset)){
			return 0;
		}
	}
	return 1;
}

int main(int argc, char **argv) {

	mapWidth = atoi(argv[1]);
	mapHeight = atoi(argv[2]);

    while(1) {
    	read(1, &msg, sizeof(server_message));

    	currentPos.x = msg.pos.x;
    	currentPos.y = msg.pos.y;
    	msgToSend.move_request.x = currentPos.x;
    	msgToSend.move_request.y = currentPos.y;

    	minDistance = MD(currentPos, msg.adv_pos, 0, 0);

    	if(currentPos.x+1<mapHeight && checkDirection(1, 0)){
    		updateMsgToSend(1,0);
    	}
    	if(currentPos.x-1>=0 && checkDirection(-1, 0)) {
    		updateMsgToSend(-1, 0);
    	}
    	if(currentPos.y+1<mapWidth && checkDirection(0, 1)){
    		updateMsgToSend(0, 1);
    	}
    	if(currentPos.y-1>=0 && checkDirection(0, -1)) {
    		updateMsgToSend(0, -1);
    	}

    	write(1, &msgToSend, sizeof(ph_message));
    	FILE* file2 = fdopen(1, "w");
    	fflush(file2);

    	usleep(10000*(1+(rand()%9)));

    }


	return 0;
}