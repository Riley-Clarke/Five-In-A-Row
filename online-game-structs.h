#include "game.h"

typedef struct PLAYER {
	int clientfd;
	char stone;
	char name[11];
}Player;

typedef struct GOMOKU_GAME {
	Player* player1;
	Player* player2;
	Game* game;
	int serverfd;

}Gomoku_Game;


Player* createPlayer(int clientfd, char stone, char name[10]);
Gomoku_Game* makeGame(Player* player1, Player* player2, int serverfd);