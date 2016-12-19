/**
 * @file SimpleMessageClient.c
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
#include "simple_message_client_commandline_handling.h"
#include "SimpleMessageClient.h"
#include "ClientUtils.h"
/*
 * ------------------------------------------------------------- functions --
 */
/**
 *
 * \brief This Method prints the help page
 *
 * \param file the file stream
 * \param message error message
 * \param result value
 * \return void
 * \retval void
 *
 */
void parsingFailed(FILE* file, const char* message, int value)
{
	printHelp();
	exit(EXIT_HELPPAGE);
}
/**
 *
 * \brief Main function for the SimpleMessageClient
 *
 * This is the main entry point for any C program.
 *
 * \param argc the number of arguments
 * \param argv the arguments itselves (including the program name in argv[0])
 *
 * \return error code
 * \retval error code
 *
 */
int main(int argc, char **argv) {

	//Parse Parameter
	char *server, *port, *user, *message, *img_url;
	int verbose;

	smc_parsecommandline(argc, argv, &(parsingFailed) , &(server), &(port), &(user), &(message), &(img_url), &(verbose));
	initLog(verbose, argv[0]);

	//Resolve Target-Address
	struct addrinfo *serverAddr;
	int addrError;

	struct addrinfo hints;
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	/*hints.ai_flags = AI_PASSIVE;
	hints.ai_canonname = NULL;
	hints.ai_addr = NULL;
	hints.ai_next = NULL;*/

	//Get Endpoint-Address
	log(LOG_INFO, "Resolving server-address <%s>...\n", server);
	addrError = getaddrinfo(server, port, &hints, &(serverAddr));
	if(addrError != 0)
	{
		log(LOG_ERROR, "Failed to parse server-address: %s.\n", gai_strerror(addrError));
		return ERROR_SERVERADDR_WRONG;
	}

	int socketDesc;

	//Connecting to server
	log(LOG_INFO, "Connecting to server <%s:%s>...\n", server, port);
	if((socketDesc = connectSocket(serverAddr)) < 0)
	{
		log(LOG_ERROR, "Failed to connect to server <%s:%s>!\n", server, port);
		return ERROR_FAILED_TO_CONNECT;
	}

	//Sending request
	log(LOG_INFO, "Connection established! Sending request...\n");
	RequestPacket request;
	request.User = user;
	request.Image = img_url;
	request.Message = message;
	if(sendRequest(socketDesc, &request) < 0)
	{
		log(LOG_ERROR, "Failed to send request!\n");
		return ERROR_SENDING_REQUEST;
	}

	//Receiving response
	log(LOG_INFO, "Request sent! Receiving response...\n");
	if(receiveResponse(socketDesc) < 0)
	{
		log(LOG_ERROR, "Failed to receive response!\n");
		return ERROR_RECEIVING_RESPONSE;
	}

	//Closing and Terminating
	close(socketDesc);
	freeaddrinfo(serverAddr);

	log(LOG_INFO, "Client finished successfully!\n");
	return EXIT_SUCCESS;
}
