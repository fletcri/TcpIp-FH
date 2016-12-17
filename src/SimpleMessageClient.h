#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netdb.h>

#define ERROR_SERVERADDR_WRONG 10
#define ERROR_FAILED_TO_CONNECT 11
#define ERROR_SENDING_REQUEST 12

//Gets called from Parameter-Parser if -h is used or one parameter is invalid
void parsingFailed(FILE* file, const char* message, int value);


