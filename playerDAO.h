#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include "LinkedList.h"
//have to include linkedList.h i think
//and we need an implementation of linkedList for player objects
typedef struct PLAYER {
   char email[50];
   char password[50];  // encoded
   char name[11];
   int wins;
   int losses;
   int ties;
} Player;

//inserts a new player into the system.
//if there's an error (email already taken) then return null.
Player *registerPlayer(char *email, char *password, char *name, void *ptr);

//get a player's info by their login credentials
Player *getPlayerByLogin(char *email, char *password, void *ptr);

//Print all users in scoreboard and their record
//we might not need it...
void sendScoreboard(int clientfd, void *ptr);

//Returns an int for whether or not an email is already registered.
//Returns 1 if email is already registered, or 0 if not registered yet.
//also might not need it...
int emailOnRecord(List *scoreboard, char *email);

void changeRecord(int op, char* email, void* ptr);