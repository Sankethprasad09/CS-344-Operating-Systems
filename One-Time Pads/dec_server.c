
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>


// it is decryption server program

void error(const char *msg) { errno = 1; perror(msg);exit(1); } 

int  charToInt(char c){

	//if c is a space
	if (c == ' '){
		return 26;
	}
	//therwise subtract the ascii of A from the character
	else {
		return (c - 'A');
	}
	return 0;
}


char intToChar(int i){

	//if the number is 26, return a space
	if (i == 26){
		return ' ';
	}
	//otherwise, return the integer plus ascii of A
	else 
	{
		return (i + 'A');
	}
}	

void  decryption(char cipher[], char key[])
{
	int i=0;
	char n;
	//for the length of the message
	while (cipher[i] != '\n')
	{
		//convert the message character and it's matching key character
		//subtract the key integer from the meessage integer
		n = charToInt(cipher[i]) - charToInt(key[i]);

		//if the result is negative, add 27
	  	if (n<0)
			n += 27;
	  	//convert the result to a character and store in place of the original
		//character.
	  	cipher[i] = intToChar(n);
		i++;
	  }
	  //add a null terminator to the end.
	  cipher[i] = '\0';
	  return;
}



int main(int argc, char *argv[])
{
	// initialize all the information needed to store like client address,process id , port number
	int listenSocketFD, establishedConnectionFD, portNumber, charsWritten, charsSent, status;
	socklen_t sizeOfClientInfo;
	struct sockaddr_in serverAddress, clientAddress;
	pid_t pid;

// check the input
	if (argc < 2) { fprintf(stderr, ": %s as port\n", argv[0]); exit(1); } 

	// initialize the server address and assign the port number recieved from the input
	// create the socket using same details
	memset((char *)&serverAddress, '\0', sizeof(serverAddress));
	portNumber = atoi(argv[1]); 
	serverAddress.sin_family = AF_INET; 
	serverAddress.sin_port = htons(portNumber); 
	serverAddress.sin_addr.s_addr = INADDR_ANY; 

	listenSocketFD = socket(AF_INET, SOCK_STREAM, 0);
	if (listenSocketFD < 0) error("cannot  open socket");

	// start binding socket to port and start lstening to the port
	if (bind(listenSocketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) 
		error("cannot bind socket to port");

	
	while(1)
	{
		// create pool of 5 connections , make availibity to connect to them  accept the connection 
		//if available
		listen(listenSocketFD, 5); 	
		sizeOfClientInfo = sizeof(clientAddress); 
		establishedConnectionFD = accept(listenSocketFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo); 
		if (establishedConnectionFD < 0) error("ERROR on accept");

		//create child process using fork
		pid = fork();
		switch (pid){

			
			case -1:
			{
				error("fork failed\n");
			}
			case 0:
			{
				//initialize the buffer to hold the text values
				char buffer[1024];
				char* encryptedMessage[150000];
				char message[150000];
				char key[150000];
				memset(buffer, '\0', sizeof(buffer));
				charsWritten = 0;

				//recieve the messages from client
				while(charsWritten == 0)
					charsWritten = recv(establishedConnectionFD, buffer, sizeof(buffer)-1, 0);
				
				if (charsWritten < 0) error("cannot read from socket");
				
				//using handshake, make sure we are connected to the correct server
				if(strcmp(buffer, "decryption") != 0)
				{
					charsWritten = send(establishedConnectionFD, "no", 2, 0);
					exit(2);
				}
				else
				{
					
					memset(buffer, '\0', sizeof(buffer));

					//set yes if connection is correct using message set above
					charsWritten = send(establishedConnectionFD, "yes", 3, 0);
					charsWritten = 0;

					
					while(charsWritten == 0)
						charsWritten = recv(establishedConnectionFD, buffer, sizeof(buffer)-1, 0);
							
					int size = atoi(buffer);
					
					charsWritten = 0;
					charsSent = 0;
					
					 
					// get the cipher text from client, add message  and buffer
					while(charsWritten < size)
					{
						
						charsSent += recv(establishedConnectionFD, buffer, sizeof(buffer)-1, 0);
						charsWritten += charsSent;
						charsSent = 0;
						strcat(message, buffer);
				
					}
					
					memset(buffer, '\0', sizeof(buffer));
					charsWritten = 0;
					charsSent = 0;

					
					// recieve the from client and store it
					while(charsWritten < size){
										
						charsSent = recv(establishedConnectionFD, buffer, sizeof(buffer)-1, 0);
						charsWritten += charsSent;
						charsSent = 0;
						strcat(key, buffer);
										
					}
					
					memset(buffer, '\0', sizeof(buffer));
					//decrypt the message using key 
					decryption(message, key);
					
					//send the decrypted plaintext back to the client
					//after producing plain text send it back to the client
					charsWritten = 0;
					while(charsWritten < size)
						charsWritten += send(establishedConnectionFD, message, sizeof(message), 0);
					
					exit(0);

				}
			}
			default:
			{
				pid_t actualpid = waitpid(pid, &status, WNOHANG);
			}
		}
		//close the socket connection
		close(establishedConnectionFD);
	}
	
	close(listenSocketFD); 
	return 0;
}


