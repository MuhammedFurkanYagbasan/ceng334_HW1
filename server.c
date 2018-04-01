#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <limits.h>
#include "model.h"

#define PIPE(fd) socketpair(AF_UNIX, SOCK_STREAM, PF_UNIX, fd)

char **theMap;
int mapWidth, mapHeight;

void updateTheMap(int numberOfObstacles,
    int numberOfHunters,
    int numberOfPreys,
    obstacle obstacles[],
    player hunters[],
    player preys[]){

    for(int x=0; x<mapHeight; x++){
        for(int y=0; y<mapWidth; y++){
            theMap[x][y] = ' ';
        }
    }

    for(int i=0; i<numberOfObstacles; i++) {
        theMap[obstacles[i].pos.x][obstacles[i].pos.y] = 'X';
    }
    
    for(int i=0; i<numberOfPreys; i++) {
        if(preys[i].isAlive)
            theMap[preys[i].pos.x][preys[i].pos.y] = 'P';
    }

    for(int i=0; i<numberOfHunters; i++) {
        if(hunters[i].isAlive)
            theMap[hunters[i].pos.x][hunters[i].pos.y] = 'H';
    }
}

void printTheMap(int numberOfObstacles,
    int numberOfHunters,
    int numberOfPreys,
    obstacle obstacles[],
    player hunters[],
    player preys[]) {

    updateTheMap(numberOfObstacles, numberOfHunters, numberOfPreys, obstacles, hunters, preys);

    printf("+");
    for(int i=0; i<mapWidth; i++) {
        printf("-");
    }
    printf("+\n");

    for(int x=0; x<mapHeight; x++){
        printf("|");
        for(int y=0; y<mapWidth; y++){
            printf("%c",theMap[x][y]);
        }
        printf("|\n");
    }
    printf("+");
    for(int i=0; i<mapWidth; i++) {
        printf("-");
    }
    printf("+\n");
}
////////////////////////////////////////////////////////
void nearestAdvPos(coordinate pos, int numberOfAdvs, player advs[], coordinate *result){
    int min = INT_MAX;
    
    for(int i=0; i<numberOfAdvs; i++) {
        if(advs[i].isAlive)
            if(MD(pos, advs[i].pos, 0, 0) < min) {
                min = MD(pos, advs[i].pos, 0, 0);
                (*result).x = advs[i].pos.x;
                (*result).y = advs[i].pos.y;
            }
    }
}

int neighborNumberForHunter(coordinate pos, coordinate *result) {
    int count=0;
    int x = pos.x;
    int y = pos.y;

    if(x+1<mapHeight && (theMap[x+1][y] == 'X' || theMap[x+1][y] == 'H')){
        result[count].x=x+1;
        result[count].y=y;
        count++;
    }
    if(x-1>=0 && (theMap[x-1][y] == 'X' || theMap[x-1][y] == 'H')){
        result[count].x = x-1;
        result[count].y = y;
        count++;
    }
    if(y+1<mapWidth && (theMap[x][y+1] == 'X' || theMap[x][y+1] == 'H')){
        result[count].x = x;
        result[count].y = y+1;
        count++;
    }
    if(y-1>=0 && (theMap[x][y-1] == 'X' || theMap[x][y-1] == 'H')){
        result[count].x = x;
        result[count].y = y-1;
        count++;
    }
    for(int i=count; i<4; i++){
        result[count].x = -1;
        result[count].y = -1;
    }
    return count;
}

int neighborNumberForPrey(coordinate pos, coordinate *result) {
    int count=0;
    int x = pos.x;
    int y = pos.y;
    
    if(x+1<mapHeight && (theMap[x+1][y] == 'X' || theMap[x+1][y] == 'P')){
        result[count].x=x+1;
        result[count].y=y;
        count++;
    }
    if(x-1>=0 && (theMap[x-1][y] == 'X' || theMap[x-1][y] == 'P')){
        result[count].x = x-1;
        result[count].y = y;
        count++;
    }
    if(y+1<mapWidth && (theMap[x][y+1] == 'X' || theMap[x][y+1] == 'P')){
        result[count].x = x;
        result[count].y = y+1;
        count++;
    }
    if(y-1>=0 && (theMap[x][y-1] == 'X' || theMap[x][y-1] == 'P')){
        result[count].x = x;
        result[count].y = y-1;
        count++;
    }
    for(int i=count; i<4; i++){
        result[count].x = -1;
        result[count].y = -1;
    }
    return count;
}

int isValidMoveForHunter(coordinate nextPos) {
    if (theMap[nextPos.x][nextPos.y] != 'X' && theMap[nextPos.x][nextPos.y] != 'H')
        return 1;
    return 0;
}

int isValidMoveForPrey(coordinate nextPos) {
    return (theMap[nextPos.x][nextPos.y] != 'X' && theMap[nextPos.x][nextPos.y] != 'P');
}

int didHunterEatPrey(coordinate c) {
    return (theMap[c.x][c.y] == 'P');
}

int didPreyJumpOnHunter(coordinate c) {
    return (theMap[c.x][c.y] == 'H');
}

