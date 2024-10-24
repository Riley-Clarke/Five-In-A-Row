#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include "playerDAO.h"
#include "authentication.h"


Player *registerPlayer(char *email, char *password, char *name, void*ptr){
    Player *newPlayer=(Player*)malloc(sizeof(Player));
    strcpy(newPlayer->email, email);
    strcpy(newPlayer->name,name);
    strcpy(newPlayer->password, encode(password));
    newPlayer->wins=0;
    newPlayer->losses=0;
    newPlayer->ties=0;
    List *scoreboard=(List*)ptr;
    ListInsert(scoreboard, newPlayer);
    return newPlayer;
}

Player *getPlayerByLogin(char *email, char *password, void *ptr){
    List *scoreboard=(List*)ptr;
    ListFirst(scoreboard);
    Player *current=ListGetNext(scoreboard);
    while(current!=NULL){
        if(strcmp(current->email, email)==0 && authenticate(password, current->password)==1){
            return current;
        }
        current=ListGetNext(scoreboard);
    }
    printf("Invalid Login.\n");
    return NULL;
}

/*
void printScoreBoard(List *scoreboard){
    ListFirst(scoreboard);
    Player *current=ListGetNext(scoreboard);
    while(current!=NULL){
        printPlayerRecord(current);
        current=ListGetNext(scoreboard);
    }
}
*/

/*
void printPlayerRecord(Player *player){
    printf("Player: %s, Record: %d-%d-%d\n", player->name, 
        player->wins, player->losses, player->ties);
}
*/

int emailOnRecord(List *scoreboard, char *email){
    ListFirst(scoreboard);
    Player *current=ListGetNext(scoreboard);
    while(current!=NULL){
        if(strcmp(current->email, email)==0){
            return 1;
        }
        current=ListGetNext(scoreboard);
    }
    return 0;
}