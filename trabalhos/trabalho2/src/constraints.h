#ifndef CONSTRAINTS_H
#define CONSTRAINTS_H

// ---------------------------------------------------
// ------------------ CONSTRAINTS --------------------
// ---------------------------------------------------

// ------------------ SERVER INFORMATION ------------------ 
#define SERVER_ADDR             "192.168.28.96"
#define SERVER_PORT             21
// ------------------ MAX LENGTHS ------------------ 
#define MAX_STRING_LENGTH       50
#define SOCKET_BUFFER_LENGTH    1000
// ------------------ INDEXES FOR PARSINS ------------------ 
#define FTP_INDEX               0
#define USER_INDEX              1
#define PASSWORD_INDEX          2
#define HOST_INDEX              3
#define PATH_INDEX              4

typedef struct arguments {
    char* user;
    char* pass;
    char* host;
    char* path;
} arguments;

#endif // CONSTRAINTS_H