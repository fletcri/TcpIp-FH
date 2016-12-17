/*
 * ClientUtils.h
 *
 *  Created on: 13.12.2016
 *      Author: thomas
 */

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

#endif /* CLIENTUTILS_H_ */
