#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<strings.h>
#include<netdb.h>
//http://www.linuxhowtos.org/C_C++/socket.htm
/* Usage:
 *  *** For client ***
 *	./lab2   [send]   [FILE_NAME] [HOST_NAME] [TCP|UDP] PORT_NUM
 *	argv[0]  argv[1]    argv[2]     argv[3]	   argv[4]   argv[5]
 *
 *	*** For server ***
 *	./lab2 	  [get]    [TCP|UDP]   PORT_NUM
 *	argv[0]  argv[1]	argv[2]	    argv[3]	
 */
void client(int, char*, char*, int);
void server(int, int);

int eofFlag = 0;

struct message {
	int num;
	char text[256];
};

int main(int argc, char* argv[]) {
	int socketfd, portno;		//socket file descriptor
	if (strcmp(argv[1],"get") == 0) {
		if (strcmp(argv[2],"TCP") == 0)
			socketfd = socket(AF_INET,SOCK_STREAM,0);	//create socket and assign to socketfd with TCP protocol
		else if (strcmp(argv[2],"UDP") == 0)
			socketfd = socket(AF_INET,SOCK_DGRAM,0);	
		if (socketfd < 0)
			printf("socket creating error");
		portno = atoi(argv[3]);		//format portno and assign it
		printf("Server Selected!");
		server(socketfd, portno);
	}
	else if (strcmp(argv[1],"send") == 0) {
		if (strcmp(argv[4],"TCP") == 0)
			socketfd = socket(AF_INET,SOCK_STREAM,0);
		else if (strcmp(argv[4],"UDP") == 0)
			socketfd = socket(AF_INET,SOCK_DGRAM,0);	//create socket and assign to socketfd with UDP protocol
		if (socketfd < 0)
			printf("socket creating error");
		portno = atoi(argv[5]);
		printf("Client Selected!");
		client(socketfd, argv[2], argv[3], portno);
	}
}

void server(int socketfd, int portno){
	int clilen, newsocketfd;
	struct sockaddr_in serv_addr, cli_addr;
	char buffer[256];
	char filepath[256];
	FILE* receivedFile;
	struct message thisSocket;
	bzero((char*) &serv_addr, sizeof(serv_addr));	// fill serv_addr with zero
	serv_addr.sin_family = AF_INET;					//}
	serv_addr.sin_port = htons(portno);				//} setup serv_addr fields
	serv_addr.sin_addr.s_addr = INADDR_ANY;			//}
	if (bind(socketfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0)
		printf("Socket binding error!");
	listen(socketfd, 5);
	clilen = sizeof(cli_addr);
	newsocketfd = accept(socketfd, (struct sockaddr*) &cli_addr, &clilen);
	if (newsocketfd < 0)
		printf("Socket accepting error!");
	mkdir("received");
	chmod("./received",0755);
	bzero(buffer,256);
	bzero(filepath,256);
	strcat(filepath,"./received/");
	read(newsocketfd, buffer, 256);		//get file name
	strcat(filepath,buffer);
	bzero(buffer,256);
	printf("%s\n",filepath);

	receivedFile = fopen(filepath,"w");
	if (receivedFile == NULL)
		printf("Error creating FIle!");
	while (1) {
		if (read(newsocketfd, thisSocket, sizeof(thisSocket)) < 0)
			printf("Socket reading error!");
		if (thisSocket.text[0] == '\0')
			break; 
		fprintf(receivedFile,"%s", thisSocket.text);
		bzero(thisSocket.text, 256);
	}
}

void client(int socketfd, char* fileName, char* hostName, int portno){
	struct sockaddr_in serv_addr;
	struct hostent* server;
	char buffer[256];
	FILE* yourFile;
	struct message thisSocket;
	yourFile = fopen(fileName,"r");
	if (yourFile == NULL)
		printf("Error opening file!");
	server = gethostbyname(hostName);
	if (server == NULL)
		printf("Host not found!");
	bzero((char*) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	bcopy((char*) server->h_addr, (char*) &serv_addr.sin_addr.s_addr, server->h_length);
	serv_addr.sin_port = htons(portno);
	if (connect(socketfd, &serv_addr, sizeof(serv_addr)) < 0)
		printf("Socket connecting error!");
	bzero(buffer,256);
	strncpy(buffer,fileName,sizeof(fileName));
	printf("%s\n",buffer);
	write(socketfd, buffer, 256);
	while (1) {
		bzero(thisSocket.text, 256);
		fgets(thisSocket.text, 255, yourFile);
		if (write(socketfd, thisSocket, sizeof(thisSocket)) < 0)
			printf("Socket writting error!");
		if (thisSocket.text[0] == '\0')
			break;
	}
	fclose(yourFile);
	bzero(thisSocket.text, 256);
	write(socketfd, thisSocket, sizeof(thisSocket));	//send a empty socket means it is over
}
