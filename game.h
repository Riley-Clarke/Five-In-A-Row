#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>

#define NOT_OVER 0
#define WINNER 1
#define TIE 2
#define MAX_MOVES 64

typedef struct GAME {
    int nMoves;      // total number of valid moves
    int gameOver;    // 0: not over, 1: a winner, 2: a tie
    char stone;      // 'W': for white stone, 'B' for black
    int x, y;        // (x, y): current move
    char board[8][8];
    int p1fd, p2fd, currentfd; //for keeping track of fds
} Game;

//might not be needed since all the functions are distributed between game-server and game-player
//(neither uses all of them, some aren't used at all anymore)
void initializeBoard(Game* this);
Game* initializeGame();
void changeSides(Game* game);
void printPlayerNumber(Game* game);
void getStone(Game* game);
void takeTurn(Game* game);
void playGame(Game* game);
