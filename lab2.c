#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<time.h>
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
void client(int, char*, char*, int, char*);
void server(int, int, char*);
void requestCheck(char* argv[]);

int EOFlag = 0;

struct message {
	int num;
	char text[1024];
};

int main(int argc, char* argv[]) {
	requestCheck(argv);
}

void requestCheck(char* argv[]) {
	int socketfd, portno;
	if (strcmp(argv[1],"get") == 0) {		//if it is a server
		if (strcmp(argv[2],"TCP") == 0)
			socketfd = socket(AF_INET,SOCK_STREAM,0);	//create socket and assign to socketfd with TCP protocol
		else if (strcmp(argv[2],"UDP") == 0)
			socketfd = socket(AF_INET,SOCK_DGRAM,0);	
		if (socketfd < 0)
			printf("socket creating error");
		portno = atoi(argv[3]);		//format portno and assign it
		printf("Server Selected!\n");
		server(socketfd, portno, argv[2]);
	}
	else if (strcmp(argv[1],"send") == 0) {		//if it is a client
		if (strcmp(argv[4],"TCP") == 0)
			socketfd = socket(AF_INET,SOCK_STREAM,0);
		else if (strcmp(argv[4],"UDP") == 0)
			socketfd = socket(AF_INET,SOCK_DGRAM,0);	//create socket and assign to socketfd with UDP protocol
		if (socketfd < 0)
			printf("socket creating error\n");
		portno = atoi(argv[5]);
		printf("Client Selected!\n");
		client(socketfd, argv[2], argv[3], portno, argv[4]);
	}
}

void server(int socketfd, int portno, char* TCP_UDP){
	int clilen, newsocketfd;
	struct sockaddr_in serv_addr, cli_addr;
	char buffer[1028];
	char filepath[1100];
	FILE* receivedFile;
	struct message thisSocket;
	
	//some socket file descriptor to defined
	bzero((char*) &serv_addr, sizeof(serv_addr));	// fill serv_addr with zero
	serv_addr.sin_family = AF_INET;					//}
	serv_addr.sin_port = htons(portno);				//} setup serv_addr fields
	serv_addr.sin_addr.s_addr = INADDR_ANY;			//}
	if (bind(socketfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0)
		printf("Socket binding error!\n");
	
	//listen
	listen(socketfd, 5);
	
	//accept
	clilen = sizeof(cli_addr);
	newsocketfd = accept(socketfd, (struct sockaddr*) &cli_addr, &clilen);	//accept connecting
	if (newsocketfd < 0)
		printf("Socket accepting error!\n");
		
	//make directory for received file
	mkdir("received");
	chmod("./received",0755);
	
	//to read file name
	bzero(buffer,1028);
	bzero(filepath,1100);
	strcat(filepath,"./received/");
	read(newsocketfd, buffer, 1028);
	strcat(filepath,buffer);
	bzero(buffer,1028);
	printf("Received file location: %s\n",filepath);

	//open new file and ready for catching data
	receivedFile = fopen(filepath,"w");
	if (receivedFile == NULL)
		printf("Error creating File!\n");
		
	while (1) {
		if (read(newsocketfd, buffer, sizeof(thisSocket)) < 0)
			printf("Socket reading error!\n");
		memcpy(&thisSocket, buffer, sizeof(thisSocket));
		if (thisSocket.text[0] == '\0')
			break; 
		fprintf(receivedFile,"%s", thisSocket.text);
		printf("Socket %d received!\n",thisSocket.num);
		bzero(thisSocket.text, 1024);
		bzero(buffer,1028);
	}
	fclose(receivedFile);
}

void client(int socketfd, char* fileName, char* hostName, int portno, char* TCP_UDP){
	struct sockaddr_in serv_addr;
	struct hostent* server;
	char buffer[1024];
	FILE* yourFile;
	struct message thisSocket;
	char* data = (unsigned char*)malloc(sizeof(thisSocket));	//data is the serialized data to send
	
	//open selected file
	yourFile = fopen(fileName,"r");	
	printf("FileName is %s",fileName);
	if (yourFile == NULL)
		printf("Error opening file!\n");
	
	//analyze host name	
	server = gethostbyname(hostName);
	if (server == NULL)
		printf("Host not found!\n");
	
	//some socket file descriptor to defined
	bzero((char*) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	bcopy((char*) server->h_addr, (char*) &serv_addr.sin_addr.s_addr, server->h_length);
	serv_addr.sin_port = htons(portno);
	if (connect(socketfd, &serv_addr, sizeof(serv_addr)) < 0)
		printf("Socket connecting error!\n");
		
	//tell server what is the file name 
	bzero(buffer,1024);
	strncpy(buffer,fileName,strlen(fileName));
	printf("send filename: %s\n",buffer);
	write(socketfd, buffer, 1024);
	bzero(buffer,1024);
	bzero(thisSocket.text,1024);
	//start to send data
	int count = 1;
	while (1) {
		if (fgets(buffer, 1024, yourFile) == NULL)
			EOFlag = 1;
		if (strlen(buffer) + strlen(thisSocket.text) >= 1024 || EOFlag ==1) {
			//printf("%s",thisSocket.text);
			thisSocket.num = count;
			count += 1;
			memcpy(data, &thisSocket, sizeof(thisSocket));
			if (write(socketfd, data, sizeof(thisSocket)) < 0)
				printf("Socket writting error!\n");
			bzero(thisSocket.text, 1024);
			if (EOFlag == 1)
				break;
		}
		strcat(thisSocket.text,buffer);
	}
	
	fclose(yourFile);
	bzero(thisSocket.text, 1024);
	memcpy(data, &thisSocket, sizeof(thisSocket));
	write(socketfd, data, sizeof(thisSocket));	//send a empty socket means it is over
}