int findPlayer(coordinate c, int number, player players[]){
    int idx=-1;
    for(int i=0;i<number;i++){
        if(c.x == players[i].pos.x && c.y == players[i].pos.y && players[i].isAlive)
            idx=i;
    }
    return idx;
}

int countAlives(int size, player arr[]){
    int counter = 0;
    for(int i=0; i<size; i++){
        if(arr[i].isAlive) counter++;
    }
    return counter;
}

int main() {

    /* VARIABLES */

    int numberOfObstacles;
    int numberOfHunters;
    int numberOfPreys;
    pid_t pid;
    int status;
    
    /**********************************************************************************/
    /* GET INPUT AND PRINT */

    scanf("%d %d", &mapWidth, &mapHeight);
    theMap = (char **)malloc(mapHeight * sizeof(char *));
    for (int i=0; i<mapHeight; i++)
         theMap[i] = (char *)malloc(mapWidth * sizeof(char));

    scanf("%d", &numberOfObstacles);
    obstacle obstacles[numberOfObstacles];
    for(int i=0; i<numberOfObstacles; i++){
        scanf("%d %d", &(obstacles[i].pos
            .x), &(obstacles[i].pos
        .y));
    }

    scanf("%d", &numberOfHunters);
    player hunters[numberOfHunters];
    for(int i=0; i<numberOfHunters; i++){
        scanf("%d %d %d", &(hunters[i].pos
            .x), &(hunters[i].pos
        .y), &(hunters[i].energy));
        hunters[i].isAlive = 1;
    }

    scanf("%d", &numberOfPreys);
    player preys[numberOfPreys];
    for(int i=0; i<numberOfPreys; i++){
        scanf("%d %d %d", &(preys[i].pos
            .x), &(preys[i].pos
        .y), &(preys[i].energy));
        preys[i].isAlive = 1;
    }

    printTheMap(numberOfObstacles, numberOfHunters, numberOfPreys, obstacles, hunters, preys);
    fflush(stdout);
    /* END OF INPUT SECTION */
    /**********************************************************************************/
    /* CREATE ALL PLAYERS */

    for(int i=0; i<numberOfHunters; i++) {
        int fd[2];
        PIPE(fd);
        pid = fork();
        
        if(pid) {
            //parent
            close(fd[1]);
            hunters[i].pid = pid;
            hunters[i].fd = fd[0];

            server_message msg;
            msg.pos.x = hunters[i].pos.x;
            msg.pos.y = hunters[i].pos.y;
            nearestAdvPos(hunters[i].pos, numberOfPreys, preys, &(msg.adv_pos));
            msg.object_count = neighborNumberForHunter(msg.pos, msg.object_pos);

            write(hunters[i].fd, &msg, sizeof(server_message));
            FILE* file = fdopen(hunters[i].fd, "w");
            fflush(file);

        } else {
            //child
            dup2(fd[0],0);
            dup2(fd[1],1);
            close(0);
            for(int e=3; e<=fd[1]; e++)
                close(e);
            
            char mapW[12];
            sprintf(mapW, "%d", mapWidth);
            char mapH[12];
            sprintf(mapH, "%d", mapHeight);
            execl("./hunter","./hunter", mapW, mapH, (char*) NULL);
            break;
        }

    }

    for(int i=0; i<numberOfPreys; i++) {
        int fd[2];
        PIPE(fd);
        pid = fork();
        
        if(pid) {
            //parent
            close(fd[1]);
            preys[i].pid = pid;
            preys[i].fd=fd[0];

            server_message msg;
            msg.pos.x = preys[i].pos.x;
            msg.pos.y = preys[i].pos.y;
            nearestAdvPos(preys[i].pos, numberOfHunters, hunters, &(msg.adv_pos));
            msg.object_count = neighborNumberForPrey(msg.pos, msg.object_pos);

            write(preys[i].fd, &msg, sizeof(server_message));
            FILE* file = fdopen(preys[i].fd, "w");
            fflush(file);

        } else {
            //child
            dup2(fd[0],0);
            dup2(fd[1],1);
            close(0);
            for(int e=3; e<=fd[1]; e++)
                close(e);
            
            char mapW[12];
            sprintf(mapW, "%d", mapWidth);
            char mapH[12];
            sprintf(mapH, "%d", mapHeight);
            execl("./prey","./prey", mapW, mapH, (char*) NULL);
            break;
        }

    }

    /* END OF PLAYER CREATION SECTION */
    /**********************************************************************************/
    /* THE LOOP */

    fd_set readset;
    ph_message msgReceived;
    int total = numberOfHunters+numberOfPreys;
    int m = total+3;
    int gameOver = 0;
    if(total != 0)
    while(!gameOver) {
        
        FD_ZERO(&readset);
        for(int i =0; i<numberOfHunters; i++)
            if(hunters[i].isAlive)
                FD_SET(hunters[i].fd, &readset);

        for(int i =0; i<numberOfPreys; i++)
            if(preys[i].isAlive)
                FD_SET(preys[i].fd, &readset);
        
        select(m, &readset, NULL,NULL,NULL);

        for(int i =0; i<numberOfHunters; i++) {
            if(hunters[i].isAlive)
                if (FD_ISSET(hunters[i].fd, &readset)) {
                    read(hunters[i].fd, &msgReceived, sizeof(server_message));

                    if(isValidMoveForHunter(msgReceived.move_request)){
                        hunters[i].pos.x=msgReceived.move_request.x; 
                        hunters[i].pos.y=msgReceived.move_request.y;
                        hunters[i].energy--;
                        if(didHunterEatPrey(hunters[i].pos)){
                            updateTheMap(numberOfObstacles, numberOfHunters, numberOfPreys, obstacles, hunters, preys);
                            int foundPrey = findPlayer(hunters[i].pos, numberOfPreys, preys);
                            hunters[i].energy += preys[foundPrey].energy;
                            preys[foundPrey].isAlive = 0;
                            close(preys[foundPrey].fd);
                            kill(preys[foundPrey].pid, SIGTERM);
                            waitpid(preys[foundPrey].pid, &status, WNOHANG);
                        }
                        if(hunters[i].energy == 0){
                            hunters[i].isAlive = 0;
                            close(hunters[i].fd);
                            kill(hunters[i].pid, SIGTERM);
                            waitpid(hunters[i].pid, &status, WNOHANG);
                        }
                        //updateTheMap(numberOfObstacles, numberOfHunters, numberOfPreys, obstacles, hunters, preys);

                        printTheMap(numberOfObstacles, numberOfHunters, numberOfPreys, obstacles, hunters, preys);
                        fflush(stdout);
                    }
                    updateTheMap(numberOfObstacles, numberOfHunters, numberOfPreys, obstacles, hunters, preys);

                    if(hunters[i].isAlive){
                        server_message msg;
                        msg.pos.x = hunters[i].pos.x;
                        msg.pos.y = hunters[i].pos.y;
                        nearestAdvPos(msg.pos, numberOfPreys, preys, &(msg.adv_pos));
                        msg.object_count = neighborNumberForHunter(msg.pos, msg.object_pos);

                        write(hunters[i].fd, &msg, sizeof(server_message));
                        FILE* file = fdopen(hunters[i].fd, "w");
                        fflush(file);
                    }
                }
        }

        if(countAlives(numberOfHunters, hunters) == 0) {
            for(int k=0; k<numberOfPreys; k++){
                if(preys[k].isAlive){
                    preys[k].isAlive = 0;
                    close(preys[k].fd);
                    kill(preys[k].pid, SIGTERM);
                    waitpid(preys[k].pid, &status, WNOHANG);
                }
            }
            gameOver = 1;
        } else if(countAlives(numberOfPreys, preys) == 0) {
            for(int k=0; k<numberOfHunters; k++){
                if(hunters[k].isAlive){
                    hunters[k].isAlive = 0;
                    close(hunters[k].fd);
                    kill(hunters[k].pid, SIGTERM);
                    waitpid(hunters[k].pid, &status, WNOHANG);
                }
            }
            gameOver = 1;
        }

        for(int i =0; i<numberOfPreys; i++) {
            if(preys[i].isAlive)
                if (FD_ISSET(preys[i].fd, &readset)) {
                    read(preys[i].fd, &msgReceived, sizeof(server_message));

                    if(isValidMoveForPrey(msgReceived.move_request)){
                        preys[i].pos.x=msgReceived.move_request.x;
                        preys[i].pos.y=msgReceived.move_request.y;

                        updateTheMap(numberOfObstacles, numberOfHunters, numberOfPreys, obstacles, hunters, preys);

                        if(didPreyJumpOnHunter(preys[i].pos)){
                            int foundHunter = findPlayer(preys[i].pos, numberOfHunters, hunters);
                            hunters[foundHunter].energy += preys[i].energy;
                            preys[i].isAlive = 0;
                            close(preys[i].fd);
                            kill(preys[i].pid, SIGTERM);
                            waitpid(preys[i].pid, &status, WNOHANG);
                        }

                        printTheMap(numberOfObstacles, numberOfHunters, numberOfPreys, obstacles, hunters, preys);
                        fflush(stdout);
                    }
                    updateTheMap(numberOfObstacles, numberOfHunters, numberOfPreys, obstacles, hunters, preys);

                    server_message msg;
                    msg.pos.x = preys[i].pos.x;
                    msg.pos.y = preys[i].pos.y;
                    nearestAdvPos(msg.pos, numberOfHunters, hunters, &(msg.adv_pos));
                    msg.object_count = neighborNumberForPrey(msg.pos, msg.object_pos);

                    write(preys[i].fd, &msg, sizeof(server_message));
                    FILE* file = fdopen(preys[i].fd, "w");
                    fflush(file);

                }
        }
        //if(!gameOver)
        //printTheMap(numberOfObstacles, numberOfHunters, numberOfPreys, obstacles, hunters, preys);
    }

    /* END OF THE LOOP */

    return 0;
}
