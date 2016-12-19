/**
 * @file ClientUtils.h
 * SimpleMessageClient
 * TCPIP Project
 *
 * @author Christoph Fletzer <ic16b014@technikum-wien.at>
 * @date 2016/12/19
 *
 * @version 001
 *
 */

/*
 * -------------------------------------------------------------- includes --
 */
#include <stdio.h>
/*
 * --------------------------------------------------------------- defines --
 */
#ifndef CLIENTUTILS_H_
#define CLIENTUTILS_H_
/*
 * -------------------------------------------------------------- typedefs --
 */
typedef struct RequestPacketDef
{
	char* User;
	char* Image;
	char* Message;
} RequestPacket;
/*
 * ------------------------------------------------------------- functions --
 */
void initLog(int verbose, char* progName);
void printHelp();
void log(int logLevel, char* logMessage, ...);
int connectSocket(struct addrinfo* serverAddr);
int sendRequest(int socketDesc, struct RequestPacketDef* request);
int receiveResponse(int socketDesc);
int readStatus(FILE* stream);
int readFiles(FILE* stream);
int readFile(FILE* stream);

#endif /* CLIENTUTILS_H_ */
/*
 * --------------------------------------------------------------- defines --
 */
#define BUFFER_STATUS 100
#define BUFFER_FILENAME 256
#define BUFFER_FILENAME 256
#define BUFFER_FILECHUNK 200

#define LOG_ERROR 1
#define LOG_INFO 3
