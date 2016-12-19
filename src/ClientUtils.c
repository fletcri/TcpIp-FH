/**
 * @file ClientUtils.c
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
#include <stddef.h>
#include <stdio.h>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include "ClientUtils.h"
#include <stdarg.h>
#include <libgen.h>
/*
 * --------------------------------------------------------------- globals --
 */
static char* programName;
static int verboseMode = 0;
/*
 * ------------------------------------------------------------- functions --
 */
/**
 *
 * \brief Initializes the logger
 *
 * \param verbose Verbose-Mode
 * \param progName The Program-Name
 *
 * \return void
 * \retval void
 *
 */
void initLog(int verbose, char* progName)
{
	verboseMode = verbose;
	programName = basename(progName);

	log(3, "Verbose-Mode is activated!\n");
}
/**
 *
 * \brief Logs messages to the console
 *
 * \param logLevel The loglevel used
 * \param logMessage The Format-Message to log out
 *
 * \return void
 * \retval void
 *
 */
void log(int logLevel, char* logMessage, ...)
{
	 va_list args;
	 va_start(args, logMessage);
	 if(logLevel == 1)
	 {
		 fprintf(stderr, "[%s] - LogLevel <%i>: ", programName, logLevel);
		 vfprintf(stderr, logMessage, args);
	 }
	 else if (verboseMode == 1)
	 {
		 fprintf(stdout, "[%s] - LogLevel <%i>: ", programName, logLevel);
		 vfprintf(stdout, logMessage, args);
	 }
	 va_end(args);
}
/**
 *
 * \brief Prints the Help-Page
 *
 * \return void
 * \retval void
 *
 */
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
		log(stdout, helpLines[lineIndex]);
		lineIndex++;
	}
}
/**
 *
 * \brief Connects to the server
 *
 * \param serverAddr The Server-Address
 *
 * \return int
 * \retval error-codes
 *
 */
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
/**
 *
 * \brief Send the request to the server
 *
 * \param socketDesc The Socket-Descriptor used
 * \param request The request packet
 *
 * \return int
 * \retval error-codes
 *
 */
int sendRequest(int socketDesc, struct RequestPacketDef* request)
{
	int sdw;
	log(LOG_INFO, "Dup SocketDescriptor...\n");
	if((sdw = dup(socketDesc)) < 0)
	{
		log(LOG_ERROR, "Failed to duplicate SocketDescriptor(write)!\n");
		return -1;
	}

	log(LOG_INFO, "Opening FileDescriptor...\n");
	FILE* fw;
	if((fw = fdopen(sdw, "w")) == NULL)
	{
		log(LOG_ERROR, "Failed to open FileDescriptor(write)!\n");
		return -2;
	}

	if(request->User != NULL && strlen(request->User) > 0)
	{
		log(LOG_INFO, "Sending Username...\n");
		if(fprintf(fw, "user=%s\n", request->User) < 0)
		{
			log(LOG_ERROR, "Failed to send user-line!\n");
			return -3;
		}
	}

	if(request->Image != NULL && strlen(request->Image) > 0)
	{
		log(LOG_INFO, "Sending Image...\n");
		if(fprintf(fw, "img=%s\n", request->Image) < 0)
		{
			log(LOG_ERROR, "Failed to send image-line!\n");
			return -3;
		}
	}

	if(request->Message != NULL && strlen(request->Message) > 0)
	{
		log(LOG_INFO, "Sending Message...\n");
		if(fprintf(fw, "%s", request->Message) < 0)
		{
			log(LOG_ERROR, "Failed to send message!\n");
			return -3;
		}
	}

	if(fflush(fw) == EOF)
	{
		log(LOG_ERROR, "Failed to flush write-stream!\n");
		return -3;
	}

	shutdown(sdw, SHUT_WR);
	fclose(fw);

	return 0;
}

/**
 *
 * \brief Receives the response from the server
 *
 * \param socketDesc The Socket-Descriptor used
 *
 * \return int
 * \retval error-codes
 *
 */
int receiveResponse(int socketDesc)
{
	int sdr;

	if((sdr = dup(socketDesc)) < 0)
	{
		log(LOG_ERROR, "Failed to duplicate SocketDescriptor(read)!\n");
		return -1;
	}

	FILE* fr;
	if((fr = fdopen(sdr, "r")) == NULL)
	{
		log(LOG_ERROR, "Failed to open FileDescriptor(read)!\n");
		return -2;
	}

	if(readStatus(fr) < 0)
	{
		log(LOG_ERROR, "Failed to read status!\n");
		return -3;
	}

	int readResult = readFiles(fr);
	if(readResult < 0)
	{
		log(LOG_ERROR, "Failed to read files!\n");
		return -3;
	}
	else
	{
		log(LOG_INFO, "Received <%i> files!\n", readResult);
	}

	log(LOG_INFO, "Finished receiving response!\n");
	shutdown(sdr, SHUT_RD);
	fclose(fr);

	return 0;
}
/**
 *
 * \brief Receives the server status response
 *
 * \param stream The file-stream to the server
 *
 * \return int
 * \retval error-codes
 *
 */
