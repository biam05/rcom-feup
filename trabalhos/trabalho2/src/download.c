#include <stdio.h> 
#include <stdlib.h> 
#include <errno.h> 
#include <netdb.h> 
#include <sys/types.h>
#include <netinet/in.h> 
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

#include "constraints.h"
#include "auxfunctions.c"

/* 
--------------------------------------------------------------------------
----                            MAIN                                  ----
--------------------------------------------------------------------------
*/

int main(int argc, char** argv){

    // ---------------------------- VARIABLES ---------------------------- 

    int socketfd;
    int socketfdClient =-1;
    struct hostent *h;
    int port;
    char ip[17];
    
    struct arguments args;
    
    char responsecode[4], string[4];
    char information[MAX_STRING_LENGTH];
    char filename[MAX_STRING_LENGTH]; memset(filename, 0, MAX_STRING_LENGTH);

    // ----------------------------- PARSING -----------------------------

    parseArgs(argv[1], &args);
    parseFilename(args.path, filename);
    h = getIP(args.host);

    printf("--------------------------------\n");
    printf(" * Username: %s\n", args.user);
    printf(" * Password: %s\n", args.pass);
    printf(" * Host: %s\n", args.host);
    printf(" * Path: %s\n", args.path);
    printf(" * Filename: %s\n", filename);
    printf(" * IP Address: %s\n", inet_ntoa(*((struct in_addr *) h->h_addr)));
    printf("--------------------------------\n");

    // ------------------------ INITIAL CONNECTION -----------------------

    connection(inet_ntoa(*((struct in_addr *)h->h_addr)), &socketfd, SERVER_PORT);
    readResponseCode(socketfd,responsecode); 

    // 220 - Service ready for new user.
	stringMaker(string, "220");
	if (strcmp(responsecode, string) != 0) {
		printf("Error when Establishing Connection\n");
		return 1;
	}

    // ------------------------------- LOGIN ------------------------------

    printf("> sending username\n");
    sprintf(information, "user %s\n", args.user);
    sendInformation(socketfd, information);
    readResponseCode(socketfd,responsecode); 
    // 331 - User name okay, need password
	stringMaker(string, "331");
	if (strcmp(responsecode, string) == 0) {
        printf("> sending password\n");
        sprintf(information, "pass %s\n", args.pass);
        sendInformation(socketfd, information);
        readResponseCode(socketfd,responsecode); 
	}
    // 230 - User logged in, proceed. Logged out if appropriate.
	stringMaker(string, "230");
	if (strcmp(responsecode, string) != 0) {
		printf("Error when Logging in.\n");
		return 1;
	} 

    // ------------------------ ENTER PASSIVE MODE ------------------------

    printf("> sending passive\n");
    sprintf(information, "pasv\n");
    sendInformation(socketfd, information);

    readIPPort(socketfd, responsecode, ip, &port);
    // 227 - Entering Passive Mode (h1,h2,h3,h4,p1,p2).
	stringMaker(string, "227");
	if (strcmp(responsecode, string) != 0) {
		printf("Error Entering Passive Mode.\n");
		return 1;
	} 

    // ------------------------- SECOND CONNECTION ------------------------
    
    connection(inet_ntoa(*((struct in_addr *)h->h_addr)), &socketfdClient, port);

    // --------------------------- DOWNLOAD FILE --------------------------

    printf("> sending retrieve\n");
    sprintf(information, "retr %s\n", args.path);
    sendInformation(socketfd, information);
    readResponseCode(socketfd,responsecode); 

    // 150 - File status okay; about to open data connection.
	stringMaker(string, "150");
	if (strcmp(responsecode, string) != 0) {
		printf("Error Sending Retrieve Command.\n");
		return 1;
	} 

    printf("> starting download\n");
    downloadFile(socketfdClient, filename);
    printf("> download complete\n");

    // --------------------------- CLOSE SOCKETS --------------------------

    close(socketfd);
    close(socketfdClient);

    return 0;
}
