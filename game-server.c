//note: figure out which includes we don't really need
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <netdb.h>
#include <pthread.h>
#include "server-thread-2021.h"
#include "game.h"
#include "playerDAO.h"
#include "authentication.h"
#include "libpq-fe.h"


/*
compile: gcc -lpthread -lcrypt -lpq game-server.c server-thread-2021.c game-database.c authentication.c -o gameserver
run: ./gameserver 17300 
RUN ON FREEBSD1
*/

#define HOST "freebsd1.cs.scranton.edu"
#define BACKLOG 10
#define BUFFER_SIZE 256


typedef struct GAME_CONTEXT {
    int player1fd;
    int player2fd;
    Player* player1;
    Player* player2;

    pthread_mutex_t lock;
    PGconn* conn;
}GameContext;


Player* createPlayer(char *name) {
    Player* this = (Player*)malloc(sizeof(Player));
    strcpy(this->name, name);
    return this;
}


GameContext* initializeGameContext(int player1fd, int player2fd, Player* player1, Player* player2, pthread_mutex_t lock, PGconn* conn) {
    GameContext* this = (GameContext*)malloc(sizeof(GameContext));
    this->player1fd = player1fd;
    this->player2fd = player2fd;
    this->player1 = player1;
    this->player2 = player2;
    this->lock = lock;
    this->conn = conn;
    return this;

}

void initializeBoard(Game* this) {
    for (int x = 0; x < 8; x++) {
        for (int y = 0; y < 8; y++) {
            this->board[x][y] = '-';
        }
    }
}

