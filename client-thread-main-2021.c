/*
** a simple web browser
** compile: gcc -o client client-thread-main-2021.c client-thread-2021.c
** run: client freebsd1.cs.scranton.edu PORT socket.html
**
** PORT is the socket port of the server. 
** make sure the server has been started before this.
**
** Basic steps for client to communicate with a servr
** 1: get a socket
** 2: connect to the server using ip and port
** 3: read and/or write as many times as needed
** 4: close the socket.
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "client-thread-2021.h"

#define BUFFER_SIZE 256 
#define HTTP_REQUEST_TYPE 1
#define GREETING_TYPE 2

int get_user_choice();

char *compose_http_request(char *filename);
void send_http_request(int server_socket, char *http_request);
void receive_http_response(int server_socket);

void send_greeting(int server_socket, char * greeting);

void sendPacket(int server_socket, int type, char *message);

int main(int argc, char *argv[]) {
    int server_socket = get_server_connection(argv[1], argv[2]);

    int choice = get_user_choice();
    while (choice > 0 ) { 
       if (choice == 1) {
          send_http_request(server_socket, argv[3]);
          receive_http_response(server_socket);
       } else if (choice == 2) {
          char *greeting = "Hello World!\n";
          send_greeting(server_socket, greeting);
       }
       choice = get_user_choice();
   }       
   close(server_socket);
}

int get_user_choice() {
   int choice = 1;
   printf("1: send URL, 2: send greeting, 0: quit\n");
   scanf("%d", &choice);

   return choice;
}

char *compose_http_request(char *filename) {
    char *http_request = (char*) malloc(BUFFER_SIZE);
    memset(http_request, '\0', BUFFER_SIZE);
    strcpy(http_request, "GET /");
    strcpy(&http_request[5], filename);
    strcpy(&http_request[5+strlen(filename)], " ");

    return http_request;
}

void send_http_request(int server_socket, char *webpage) {
   char *http_request = compose_http_request(webpage);

   sendPacket(server_socket, HTTP_REQUEST_TYPE, http_request);
}
 
void receive_http_response(int server_socket) {
   int nExpected = 0;
   int nReceived = 0;
   char buf[BUFFER_SIZE];

   recv(server_socket, &nExpected, sizeof(int), 0);
   while (nExpected > 0) {
      nReceived=recv(server_socket, buf, nExpected,  0);
      buf[nReceived] = '\0';
      printf("%s",buf);
      recv(server_socket, &nExpected, sizeof(int), 0);
   }
}
void send_greeting(int server_socket, char *greeting) {
   printf("msg: %s\n", greeting);
   sendPacket(server_socket, GREETING_TYPE, greeting);
}

void sendPacket(int server_socket, int type, char *message) {
   int nBytes = strlen(message) + 1;
   send(server_socket, &type, sizeof(int), 0);
   send(server_socket, &nBytes, sizeof(int), 0);
   send(server_socket, message, nBytes, 0);
}
