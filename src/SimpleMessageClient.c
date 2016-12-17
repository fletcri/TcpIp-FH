/*
 ============================================================================
 Name        : SimpleMessageClient.c
 Author      : Fletzer Christoph
 Version     :
 Copyright   : 
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include "simple_message_client_commandline_handling.h"
#include "SimpleMessageClient.h"
#include "ClientUtils.h"

void parsingFailed(FILE* file, const char* message, int value)
{
	printHelp();
}

int main(int argc, char **argv) {

	//Parse Parameter
	char *server, *port, *user, *message, *img_url;
	int verbose;

	smc_parsecommandline(argc, argv, &(parsingFailed) , &(server), &(port), &(user), &(message), &(img_url), &(verbose));

	//Resolve Target-Address
	struct addrinfo *serverAddr;
	int addrError;

	struct addrinfo hints;
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;
	hints.ai_canonname = NULL;
	hints.ai_addr = NULL;
	hints.ai_next = NULL;

	addrError = getaddrinfo(server, port, &hints, &(serverAddr));
	if(addrError != 0)
	{
		fprintf(stderr, "Failed to parse server-address: %s.\n", gai_strerror(addrError));
		return ERROR_SERVERADDR_WRONG;
	}

	int socketDesc;

	if((socketDesc = connectSocket(serverAddr)) < 0)
	{
		return ERROR_FAILED_TO_CONNECT;
	}

	RequestPacket* request;
	request->User = user;
	request->Image = img_url;
	request->Message = message;
	if(sendRequest(socketDesc, request) < 0)
	{
		return ERROR_SENDING_REQUEST;
	}

	fprintf(stdout, "Finished successfully!");
	return EXIT_SUCCESS;
}
