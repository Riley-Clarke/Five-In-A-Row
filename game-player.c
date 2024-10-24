//note: figure out which includes we don't really need
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "client-thread-2021.h"
#include "game.h"
#include "authentication.h"

#define BUFFER_SIZE 256 

/*
compile: gcc -lpthread -lcrypt game-player.c client-thread-2021.c authentication.c -o player
run: ./player freebsd1.cs.scranton.edu 17300

run on freebsd 2 or 3
*/

void recvAndPrintBoard(int serverfd){
    char currentBoard[8][8];
    recv(serverfd, currentBoard, 64, 0);
    printf("  0 1 2 3 4 5 6 7 \n");
    for (int x = 0; x < 8; x++) {
        printf("%d ", x);
        for (int y = 0; y < 8; y++) {
            printf("%c ", currentBoard[x][y]);
        }
        printf("\n");
    }
}

void sendName(int serverfd, char* name) {
    int len = strlen(name) + 1;
    send(serverfd, &len, sizeof(int), 0);
    send(serverfd, name, len, 0);
}

char* recvName(int serverfd) {
    int len = 0;
    recv(serverfd, &len, sizeof(int), 0);
	char *name = (char*) malloc(len);
	recv(serverfd, name, len, 0);
	return name;
}

int recvPlayerNumber(int serverfd){
    int num = 0;
	int bytes = recv(serverfd, &num, sizeof(int), 0);
    //printf("bytes received: %d\n", bytes); //for debugging
	return num;
}

int recvGameStatus(int serverfd){
    int status = 0;
    recv(serverfd, &status, sizeof(int), 0);
    recvAndPrintBoard(serverfd);
    return status;
}

void printErrors(int error, int x, int y){
    if(error == 2){
        printf("Error: (%d, %d) out of bounds.\nPlease try again:", x, y);
    } 
    if(error == 3){
        printf("Error: (%d, %d) already taken.\nPlease try again:", x, y);
    }  
}

void sendMove(int serverfd){
    int isValid = 0;
    int x;
    int y;
    printf("enter a move:");
    while (isValid != 1) {
        scanf("%d %d", &x, &y);
        int c;
        while((c=getchar()) != EOF && c != '\n');
        send(serverfd, &x, sizeof(int), 0);
        send(serverfd, &y, sizeof(int), 0);
        
        //verify that the move is valid
        recv(serverfd, &isValid, sizeof(int), 0);
        printErrors(isValid, x, y); 
    }
}

int recvEndMessage(int serverfd){
    int winnerStatus;
    recv(serverfd, &winnerStatus, sizeof(int), 0);
    return winnerStatus;
}

int recvRegStatus(int serverfd){
    int status;
    recv(serverfd, &status, sizeof(int), 0);
    return status;
}

void recvAndPrintScoreboard(int serverfd){
    char *name = recvName(serverfd);
    int wins = recvRegStatus(serverfd);
    int losses = recvRegStatus(serverfd);
    int ties = recvRegStatus(serverfd);

    printf("%s: %dW/%dL/%dT\n", name, wins, losses, ties);
}
void sendPlayerChoice(int serverfd, int choice){
    send(serverfd, &choice, sizeof(int), 0);
}

void sendRegistrationInfo(int serverfd, char *email, char *password, char *name){
    sendName(serverfd, email);
    sendName(serverfd, password);
    sendName(serverfd, name);
}

void sendLoginInfo(int serverfd, char* email, char *password){
    sendName(serverfd, email);
    sendName(serverfd, password);
}

void loginPlayer(char* argv[]){
    char* email = malloc(sizeof(char)*50);;
    char* password = malloc(sizeof(char)*50);

    printf("Enter your email: \n");
    scanf("%s", email);
    printf("Enter your password: \n");
    int c;
    while((c = getchar()) != '\n' && c!= EOF){}
    getpasswd(password, 50);

    printf("password: %s\n", password);

    int serverfd = get_server_connection(argv[1], argv[2]);
    sendPlayerChoice(serverfd, 2);
    sendLoginInfo(serverfd, email, password);

    int loginStatus = recvRegStatus(serverfd);
    if(loginStatus == 0){
    printf("waiting for game...\n");
    //wait to receive player info
    int playernum = recvPlayerNumber(serverfd);
    char *name = recvName(serverfd);
    char *oppname = recvName(serverfd);

    printf("Player %d: %s, opponent: %s\n", playernum, name, oppname);

    //receive status of the game
    int status = 0;
    while(status == 0){
        status = recvGameStatus(serverfd);
        if(status == NOT_OVER){
            sendMove(serverfd);
            recvAndPrintBoard(serverfd);
        }
        else if(status == WINNER){
            int isWinner=recvEndMessage(serverfd);
            if(isWinner==1){
                printf("You won and %s lost\n", oppname);
            }else{
                printf("You lost and %s won\n", oppname);
            }
        }
        else{ //tie
            printf("it's a draw.\n");
        }
    }
    //print out w/l/t
    recvAndPrintScoreboard(serverfd);
    recvAndPrintScoreboard(serverfd);
    close(serverfd);   
    }
    else{
        printf("error: invalid username or password. please try again.\n");
        loginPlayer(argv);
    }
}

void registerPlayer(char* argv[]){
    
    char* name = malloc(sizeof(char)*50);
    char* email = malloc(sizeof(char)*50);;
    char* password = malloc(sizeof(char)*50);

    printf("Enter your name: \n");
    scanf("%s", name);
    printf("Enter your email: \n");
    scanf("%s", email);
    printf("Enter a password: \n");
    int c;
    while((c = getchar()) != '\n' && c!= EOF){}
    getpasswd(password, 50);
    //scanf("%s", password);
    int serverfd = get_server_connection(argv[1], argv[2]);
    sendPlayerChoice(serverfd, 1);
    //printf("sent player choice\n");
    sendRegistrationInfo(serverfd, email, password, name);
    //printf("sent registration info \n");
    int status =recvRegStatus(serverfd);
    close(serverfd);
    if(status == 0){
        //printf("registered: %s %s %s\n", name, email, password);
        printf("registered!\n");
    }
    else{
        printf("error: email already taken. please try again.\n");
        registerPlayer(argv);
    }
}



int main(int argc, char* argv[]) {
    printf("would you like to: \n1: register\n2: login\n");
    int choice;
    
    scanf("%d", &choice);
    getchar();
    if(choice == 2){
        loginPlayer(argv);
    }
    else if(choice == 1){
        registerPlayer(argv);
    }
    else{
        printf("invalid choice!");
    }

}