#include <stddef.h>
#include <stdio.h>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include "ClientUtils.h"

void printHelp()
{
	static char* helpLines[] =
	{
			"options:\n",
			"-s, --server \t <server> full qualified domain name or IP address of the server\n",
			"-p, --port \t <port> well-known port of the server [0..65535]\n",
			"-u, --user \t <name> name of the posting user\n",
			"-i, --image \t <URL> URL pointing to an image of the posting user\n",
			"-m, --message \t <message> message to be added to the bulletin board\n",
			"-v, --verbose \t verbose output\n",
			"-h, --help\n",
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
	struct addrinfo* info;
	int connectResult = -1;

	for (info = serverAddr; info != NULL; info = info->ai_next)
	{

		if((socketDesc = socket(serverAddr->ai_family, serverAddr->ai_socktype, serverAddr->ai_protocol)) < 0)
		{
			continue;
		}


		if((connectResult = connect(socketDesc, serverAddr->ai_addr, serverAddr->ai_addrlen)) == 0)
		{
			break;
		}
	}

	if(connectResult == -1)
		return connectResult;

	return socketDesc;
}

int sendRequest(int socketDesc, struct RequestPacketDef* request)
{
	int sdw;
	fprintf(stderr, "Dup SocketDescriptor...\n");
	if((sdw = dup(socketDesc)) < 0)
	{
		fprintf(stderr, "Failed to duplicate SocketDescriptor(write)!\n");
		return -1;
	}

	fprintf(stderr, "Opening FileDescriptor...\n");
	FILE* fw;
	if((fw = fdopen(sdw, "w")) == NULL)
	{
		fprintf(stderr, "Failed to open FileDescriptor(write)!\n");
		return -2;
	}

	fprintf(stderr, "Sending Username...\n");
	if(fprintf(fw, "user=%s\n", request->User) < 0)
	{
		fprintf(stderr, "Failed to send user-line!\n");
		return -3;
	}

	if(request->Image != NULL)
	{
		if(fprintf(fw, "img=%s\n", request->Image) < 0)
		{
			fprintf(stderr, "Failed to send image-line!\n");
			return -3;
		}
	}

	if(request->Message != NULL)
	{
		if(fprintf(fw, "%s", request->Message) < 0)
		{
			fprintf(stderr, "Failed to send message!\n");
			return -3;
		}
	}

	if(fflush(fw) == EOF)
	{
		fprintf(stderr, "Failed to flush write-stream!\n");
		return -3;
	}

	shutdown(sdw, SHUT_WR);
	fclose(fw);

	return 0;
}

int receiveResponse(int socketDesc)
{
	int sdr;

	if((sdr = dup(socketDesc)) < 0)
	{
		fprintf(stderr, "Failed to duplicate SocketDescriptor(read)!\n");
		return -1;
	}

	FILE* fr;
	if((fr = fdopen(sdr, "r")) == NULL)
	{
		fprintf(stderr, "Failed to open FileDescriptor(read)!\n");
		return -2;
	}

	if(readStatus(fr) < 0)
	{
		fprintf(stderr, "Failed to read status!");
		return -3;
	}



	fprintf(stderr, "Received status!\n");
	return 0;
}

int readStatus(FILE* stream)
{
	char statusBuffer[200];
	//statusBuffer = malloc(BUFFER_STATUS * sizeof(char));
	/*if(statusBuffer == NULL)
	{
		fprintf(stderr, "Failed to allocate statusBuffer!\n");
		return -3;
	}*/

	if(fgets(statusBuffer, sizeof(statusBuffer), stream) == NULL)
	{
		fprintf(stderr, "Failed to receive statusBuffer!\n");
		return -1;
	}

	if(strncmp("status=", statusBuffer, 7) != 0)
	{
		fprintf(stderr, "Protocoll-Error! Invalid status-field!\n");
		return -2;
	}

	if(*(statusBuffer + 7) == '0')
	{
		fprintf(stderr, "Status is <%c> OK!\n", *(statusBuffer + 7));
		return 1;
	}
	else
	{
		fprintf(stderr, "Status is <%c> WRONG!\n", *(statusBuffer + 7));
		return -3;
	}
}
