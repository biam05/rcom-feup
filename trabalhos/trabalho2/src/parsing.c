#include <string.h>

#include "constraints.h"

// ---------------------------------------------------
// ---------------- PARSING FUNCTIONS ----------------
// ---------------------------------------------------

/**
 * Parse the argument given into user, password, host and path
**/
void parseArgs(char * arguments, struct arguments * args){
    // ./download ftp://[<user>:<password>@]<host>/<url-path>
    char * ftp = strtok(arguments, "/");
    if (strcmp(ftp, "ftp:") != 0) {
        perror("not using ftp");
        exit(1);
    }
    char *aux = strtok(NULL, "/");
    char *path = strtok(NULL, "");
    char *user = strtok(aux, ":");
    char *pass = strtok(NULL, "@");
    char *host;

    if (pass == NULL) {
        //anonymous user
        user = "anonymous";
        pass = "kjewbkwfbe";
        host = aux;
    }
    else {
        host = strtok(NULL, "");
    }
    args->user = user;
    args->pass = pass;
    args->host = host;
    args->path = path;
}

/**
 * Parse the path given to get the filename
**/
void parseFilename(char * path, char * filename) {
    int indexFilename = 0;
    memset(filename, 0, MAX_STRING_LENGTH);

    for (size_t indexPath = 0; indexPath < strlen(path); indexPath++){
        if (path[indexPath] == '/') {
            indexFilename = 0;
            memset(filename, 0, MAX_STRING_LENGTH);
        }
        else {
            filename[indexFilename] = path[indexPath];
            ++ indexFilename;
        }
    }
}

/**
 * Parse IP and Port read in passive mode
 * 
 * FTP server subcommand
 * PORT h1,h2,h3,h4,p1,p2
 * 
 * h n
 * Represents the system IP address and is a character string that is a decimal value between 0 and 255.
 * 
 * p n
 * Represents the TCP port number and is a character string that is a decimal value between 0 and 255.
 * 
 * To convert the p1 and p2 values to a TCP port number, use this formula:
 * port = ( p1 * 256 ) + p2
 * 
 * For example, in this PORT subcommand:
 *     PORT 9,180,128,180,4,8
 * the port number is 1032 and the IP address is 9.180.128.180.
 **/
void parseIPPort(char * buf, char * ip, int * port) {
    strtok(buf, "(");
    char* h1 = strtok(NULL, ",");
    char* h2 = strtok(NULL, ",");
    char* h3 = strtok(NULL, ",");
    char* h4 = strtok(NULL, ",");
    char* p1 = strtok(NULL, ",");
    char* p2 = strtok(NULL, ")");

    sprintf(ip, "%s.%s.%s.%s", h1, h2, h3, h4);

    *port = atoi(p1)*256 + atoi(p2);   
}
