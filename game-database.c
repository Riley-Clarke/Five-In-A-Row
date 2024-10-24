#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "authentication.h"
#include "playerDAO.h"
#include "libpq-fe.h"



static void finishConn(PGconn* conn) {
	PQfinish(conn);
	exit(1);
}

Player* createPlayerObject(char* email, char* password, char* name, int wins, int loses, int ties) {
	Player* this = (Player*)malloc(sizeof(Player));
	strcpy(this->email,email);
	strcpy(this->password,password); //encrypted
	strcpy(this->name, name);
	this->wins = wins;
	this->losses = loses;
	this->ties = ties;
	return this;
}

Player* registerPlayer(char* email, char* password, char* name, void* ptr) {
	PGconn* conn = (PGconn*)ptr;
	Player* result = NULL;
	char* encryptedPass = encode(password);
	const char* params[100] = { email, encryptedPass };
	const int paramLen[100] = { strlen(email), strlen(encryptedPass) };
	PGresult* res = PQexecParams(conn, "insert into player_login (email, password) values ($1, $2);", 2, NULL, params, paramLen, NULL, 0);
	if (PQcmdTuples(res) == "0") {
		printf("Error: could not add player.\n");
	}
	else {
		const char* params2[100] = { email, name };
		const int paramLen2[100] = { strlen(email), strlen(name) };
		res = PQexecParams(conn, "insert into player_score (email, name, wins, losses, ties) values ($1, $2, 0, 0, 0);", 2, NULL, params2, paramLen2, NULL, 0);
		if (PQcmdTuples(res) == "0") {

			char nameBuffer[11];
			strcpy(nameBuffer, name);
			result = createPlayerObject(email, encryptedPass, nameBuffer, '0', '0', '0');
			printf("Player added!");
		}
		else {
			printf("Error: could not add player.\n");
		}
	}
	return result;
}

/*
void sendScoreboard(int clientfd, void* ptr) {
	PGconn* conn = (PGconn*)ptr;
	PGresult* res = PQexec(conn, "select name, wins, loses, ties from player_score order by wins desc");
	if (PQresultStatus(res) != PGRES_COMMAND_OK) {
		printf("Error: could not retrieve score.\n");
	}
	else {
		List* result = ListCreate();
		for (int i = 0; i <= PQntuples(res); i++) {
			char* name = PQgetvalue(res, i, 0);
			int wins = atoi(PQgetvalue(res, i, 1));
			int loses = atoi(PQgetvalue(res, i, 2));
			int ties = atoi(PQgetvalue(res, i, 3));
			char* entry = ("%s,%d,%d,%d", name, &wins, &loses, &ties);
			char* listEntry = (char*)malloc(sizeof(entry) + 1);
			strcpy(listEntry, entry);
			ListInsert(result, listEntry);
		}
	}
}
*/
Player* getPlayerByLogin(char* email, char* password, void* ptr) {
	PGconn* conn = (PGconn*)ptr;
	Player* result = NULL;
	const char* params[50] = { email }; 
	const int paramLen[50] = { strlen(email) };
	PGresult* res = PQexecParams(conn, "select password from player_login where email = $1;", 1, NULL, params, paramLen, NULL, 0);
	if (PQgetisnull(res, 0, 0) == 1) {
		printf("Error: User not in system.\n");

	}
	else {
		char* encryptedPass = PQgetvalue(res, 0, 0);
		if (authenticate(password, encryptedPass) == 0) {
			printf("Error: Invalid password.\n");
		}
		else {
			res = PQexecParams(conn, "select name, wins, losses, ties from player_score where email = $1;", 1, NULL, params, paramLen, NULL, 0);
			char* name = PQgetvalue(res, 0, 0);
			char nameBuffer[11];
			strcpy(nameBuffer, name);
			int wins = atoi(PQgetvalue(res, 0, 1));
			int loses = atoi(PQgetvalue(res, 0, 2));
			int ties = atoi(PQgetvalue(res, 0, 3));
			result = createPlayerObject(email, encryptedPass, nameBuffer, wins, loses, ties);

		}
	}
	if (result == NULL) {
		printf("result is null.\n");
	}
	else {
		printf("Player Returned: %s, %d, %d, %d\n", result->name, result->wins, result->losses, result->ties);
	}
	return result;
}

void changeRecord(int op, char* email, void* ptr) {
	PGconn* conn = (PGconn*)ptr;
	printf("%s", email);
	int emailLen = strlen(email);
	const char* addTo[50] = { email };
	const int opLen[50] = { emailLen };
	if (op == 1) {	
		PGresult* res = PQexecParams(conn, "update player_score set wins = wins + 1 where email = $1 ", 1, NULL, addTo, opLen, NULL, 0);
	}
	else if (op == 2) {
		PGresult* res = PQexecParams(conn, "update player_score set losses = losses + 1 where email = $1 ", 1, NULL, addTo, opLen, NULL, 0);
	}
	else if (op == 3) {
		PGresult* res = PQexecParams(conn, "update player_score set ties = ties + 1 where email = $1 ", 1, NULL, addTo, opLen, NULL, 0);	
	}

}
/*
int main(int argc, char* argv[]) {

	PGconn* conn = (PGconn*)malloc(sizeof(conn));
	conn = PQconnectdb("postgresql://dbuser2:picard456@lovelace.cs.scranton.edu/greenfieldj5");
	if (PQstatus(conn) != CONNECTION_OK) {
		printf("Connection Failed");
		finishConn(conn);
	}

	int keepGoing = 1;
	while (keepGoing == 1) {
		printf("1. test createPlayer\n2. test getPlayerByLogin\n3. test add win/loss/tie\n4. quit\n");
		int choice;
		scanf(" %d", &choice);
		if (choice == 1) {
			char name[10];
			char email[50];
			char password[50];
			printf("name:");
			scanf(" %s", name);
			printf("email:");
			scanf(" %s", email);
			printf("password:");
			scanf(" %s", password);
			registerPlayer(email, password, name, (void*)conn);
		}
		else if (choice == 2) {
			char email[50];
			char password[50];
			printf("email:");
			scanf(" %s", email);
			printf("password:");
			scanf(" %s", password);
			getPlayerByLogin(email, password, (void*)conn);
		}
		else if (choice == 3) {
			int choice2;
			char email[50];
			printf("1. win\n2. loss\n3. tie\n");
			scanf(" %d", &choice2);
			printf("enter email:\n");
			scanf(" %s", email);
			changeRecord(choice2, email, (void*)conn);
		}
		else if (choice == 4) {
			finishConn(conn);
		}
	}
}
*/
