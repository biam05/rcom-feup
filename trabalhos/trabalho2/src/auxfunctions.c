#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>

#include "constraints.h"
#include "parsing.c"

/**
 * Function that returns the IP auxiliary information from host
 * Based on file: getip.c
 **/
struct hostent * getIP(char host[]) {
    struct hostent * h;
    if ((h = gethostbyname(host)) == NULL) {  
        herror("gethostbyname");
        exit(1);
    }
    return h;
}

/**
 * Function that makes the connection with the server address
 * Based on file: clientTCP.c
 **/
int connection(char * ip, int * socketfd, int port) {
	struct sockaddr_in server_addr;

    /* server address handling */
	bzero((char *) & server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(ip);
	/* 32 bit Internet address network byte ordered */
	server_addr.sin_port = htons(port);
    /*server TCP port must be network byte ordered */

    /* open a TCP socket */
	if ((*socketfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket()");
		return 1;
	}

    /* connect to the server */
    if (connect(*socketfd, (struct sockaddr *) &server_addr,
        sizeof(server_addr)) < 0) {
		perror("connect()");
		return 1;
	}
    return 0;
}

/**
 * Response Code String
 */
void stringMaker(char * string, char * maker) {
    string[0] = maker[0];
    string[1] = maker[1];
    string[2] = maker[2];
    string[3] = 0;
}

/**
 * Sends information through socket
 **/
int sendInformation(int fd, char * information){
    int s = send(fd, information, strlen(information), 0);
    return s;
}

/**
 * Reads the response from the socket
 **/
void readResponse(int fd, char * buf) {
    do {
        bzero(buf, SOCKET_BUFFER_LENGTH);
        recv(fd, buf, SOCKET_BUFFER_LENGTH, 0);
    } while (buf[3] != ' ');
}

/**
 * Reads the response code from the socket
 */
void readResponseCode(int fd, char * response) {
    char buf[SOCKET_BUFFER_LENGTH];
    readResponse(fd, buf);
    stringMaker(response, buf);
    printf("\t< %s", buf);
}

/**
 * Reads the response from the socket
 * Determines the port
 */
void readIPPort(int fd, char * response, char * ip, int * port) {
    char buf[SOCKET_BUFFER_LENGTH];
    readResponse(fd, buf);
    stringMaker(response, buf);
    printf("\t< %s", buf);
    parseIPPort(buf, ip, port);
}

/**
 * Creates the file with the data that has to be downloaded
 **/
void downloadFile(int fd, char * filename) {
    FILE *file = fopen((char *)filename, "wb+");

	char buf[SOCKET_BUFFER_LENGTH];
	int size;
	while ((size = recv(fd, buf, SOCKET_BUFFER_LENGTH, 0)) != 0) {
		fwrite(buf, size, 1, file);
	}

    fclose(file);

    printf("> finished downloading file\n");
}
