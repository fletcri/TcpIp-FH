/*
 * ClientUtils.h
 *
 *  Created on: 13.12.2016
 *      Author: Fletzer Christoph
 */
#include <stdio.h>
#ifndef CLIENTUTILS_H_
#define CLIENTUTILS_H_

typedef struct RequestPacketDef
{
	char* User;
	char* Image;
	char* Message;
} RequestPacket;

void printHelp();
int connectSocket(struct addrinfo* serverAddr);
int sendRequest(int socketDesc, struct RequestPacketDef* request);
int receiveResponse(int socketDesc);
int readStatus(FILE* stream);

#endif /* CLIENTUTILS_H_ */

#define BUFFER_STATUS 100