int readStatus(FILE* stream)
{
	char statusBuffer[BUFFER_STATUS * sizeof(char)];
	//statusBuffer = malloc(BUFFER_STATUS * sizeof(char));
	/*if(statusBuffer == NULL)
	{
		log(stderr, "Failed to allocate statusBuffer!\n");
		return -3;
	}*/

	if(fgets(statusBuffer, sizeof(statusBuffer), stream) == NULL)
	{
		log(LOG_ERROR, "Failed to receive statusBuffer!\n");
		return -1;
	}

	if(strncmp("status=", statusBuffer, 7) != 0)
	{
		log(LOG_ERROR, "Protocoll-Error! Invalid status-field!\n");
		return -2;
	}

	if(*(statusBuffer + 7) == '0')
	{
		log(LOG_INFO, "Status is <%c> OK!\n", *(statusBuffer + 7));
		return 1;
	}
	else
	{
		log(LOG_ERROR, "Status is <%c> WRONG!\n", *(statusBuffer + 7));
		return -3;
	}
}
/**
 *
 * \brief Receives the files from the server
 *
 * \param stream The file-stream to the server
 *
 * \return int
 * \retval error-codes
 *
 */
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
/**
 *
 * \brief Receives the file from the server
 *
 * \param stream The file-stream to the server
 *
 * \return int
 * \retval error-codes
 *
 */
int readFile(FILE* stream)
{
	char fileNameBuffer[BUFFER_FILENAME * sizeof(char)];

	if(fgets(fileNameBuffer, sizeof(fileNameBuffer), stream) == NULL)
	{
		log(LOG_INFO, "No more FileNames available!\n");
		return 0;
	}

	char fileName[strlen(fileNameBuffer) - 5];

	if (*(fileNameBuffer + strlen(fileNameBuffer) - 1) == '\n')
	{
		*(fileNameBuffer + strlen(fileNameBuffer) - 1) = '\0';
	}

	if (strncmp("file=", fileNameBuffer, 5) != 0)
	{
		log(LOG_ERROR, "Protocoll-Error! Invalid file-field!\n");
		return -1;
	}

	/*if (strncmp("file=/dev/null", fileNameBuffer, 14) == 0)
	{
		log(LOG_ERROR, "BAD TEACHER! The server is running test-case TESTCASE_HUGE_FILE! That's not nice =(\n");
		memset(fileNameBuffer,0,sizeof(fileNameBuffer));
		strncpy(fileNameBuffer, "badboy", sizeof(6));
	}*/

	strncpy(&fileName, (fileNameBuffer + 5), sizeof(fileName));

	char fileSizeBuffer[BUFFER_FILENAME * sizeof(char)];
	unsigned long fileLength;

	if(fgets(fileSizeBuffer, sizeof(fileSizeBuffer), stream) == NULL)
	{
		log(LOG_ERROR, "Failed to read FileSize!");
		return -2;
	}

	if (*(fileSizeBuffer + strlen(fileSizeBuffer) - 1) == '\n')
	{
		*(fileSizeBuffer + strlen(fileSizeBuffer) - 1) = '\0';
	}

	if (strncmp("len=", fileSizeBuffer, 4) != 0)
	{
		log(LOG_ERROR, "Protocoll-Error! Invalid len-field!\n");
		return -1;
	}

	char* parseEnd;
	fileLength = strtol(fileSizeBuffer + 4, &parseEnd, 10);
	if(fileLength <= 0)
	{
		log(LOG_ERROR, "Protocoll-Error! Invalid or zero FileLength!\n");
		return -1;
	}

	log(LOG_INFO, "Receiving File <%s> with Length <%lu>....\n", fileName, fileLength);

	FILE* file;
	if((file = fopen(fileName, "w")) == NULL)
	{
		log(LOG_ERROR, "Failed to open File <%s>!\n", fileName);
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
				log(LOG_ERROR, "EOF happened before receiving the full file!\n");
				return -3;
			}
			else
			{
				log(LOG_ERROR, "Failed to read file!\n");
				return -3;
			}
			fclose(file);
		}

		if(fwrite(fileChunkBuffer, sizeof(char), readCount, file) != readCount)
		{
			log(LOG_ERROR, "Failed to write data to file <%s>!\n", fileName);
			fclose(file);
			return -3;
		}

		stillToRead -= readCount;
	}

	fclose(file);
	log(LOG_INFO, "Finished writing file <%s>!\n", fileName);

	return 1;
}
