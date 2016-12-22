/*
* Client.cpp
*
*  Created on: Nov 3, 2016
*      Author: Clayton.Bagnall
*/
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>


fd_set readfds; //set of socket file_descriptors

int main(int argc, char *argv[])
{
	WSADATA wsadata;
	WSAStartup(MAKEWORD(2, 2), &wsadata); // 2.2 version

	int sockfd, portnum;
	int data_write = 0;
	int data_read = 0;
	struct sockaddr_in serv_addr;
	struct hostent *server;

	char send_buff[256];
	char recieve_buff[256];
	if (argc < 3) {
		fprintf(stderr, "usage %s hostname port\n", argv[0]);		//check for invalid input arguments
		exit(0);
	}
	portnum = atoi(argv[2]);
	sockfd = socket(AF_INET, SOCK_STREAM, 0);					//create new socket for client
	if (sockfd < 0) {
		perror("error opening socket");
		exit(1);
	}
	server = gethostbyname(argv[1]);							//retrieve server host-name
	if (server == NULL) {
		fprintf(stderr, "error, no such host\n");
		exit(0);
	}
	memset((char *)&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	memcpy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);

	serv_addr.sin_port = htons(portnum);
	if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)	//connect to host
		perror("error connecting");
	printf("[Client] Connected to server: %s on portnumber: %d\n", server->h_name, portnum);

	int request[41]{ 0 };

	while (true) {
		FD_ZERO(&readfds);										//clear socket set
		FD_SET(sockfd, &readfds); 								//add socket to set

		struct timeval timeout = { 1,0 }; 						//1 second timeout (needs to be reset

		request[0] = 1;
		request[1] = 300;
		request[2] = 100;
		request[3] = 40;
		request[4] = 40;


		data_write = send(sockfd, (char*)request, 41 * sizeof(int), 0); //send data to server
		if (data_write < 0) {
			printf("Error sending message.\n");
			memset(request, 0, 256);
			continue;											//if data is invalid try again
		}
		Sleep(1000);												//delay so server can process message

		data_read = select(sockfd + 1, &readfds, NULL, NULL, &timeout);
		if (data_read < 0) {
			perror("error reading from socket");
			exit(1);
		}
		else if (data_read > 0) {								//if socket contains data read it
			memset(recieve_buff, 0, 2500);						//clear receive_buff (server clears send_buff)
			FD_CLR(sockfd, &readfds);
			data_read = recv(sockfd, recieve_buff, 2500, 0);
			if (data_read > 0) {
#if 0
				FILE* Test;
				Test = fopen("/mnt/dmp.txt", "w+");
				for (int i = 0; i < 40 * 40; i++) {
					if (i % 40 == 0 && i != 0) {
						fprintf(Test, "\n");
					}
					fprintf(Test, "%d,", recieve_buff[i]);
				}
				fclose(Test);
#endif
				break; 											//break out of while loop and exit
			}
		}
	}
	closesocket(sockfd);

	WSACleanup();

	return 0;
}