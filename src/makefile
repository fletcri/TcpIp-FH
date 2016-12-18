LDFLAGS += -L/usr/local/lib

all: SimpleMessageClient

clean:
	rm SimpleMessageClient SimpleMessageClient.o ClientUtils.o simple_message_client_commandline_handling.o

SimpleMessageClient: SimpleMessageClient.o simple_message_client_commandline_handling.o ClientUtils.o
	gcc -g -o SimpleMessageClient SimpleMessageClient.o simple_message_client_commandline_handling.o ClientUtils.o

SimpleMessageClient.o: SimpleMessageClient.c SimpleMessageClient.h
	gcc -c -g SimpleMessageClient.c SimpleMessageClient.h
	
ClientUtils.o: ClientUtils.c ClientUtils.h 
	gcc -c -g ClientUtils.c ClientUtils.h 

simple_message_client_commandline_handling.o: simple_message_client_commandline_handling.c simple_message_client_commandline_handling.h
	gcc -c -g simple_message_client_commandline_handling.c simple_message_client_commandline_handling.h
	