Game* initializeGame(int p1fd, int p2fd) {
    Game* this = (Game*)malloc(sizeof(Game));
    this->nMoves = 0;
    this->gameOver = 0;
    this->stone = 'B';
	this->p1fd = p1fd;
	this->p2fd = p2fd;
	this->currentfd = p1fd;
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

void printLastMove(Game* this) { //ideally prints name but... game doesn't store that yet
    if(this->stone == 'B'){
        printf("player 1 placed 'B' at (%d, %d)\n", this->x, this->y);
    }
    else{
        printf("player 2 placed 'W' at (%d, %d)\n", this->x, this->y);
    }
}

void sendName(int clientfd, char* name) {
    int len = strlen(name) + 1;
    send(clientfd, &len, sizeof(int), 0);
    send(clientfd, name, len, 0);
}

char* recvName(int clientfd) {
    int len = 0;
    recv(clientfd, &len, sizeof(int), 0);
	char *name = (char*) malloc(len);
	recv(clientfd, name, len, 0);
	return name;
}

void sendPlayerNumber(int clientfd, int num){
	send(clientfd, &num, sizeof(int), 0);
}

//sends information to the current player to start their turn.
//server will send the integer status of the game, and then the board
void sendGameStatus(Game *game){
	send(game->currentfd, &game->gameOver, sizeof(int), 0);
	send(game->currentfd, game->board, 64, 0);
}

void recvMove(Game *game){
	int x, y;
	int isValid = 0;
	while(isValid != 1){
		recv(game->currentfd, &x, sizeof(int), 0);
		recv(game->currentfd, &y, sizeof(int), 0);
		//validate the move
		if(x<0 || x>=8 || y<0 || y>=8){
			isValid = 2; //out of bounds error status
        }
        else if (game->board[x][y] != '-') {
			isValid = 3; //already taken error status
        }
        else {
			isValid = 1;
			//printf("(%d, %d) is a valid move.\n", x, y);
		}
		send(game->currentfd, &isValid, sizeof(int), 0);
	}
	game->x = x;
	game->y = y;
	
}

void takeTurn(Game* game) {
    //printBoard(game);
	sendGameStatus(game);
    recvMove(game);
    game->board[game->x][game->y] = game->stone;
    printLastMove(game);
	send(game->currentfd, game->board, 64, 0);
    game->nMoves = game->nMoves+1;
    if(game->nMoves==MAX_MOVES){
        game->gameOver=TIE;
    }
}

void changeSides(Game* game) {
    if (game->stone == 'B') {
        game->stone = 'W';
		game->currentfd = game->p2fd;
    }
    else {  // game->stone == 'W'
        game->stone = 'B';
		game->currentfd = game->p1fd;
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

void playGame(Game* game) {
    pthread_t verticalThread, horizontalThread;
    while (game->gameOver == NOT_OVER) {
        takeTurn(game);

        pthread_create(&verticalThread, NULL, verticalCheck, (void*)game);
        pthread_create(&horizontalThread, NULL, horizontalCheck, (void*)game);

        pthread_join(verticalThread, NULL);
        pthread_join(horizontalThread, NULL);

        changeSides(game);
    }
}

int sendWinnerNumber(Game* game){
	int win=1;
	int lose=0;
    int ret=0;
	if(game->currentfd==game->p1fd){
		send(game->p1fd, &win, sizeof(int), 0);
		send(game->p2fd, &lose, sizeof(int), 0);
        ret=1;
	}else{
		send(game->p1fd, &lose, sizeof(int), 0);
		send(game->p2fd, &win, sizeof(int), 0);
        ret=2;
	}
    return ret;
}

int endGame(Game *game) {
	sendGameStatus(game);
	changeSides(game);
	sendGameStatus(game);
	int winnerNum=sendWinnerNumber(game);
	printf("end status: %d", game->gameOver);
    return winnerNum;
}

void sendRecordInfo(int playerfd, Player *player){
    sendName(playerfd, player->name);
    sendPlayerNumber(playerfd, player->wins);
    sendPlayerNumber(playerfd, player->losses);
    sendPlayerNumber(playerfd, player->ties);
}

void updateWinNumbers(GameContext *thisGame, int winnerNum, pthread_mutex_t lock, PGconn* conn){
    if(winnerNum==1){
        thisGame->player1->wins=thisGame->player1->wins+1;
        thisGame->player2->losses=thisGame->player2->losses+1;
        pthread_mutex_lock(&lock);
        changeRecord(1, thisGame->player1->email, conn);
        changeRecord(2, thisGame->player2->email, conn);
        pthread_mutex_unlock(&lock);
    }else if(winnerNum==2){
        thisGame->player1->losses=thisGame->player1->losses+1;
        thisGame->player2->wins=thisGame->player2->wins+1;
        pthread_mutex_lock(&lock);
        changeRecord(2, thisGame->player1->email, conn);
        changeRecord(1, thisGame->player2->email, conn);
        pthread_mutex_unlock(&lock);
    }else{
        thisGame->player1->ties=thisGame->player1->ties+1;
        thisGame->player2->ties=thisGame->player2->ties+1;
        pthread_mutex_lock(&lock);
        changeRecord(3, thisGame->player1->email, conn);
        changeRecord(3, thisGame->player2->email, conn);
        pthread_mutex_unlock(&lock);
    }
    sendRecordInfo(thisGame->player1fd, thisGame->player1);
    sendRecordInfo(thisGame->player1fd, thisGame->player2);
    sendRecordInfo(thisGame->player2fd, thisGame->player1);
    sendRecordInfo(thisGame->player2fd, thisGame->player2);
}

void* playOnline(void* ptr) {
    GameContext* thisGame = (GameContext*)ptr;
    Game* game = initializeGame(thisGame->player1fd, thisGame->player2fd);
    playGame(game);
    int winnerNum=endGame(game);
    updateWinNumbers(thisGame, winnerNum, thisGame->lock, thisGame->conn);
    pthread_exit(NULL);
}

/*
List * initializeScoreboard(){
    List* scoreboard=ListCreate();
    return scoreboard;
}
*/

Player * checkStartGame(int playerfd, Player* loggedPlayer, int *loggedPlayersFD, pthread_mutex_t lock, PGconn* conn){
        char *email=recvName(playerfd);
        char *password=recvName(playerfd);
        pthread_mutex_lock(&lock);
        Player *player1=getPlayerByLogin(email, password, (void*)conn);
        pthread_mutex_unlock(&lock);
        if(player1==NULL){
            sendPlayerNumber(playerfd, 1); //sending an error code to client
            return loggedPlayer;
        }
        sendPlayerNumber(playerfd, 0); //sending an OK code
        printf("Player %s logged in\n", player1->email);
        if(loggedPlayer!=NULL){ //both players have connected
                //send info to player 1 
            sendPlayerNumber(*loggedPlayersFD, 1);
            sendName(*loggedPlayersFD, loggedPlayer->name);
            sendName(*loggedPlayersFD, player1->name);
            
            
            //send info to player 2
            sendPlayerNumber(playerfd, 2);
            sendName(playerfd, player1->name);
            sendName(playerfd, loggedPlayer->name);
            //note: all of the accepting players stuff could be moved to its own function, and maybe just keep a "playerInfo" struct

            GameContext* newGame = initializeGameContext(*loggedPlayersFD, playerfd, loggedPlayer, player1, lock, conn);

            pthread_t gameThread;
            pthread_create(&gameThread, NULL, playOnline, (void*)newGame);
            printf("started game with p1 = %s and p2 = %s\n", loggedPlayer->name, player1->name);
            *loggedPlayersFD=0;
            return NULL;
        }else{
            *loggedPlayersFD=playerfd;
            return player1;
        }
}

int recvPlayerOption(int clientfd){
    int choice;
    recv(clientfd, &choice, sizeof(int) ,0);
    return choice;
}

Player* recvRegistrationInfo(int clientfd, pthread_mutex_t lock, PGconn* conn){
    char* email=recvName(clientfd);
    char* password=recvName(clientfd);
    char* name=recvName(clientfd);
    pthread_mutex_lock(&lock);
    Player* newPlayer = registerPlayer(email, password, name, (void*)conn);
    pthread_mutex_unlock(&lock);
    if(newPlayer == NULL){
        sendPlayerNumber(clientfd, 0);
    }else{
        sendPlayerNumber(clientfd, 1);
    }
    return newPlayer;
    
}



int main(int argc, char* argv[]) {
    int serverfd = start_server(HOST, argv[1], BACKLOG);
    if (serverfd == -1) {
        printf("Error: Could not start server");
        exit(1);
    }
    PGconn* conn = PQconnectdb("postgresql://dbuser2:picard456@lovelace.cs.scranton.edu/greenfieldj5");

    pthread_mutex_t lock;
    pthread_mutex_init(&lock, NULL);
    //List * scoreboard=initializeScoreboard();
    /*
        LoggedPlayer keeps track of a logged in player
        if there is one player waiting to play a game
        loggedPlayersFD is their file descriptor
    */
   Player * loggedPlayer=NULL;
   int loggedPlayersFD=0;
    while (1) {
        int playerfd = accept_client(serverfd);
        int option=recvPlayerOption(playerfd);
        if(option==1){
            //registration
            //ListFirst(scoreboard);
            Player* newRegistration=recvRegistrationInfo(playerfd, lock, conn);
            if (newRegistration != NULL) {
                printf("New player registration\nName:%s\nEmail:%s\n", newRegistration->name, newRegistration->email);
            }
            else{ }
        }else if(option==2){
            //logins
            loggedPlayer=checkStartGame(playerfd, loggedPlayer, &loggedPlayersFD, lock, conn);
        }
    }
}
