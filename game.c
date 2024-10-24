// Gomoku Game
// Compile: gcc -lpthread -lrt -o game.c game
// or: gcc -lpthread game.c -o game (for directly on server)

#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include "game.h"


#define NOT_OVER 0
#define WINNER 1
#define TIE 2
#define MAX_MOVES 64


// I think if we modify the game.c to take in player objects instead of just relying
// on B and W for turn order we can get away with just sending updates to each client's
// game board, instead of just passing the ENTIRE game object which would take a lot of
// resources

void initializeBoard(Game* this) {
    for (int x = 0; x < 8; x++) {
        for (int y = 0; y < 8; y++) {
            this->board[x][y] = '-';
        }
    }
}

Game* initializeGame() {
    Game* this = (Game*)malloc(sizeof(Game));
    this->nMoves = 0;
    this->gameOver = 0;
    this->stone = 'B';
    initializeBoard(this);
    return this;
}

void printBoard(Game* this) {
    printf("  0 1 2 3 4 5 6 7 \n");
    for (int x = 0; x < 8; x++) {
        printf("%d ", x);
        for (int y = 0; y < 8; y++) {
            printf("%c ", this->board[x][y]);
        }
        printf("\n");
    }
}

void* horizontalCheck(void* ptr) {
    Game* game = (Game*)ptr;
    int consec = 0;
    for (int i = 0; i < 8; i++) {
        if (game->board[game->x][i] == game->stone) {
            consec++;
        }
        else {
            consec = 0;
        }
        if (consec == 5) {
            game->gameOver = WINNER;
        }
    }
    pthread_exit(NULL);
}

void* verticalCheck(void* ptr) {
    Game* game = (Game*)ptr;
    int consec = 0;
    for (int x = 0; x < 8; x++) {
        if (game->board[x][game->y] == game->stone) {
            consec++;
        }
        else {
            consec = 0;
        }

        if (consec == 5) {
            game->gameOver = WINNER;
        }
    }
    pthread_exit(NULL);
}

void changeSides(Game* game) {
    if (game->stone == 'B') {
        game->stone = 'W';
    }
    else {  // game->stone == 'W'
        game->stone = 'B';
    }
}

void printPlayerNumber(Game *game){
    if(game->stone == 'B'){
        printf("Player 1:");
    }
    else{
        printf("Player 2:");
    }
}
// this would probably be the function that would be changed
void getStone(Game *game, int player1fd, int player2fd) {
    int validMove = 0;
    int x;
    int y;
    char userMove[4];
    int currentfd = 0;
    if (game->stone == 'B') {
        currentfd = player1fd;
    }
    else {
        currentfd = player2fd;
    }
    while (validMove == 0) {
        //fgets(userMove, 4, stdin);
        recv(player1fd, userMove, 4, 0);
        sscanf(userMove, "%d %d", &x, &y);
        int c;
        while((c=getchar()) != EOF && c != '\n');
        if(x<0 || x>=8 || y<0 || y>=8){
            send(currentfd, ("Error: (%d, %d) out of bounds.\nPlease try again:", x, y), 49, 0);
        }
        else if (game->board[x][y] != '-') {
            send(currentfd, ("Erorr: (%d, %d) already taken.\nPlease try again:", x, y), 49, 0);
        }
        else {validMove = 1;}
        send(currentfd, &validMove, sizeof(int), 0);
    }
    game->x = x;
    game->y = y;
}

void takeTurn(Game* game, int player1fd, int player2fd) {
    printBoard(game);
    //printPlayerNumber(game);
    getStone(game, player1fd, player2fd);
    game->board[game->x][game->y] = game->stone;
    game->nMoves = game->nMoves+1;
    if(game->nMoves==MAX_MOVES){
        game->gameOver=TIE;
    }
}

void playGame(Game* game, int player1fd, int player2fd) {
    pthread_t verticalThread, horizontalThread;
    while (game->gameOver == NOT_OVER) {
        takeTurn(game, player1fd, player2fd);

        pthread_create(&verticalThread, NULL, verticalCheck, (void*)game);
        pthread_create(&horizontalThread, NULL, horizontalCheck, (void*)game);

        pthread_join(verticalThread, NULL);
        pthread_join(horizontalThread, NULL);

        changeSides(game);
    }
}


/*
int main() {
    Game* game = initializeGame();
    playGame(game);
    printBoard(game);
    if (game->gameOver == TIE) {
        printf("The game is a tie.");
    }
    else {   // gameOver == 1 => winner
        if (game->stone == 'B') {
            printf("Player 2 has won!\n");
        }
        else if (game->stone == 'W') {
            printf("Player 1 has won!\n");
        }
    }


}
*/

