#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <ctype.h>
#include <errno.h>
// this is encryption client program, this takes plain text and send it to encryption server 
//and recive encrypted message back from server

void error(const char *msg) { errno = 1; perror(msg);exit(1); } 

int fileLength(const char* filename){
	int character;
	int count = 0;
	//open the file for reading
	FILE* file = fopen(filename, "r");

	//get the first character of the file
	character = fgetc(file);

	//keep reading until we reach the end of the file, either a null terminator or an endline.
    	while (!(character == EOF || character == '\n')) 
	{
		//if the character isn't valid, call the error function. 
		if(!isupper(character) && character != ' ')
			error("File contains bad characters!");
		
		//get the next character
		character = fgetc(file);

		//increase the count.
        	++count;
    	}

	//close the file.
	fclose(file);
	//return the final count.
	return count;
}


int main(int argc, char *argv[])
{
	// initialize server address, buffer to hold text and cipher text
	int socketFD, portNumber, charsWritten, charsRead, bytesread;
	struct sockaddr_in serverAddress;
	struct hostent* serverHostInfo;
	char buffer[1024];
	char ciphertext[150000];

// check for the inputs, plain text and port number
	if (argc < 3) { fprintf(stderr, " %s as hostname port\n", argv[0]); exit(1); } 

	//initialize the server using port number recived from the input and make this to 
	// work as localhost to establish connection like client server between 
	//encryption client and server
	memset((char*)&serverAddress, '\0', sizeof(serverAddress)); 
	portNumber = atoi(argv[3]); 
	serverAddress.sin_family = AF_INET; 
	serverAddress.sin_port = htons(portNumber); 
	serverHostInfo = gethostbyname("localhost"); 
	if (serverHostInfo == NULL) { fprintf(stderr, " no such host\n"); exit(0); }
	memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length); 
	// create the socket and connect to server
	socketFD = socket(AF_INET, SOCK_STREAM, 0); 
	if (socketFD < 0) error("ERROR opening socket");
	int yes = 1;
	setsockopt(socketFD, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
	if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0){ // Connect socket to address
		error("ERROR connecting");
	}

	// check the text file size and key size if key length is smaller than plaintext
	// raise an error
	long filelength = fileLength(argv[1]);
	long keylength = fileLength(argv[2]);

	if(filelength > keylength)
		error("Key is  short!");
	
	//setup handshake to check connection
	char* msg = "encryption";
	charsWritten = send(socketFD, msg, strlen(msg), 0);
	memset(buffer, '\0', sizeof(buffer)); 
	charsWritten = 0;
	while(charsWritten == 0) 
		charsWritten = recv(socketFD, buffer, sizeof(buffer) - 1, 0);

	//check if the connection is between opposite client and server
	if(strcmp(buffer, "no") == 0) 
		error("enc_client cannot use dec_server");		
	
	memset(buffer, '\0', sizeof(buffer)); 
	sprintf(buffer, "%d", filelength);
	charsWritten = send(socketFD, buffer, sizeof(buffer), 0);
	memset(buffer, '\0', sizeof(buffer)); 
	int fd = open(argv[1], 'r');
	charsWritten = 0;
		
	// read the file and send data to server
	while(charsWritten <= filelength)
	{
		
		
		bytesread = read(fd, buffer, sizeof(buffer)-1);
		charsWritten += send(socketFD, buffer, strlen(buffer), 0);	
	}
	memset(buffer, '\0', sizeof(buffer));
	
	//read the key and send it to server
	fd = open(argv[2], 'r');
	charsWritten = 0;
	while(charsWritten <= filelength)
	{
		bytesread = read(fd, buffer, sizeof(buffer)-1);
		charsWritten += send(socketFD, buffer, strlen(buffer), 0);
		
	}
	
	memset(buffer, '\0', sizeof(buffer)); 
	charsWritten = 0;

// recive the cipher text from the server and add new line at the end of it, print this to standard output
	while(charsWritten < filelength)
	{	
		charsWritten += recv(socketFD, buffer, sizeof(buffer)-1, 0);
		strcat(ciphertext, buffer);
	}
	strcat(ciphertext, "\n");

	printf("%s", ciphertext);
	//close the socket
	close(socketFD); 
	return 0;
}

