#include <stddef.h>
#include <stdio.h>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>
#include "ClientUtils.h"


void printHelp()
{
	static char* helpLines[] =
	{
			"Line1\n",
			"Line2\n",
			NULL
	};

	int lineIndex = 0;
	while(helpLines[lineIndex] != NULL)
	{
		fprintf(stdout, helpLines[lineIndex]);
		lineIndex++;
	}
}

int connectSocket(struct addrinfo* serverAddr)
{
	int socketDesc = 0;

	if((socketDesc = socket(serverAddr->ai_family, serverAddr->ai_socktype, serverAddr->ai_protocol)) < 0)
	{
		fprintf(stderr, "Failed to create Socket-Descriptor!");
		return -1;
	}

	if(connect(socketDesc, serverAddr->ai_addr, serverAddr->ai_addrlen) < 0)
	{
		fprintf(stderr, "Failed to connect to Server!");
		return -2;
	}

	return socketDesc;
}

int sendRequest(int socketDesc, struct RequestPacketDef* request)
{
	int sdw;

	if((sdw = dup(socketDesc)) < 0)
	{
		fprintf(stderr, "Failed to duplicate SocketDescriptor!");
		return -1;
	}

	FILE* fw;
	if((fw = fdopen(sdw, "w")) == NULL)
	{
		fprintf(stderr, "Failed to open FileDescriptor(write)!");
		return -2;
	}

	if(fprintf(fw, "user=%s\n", request->User) < 0)
	{
		fprintf(stderr, "Failed to send user-line!");
		return -3;
	}

	if(request->Image != NULL)
	{
		if(fprintf(fw, "img=%s\n", request->Image) < 0)
		{
			fprintf(stderr, "Failed to send image-line!");
			return -3;
		}
	}

	if(request->Message != NULL)
	{
		if(fprintf(fw, "%s", request->Message) < 0)
		{
			fprintf(stderr, "Failed to send message!");
			return -3;
		}
	}

	if(fflush(fw) == EOF)
	{
		fprintf(stderr, "Failed to flush write-stream!");
		return -3;
	}

	shutdown(fw, SHUT_WR);
	fclose(fw);

	return 0;
}
