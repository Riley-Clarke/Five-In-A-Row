/*
** A simple Server
   This program runs together with client-thread-main-2021.c as one system.

   To demo the whole system, you must:
   1. compile the server program:
        gcc -lpthread -o server server-thread-2021.c server-thread-main-2021.c
   2. run the program on freebsd1: ./server 41000 
   3. compile the cliient program:
        gcc -o client client-thread-2021.c client-thread-main-2021.c
   4. run the client on freebsd2: ./client freebsd1.cs.scranton.edu 41000 socket.html
         replacing HOST and HTTPPORT with the host the server runs
         on and the port # the server runs at.

   FOR STUDNETS:
     If you want to try this demo, you MUST use the port # assigned
     to you by your instructor. Since each http port can be associated
     to only one process (or one server), if another student tries to
     run the server with the default port, it will be rejected.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <netdb.h>
#include <pthread.h>
#include "server-thread-2021.h"

#define HOST "freebsd1.cs.scranton.edu"
#define BACKLOG 10
#define BUFFER_SIZE 256
#define HTTP_REQUEST_TYPE 1
#define GREETING_TYPE 2

void start_subserver(int reply_sock_fd);
void *subserver(void * reply_sock_fd_as_ptr);
void handle_greeting(int reply_sock_fd);
void handle_http_request(int reply_sock_fd);

int main(int argc, char *argv[]) {
   int http_sock_fd;			// http server socket
   int reply_sock_fd;  	                // client connection 

   http_sock_fd = start_server(HOST, argv[1], BACKLOG);

   if (http_sock_fd ==-1) {
      printf("start server error\n");
      exit(1);
   }

   while(1) {
      if ((reply_sock_fd = accept_client(http_sock_fd)) == -1) {
         continue;
      }
      start_subserver(reply_sock_fd);
   }
} 

void start_subserver(int reply_sock_fd) {
   int data_type = 0;
   int nReceived = 1;

   recv(reply_sock_fd, &data_type, sizeof(int), 0);
   while (data_type > 0 && nReceived != 0) {
      if (data_type == HTTP_REQUEST_TYPE) {
         handle_http_request(reply_sock_fd);
      } else if (data_type == GREETING_TYPE) {
         handle_greeting(reply_sock_fd);
      } else {
         printf("Client sent invalid type\n");
      }
      nReceived = recv(reply_sock_fd, &data_type, sizeof(int), 0);
   }
   close(reply_sock_fd);
   printf("Client closed the connection\n");
}

void handle_greeting(int reply_sock_fd) {
   int nExpected = 0;
   int nReceived = 0;
   char buffer[BUFFER_SIZE];
   nReceived = recv(reply_sock_fd, &nExpected, sizeof(int), 0);
   nReceived = recv(reply_sock_fd, buffer, nExpected, 0);
   printf("Client sent: %s\n", buffer);
}

void handle_http_request(int reply_sock_fd) {
   int nExpected = 0;
   int nReceived = 0;
   char buffer[BUFFER_SIZE];

   nReceived = recv(reply_sock_fd, &nExpected, sizeof(int), 0);
   nReceived = recv(reply_sock_fd, buffer, nExpected, 0);

   // get the file name according to HTTP GET method protocol
   char *html_file = strtok(&buffer[5], " \t\n");
   int html_file_fd = open(html_file, O_RDONLY);

   while ((nReceived = read(html_file_fd, buffer, BUFFER_SIZE-1))>0) {
      send(reply_sock_fd, &nReceived, sizeof(int), 0);
      send(reply_sock_fd, buffer, nReceived, 0);
   }
   nReceived = -1;
   send(reply_sock_fd, &nReceived, sizeof(int), 0);
}
