#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include "online-game-structs.h"


Player* createPlayer(int clientfd, char stone, char name[10]) {
	Player* this = (Player*)malloc(sizeof(Player));
	this->clientfd = clientfd;
	this->stone = stone;
	strcpy(this->name, name);
	return this;
}


Gomoku_Game* makeGame(Player* player1, Player* player2, int serverfd) {
	Gomoku_Game* thisGame = (Gomoku_Game*)malloc(sizeof(Gomoku_Game));
	thisGame->player1 = player1;
	thisGame->player2 = player2;
	thisGame->game = initializeGame();
	thisGame->serverfd = serverfd;
	return thisGame;

}