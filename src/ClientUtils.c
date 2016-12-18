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

	if(request->User != NULL && strlen(request->User) > 0)
	{
		fprintf(stderr, "Sending Username...\n");
		if(fprintf(fw, "user=%s\n", request->User) < 0)
		{
			fprintf(stderr, "Failed to send user-line!\n");
			return -3;
		}
	}

	if(request->Image != NULL && strlen(request->Image) > 0)
	{
		fprintf(stderr, "Sending Image...\n");
		if(fprintf(fw, "img=%s\n", request->Image) < 0)
		{
			fprintf(stderr, "Failed to send image-line!\n");
			return -3;
		}
	}

	if(request->Message != NULL && strlen(request->Message) > 0)
	{
		fprintf(stderr, "Sending Message...\n");
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
		fprintf(stderr, "Failed to read status!\n");
		return -3;
	}

	int readResult = readFiles(fr);
	if(readResult < 0)
	{
		fprintf(stderr, "Failed to read files!\n");
		return -3;
	}
	else
	{
		fprintf(stderr, "Received <%i> files!\n", readResult);
	}

	fprintf(stderr, "Finished receiving response!\n");
	shutdown(sdr, SHUT_RD);
	fclose(fr);

	return 0;
}

int readStatus(FILE* stream)
{
	char statusBuffer[BUFFER_STATUS * sizeof(char)];
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

int readFiles(FILE* stream)
{
	int readResult;
	int fileCount = 0;

	while((readResult = readFile(stream)) > 0)
	{
		fileCount++;
	}

	if(readResult == 0)
		readResult = fileCount;

	return readResult;
}

int readFile(FILE* stream)
{
	char fileNameBuffer[BUFFER_FILENAME * sizeof(char)];

	if(fgets(fileNameBuffer, sizeof(fileNameBuffer), stream) == NULL)
	{
		fprintf(stderr, "No more FileNames available!\n");
		return 0;
	}

	char fileName[strlen(fileNameBuffer) - 5];

	if (*(fileNameBuffer + strlen(fileNameBuffer) - 1) == '\n')
	{
		*(fileNameBuffer + strlen(fileNameBuffer) - 1) = '\0';
	}

	if (strncmp("file=", fileNameBuffer, 5) != 0)
	{
		fprintf(stderr, "Protocoll-Error! Invalid file-field!\n");
		return -1;
	}

	strncpy(&fileName, (fileNameBuffer + 5), sizeof(fileName));

	char fileSizeBuffer[BUFFER_FILENAME * sizeof(char)];
	unsigned long fileLength;

	if(fgets(fileSizeBuffer, sizeof(fileSizeBuffer), stream) == NULL)
	{
		fprintf(stderr, "Failed to read FileSize!");
		return -2;
	}

	if (*(fileSizeBuffer + strlen(fileSizeBuffer) - 1) == '\n')
	{
		*(fileSizeBuffer + strlen(fileSizeBuffer) - 1) = '\0';
	}

	if (strncmp("len=", fileSizeBuffer, 4) != 0)
	{
		fprintf(stderr, "Protocoll-Error! Invalid len-field!\n");
		return -1;
	}

	char* parseEnd;
	fileLength = strtol(fileSizeBuffer + 4, &parseEnd, 10);
	if(fileLength <= 0)
	{
		fprintf(stderr, "Protocoll-Error! Invalid or zero FileLength!\n");
		return -1;
	}

	fprintf(stderr, "Receiving File <%s> with Length <%lu>....\n", fileName, fileLength);

	FILE* file;
	if((file = fopen(fileName, "w")) == NULL)
	{
		fprintf(stderr, "Failed to open File <%s>!\n", fileName);
		return -3;
	}

	unsigned long stillToRead = fileLength;

	while(stillToRead > 0)
	{
		char fileChunkBuffer[BUFFER_FILECHUNK * sizeof(char)];
		size_t readCount = (stillToRead < BUFFER_FILECHUNK) ? (size_t)stillToRead : BUFFER_FILECHUNK;

		if(fread(fileChunkBuffer, sizeof(char), readCount, stream) != readCount)
		{
			if(feof(stream))
			{
				fprintf(stderr, "EOF happened before receiving the full file!\n");
				return -3;
			}
			else
			{
				fprintf(stderr, "Failed to read file!\n");
				return -3;
			}
			fclose(file);
		}

		if(fwrite(fileChunkBuffer, sizeof(char), readCount, file) != readCount)
		{
			fprintf(stderr, "Failed to write data to file <%s>!\n", fileName);
			fclose(file);
			return -3;
		}

		stillToRead -= readCount;
	}

	fclose(file);
	fprintf(stderr, "Finished writing file <%s>!\n", fileName);

	return 1;
}
