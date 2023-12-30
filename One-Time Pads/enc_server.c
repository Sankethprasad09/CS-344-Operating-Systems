

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>


// it is encryption server program

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


//encrypt a given message
void  encryption(char  plaintext[], char key[]){ 
	int i=0;
	char n;

	//for the length of the message
	while (plaintext[i] != '\n')
	{
			//get the character
	  		char c = plaintext[i];

			//convert the character and it's key match to integers.
			//add them and mod 27 to get the encrypted character.
	  		n = (charToInt(plaintext[i]) + charToInt(key[i])) % 27;
			
			//store the encrypted charcter in place of the original 
			//character
	  		plaintext[i] = intToChar(n);
			i++;
	}
	//overwrite the original newline with an endline
	plaintext[i] = '\0';
	return;
}

int main(int argc, char *argv[])
{
	// initialize all the information needed to store like client address,process id , port number

	int listenSocketFD, establishedConnectionFD, portNumber, charsWritten, status;
	socklen_t sizeOfClientInfo;
	struct sockaddr_in serverAddress, clientAddress;
	pid_t pid;

// check the input
	if (argc < 2) { fprintf(stderr, " %s as port\n", argv[0]); exit(1); } // Check usage & args

	// initialize the server address and assign the port number recieved from the input
	// create the socket using same details
	memset((char *)&serverAddress, '\0', sizeof(serverAddress)); 
	portNumber = atoi(argv[1]); 
	serverAddress.sin_family = AF_INET; 
	serverAddress.sin_port = htons(portNumber); 
	serverAddress.sin_addr.s_addr = INADDR_ANY; 

	//Create the socket and bind socket to port and enable socket to listening to port
	listenSocketFD = socket(AF_INET, SOCK_STREAM, 0);
	if (listenSocketFD < 0) error("ERROR opening socket");

	if (bind(listenSocketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to port
		error("ERROR on binding");

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
				error("Hull Breach! Couldn't fork!\n");
			}
			case 0:
			{
				//initialize the buffer to hold the encrypted message

				char buffer[1024];
				char* encryptedMessage[150000];
				char message[150000];
				char key[150000];
				memset(buffer, '\0', sizeof(buffer));
				charsWritten = 0;

				//read the client's handshake message
				while(charsWritten == 0)
					charsWritten = recv(establishedConnectionFD, buffer, sizeof(buffer)-1, 0);
			
				//using handshake, make sure we are connected to the correct server
				if(strcmp(buffer, "encryption") != 0)
				{
					charsWritten = send(establishedConnectionFD, "no", 2, 0);
					exit(2);
				}
				else
				{
					//set yes if connection is correct using message set above

					memset(buffer, '\0', sizeof(buffer));
					charsWritten = send(establishedConnectionFD, "yes", 3, 0);
					charsWritten = 0;

					while(charsWritten == 0)
						charsWritten = recv(establishedConnectionFD, buffer, sizeof(buffer)-1, 0);
					int size = atoi(buffer);
					charsWritten = 0;
					memset(buffer, '\0', sizeof(buffer));	

					//recieve the  text from the client
					while(charsWritten < size)
					{					
						//recieve the message
						charsWritten += recv(establishedConnectionFD, buffer, sizeof(buffer)-1, 0);
						
						//add the data to the message string
						strcat(message, buffer);
					}
					memset(buffer, '\0', sizeof(buffer));
					charsWritten = 0;
					
					
					while(charsWritten < size)
					{						
						charsWritten += recv(establishedConnectionFD, buffer, sizeof(buffer)-1, 0);
						strcat(key, buffer);
					}

					memset(buffer, '\0', sizeof(buffer));
		
					//encrypt the message using key
					encryption(message, key);
									
					//send the decoded plaintext back to the client
					charsWritten = 0;
					while(charsWritten < size)
						charsWritten += send(establishedConnectionFD, message, sizeof(message), 0);
					
					//exit
					exit(0);

				}
			}
			default:
			{
				pid_t actualpid = waitpid(pid, &status, WNOHANG);
			}
		}
		close(establishedConnectionFD);
	}
	close(listenSocketFD); 
	return 0;
}

