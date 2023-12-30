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



// it is decryption client program

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
	int socketFD, portNumber, charsWritten, charsRead, bytesread;
	struct sockaddr_in serverAddress;
	struct hostent* serverHostInfo;
	char buffer[1024];
	char plainText[150000];

    // check for the plain textfile and port number
	if (argc < 3) { fprintf(stderr, " %s as hostname port\n", argv[0]); exit(1); } 

	//initiate the server, get the port number from the argv, set the server address , make the machine
	// work as client-server archetecture by making it as local host
	memset((char*)&serverAddress, '\0', sizeof(serverAddress)); 
	portNumber = atoi(argv[3]); 
	serverAddress.sin_family = AF_INET; 
	serverAddress.sin_port = htons(portNumber); 
	serverHostInfo = gethostbyname("localhost"); 
	if (serverHostInfo == NULL) { fprintf(stderr, "CLIENT: ERROR, no such host\n"); exit(0); }
	memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length); 

	//create the socket, using setsockoptn make socket as reusable
	socketFD = socket(AF_INET, SOCK_STREAM, 0); 
	if (socketFD < 0) error("CLIENT: ERROR opening socket");
	int yes = 1;
	setsockopt(socketFD, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)); 

	// establish connection to server through socket, if fails raise error
	if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0){ 
		error("ERROR connecting");
	}

	// get the size of the textfile and the key, if the key is less than text file raise an error 
	//like key is short
	
	long filelength = fileLength(argv[1]);
	long keylength = fileLength(argv[2]);
	if(filelength > keylength)
		error("Key is too short!");
	
	//to avoid client server connecting to wrong ports use handshake
	char* msg = "decryption";
	charsWritten = send(socketFD, msg, strlen(msg), 0);

	
	memset(buffer, '\0', sizeof(buffer)); 
	charsWritten = 0;
	while(charsWritten == 0) 
		charsWritten = recv(socketFD, buffer, sizeof(buffer) - 1, 0);

	//check wether the connection is correct or not, if there is any connection raise an error
	// decprytion client should only connect t decryption server
	if(strcmp(buffer, "no") == 0) 
		error("dec_client cannot use enc_server");		
	
	memset(buffer, '\0', sizeof(buffer)); 
	sprintf(buffer, "%d", filelength);
	charsWritten = send(socketFD, buffer, sizeof(buffer), 0);
	memset(buffer, '\0', sizeof(buffer)); 
	
	//read the textfile to buffer and send it to server
	int fd = open(argv[1], 'r');
	charsWritten = 0;
	while(charsWritten <= filelength)
	{
		bytesread = read(fd, buffer, sizeof(buffer)-1);
		charsWritten += send(socketFD, buffer, strlen(buffer), 0);	
	}
	memset(buffer, '\0', sizeof(buffer));
	
	//read the key file and send it to server using buffer
	fd = open(argv[2], 'r');
	charsWritten = 0;
	while(charsWritten <= filelength)
	{
		bytesread = read(fd, buffer, sizeof(buffer)-1);
		charsWritten += send(socketFD, buffer, strlen(buffer), 0);
		
	}
	memset(buffer, '\0', sizeof(buffer)); 


// get the cipher from the server and add new line at the end of it
// write cipher to the standard output
	charsWritten = 0;
	while(charsWritten < filelength)
	{	
		charsWritten += recv(socketFD, buffer, sizeof(buffer)-1, 0);
		strcat(plainText, buffer);
	}
	strcat(plainText, "\n");

	printf("%s", plainText);
	//close the socket connection
	close(socketFD); 
	return 0;
}


