#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<time.h>
#include<netinet/in.h>
#include<strings.h>
#include<netdb.h>
#include"SO_contents.h"
int EOFlag = 0;

void tcp_server(int socketfd, int portno){
	int clilen, newsocketfd;
	struct sockaddr_in serv_addr, cli_addr;
	char test[1];
	test[0] = 'a';
	char buffer[1028];
	char filepath[1100];
	FILE* receivedFile;
	time_t now;
	clock_t throughput;
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
	
	//test for throughput
	//sleep(2);
	throughput = clock();
	write(newsocketfd, test, 1);
	read(newsocketfd, test, 1);
	throughput = clock() - throughput;
	printf("It used %f seconds\n", throughput);

	//open new file and ready for catching data
	receivedFile = fopen(filepath,"w");
	if (receivedFile == NULL)
		printf("Error creating File!\n");
	
	now = time(0);
	printf("File receive started at: %s",ctime(&now));
	while (1) {
		if (read(newsocketfd, buffer, sizeof(thisSocket)) < 0)
			printf("Socket reading error!\n");
		memcpy(&thisSocket, buffer, sizeof(thisSocket));
		if (thisSocket.text[0] == '\0')
			break; 
		fprintf(receivedFile,"%s", thisSocket.text);
		//printf("Socket %d received!\n",thisSocket.num);
		bzero(thisSocket.text, 1024);
		bzero(buffer,1028);
	}
	now = time(0);
	printf("File receive end at: %s",ctime(&now));
	fclose(receivedFile);
}

void tcp_client(int socketfd, char* fileName, char* hostName, int portno){
	struct sockaddr_in serv_addr;
	struct hostent* server;
	char test[1];
	test[0] = 'a';
	char buffer[1024];
	FILE* yourFile;
	struct message thisSocket;
	time_t now;
	clock_t throughput;
	char* data = (unsigned char*)malloc(sizeof(thisSocket));	//data is the serialized data to send
	
	//open selected file
	yourFile = fopen(fileName,"r");	
	printf("FileName is %s\n",fileName);
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
	write(socketfd, buffer, 1024);
	bzero(buffer,1024);
	bzero(thisSocket.text,1024);
	
	//test for throughput
	//sleep(1);
	throughput = clock();
	read(socketfd, test, 1);
	write(socketfd, test, 1);
	throughput = clock() - throughput;
	printf("It used %f seconds\n", throughput);
	
	//setup socket timeout
	struct timeval timeout;
	timeout.tv_sec = 2;
	timeout.tv_usec = 0;
	if (setsockopt(socketfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0)
		printf("set socket option error!\n");
	
	//start to send data
	now = time(0);
	printf("Transmit started at: %s",ctime(&now));
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
	
	bzero(thisSocket.text, 1024);
	memcpy(data, &thisSocket, sizeof(thisSocket));
	write(socketfd, data, sizeof(thisSocket));	//send a empty socket means it is over
	now = time(0);
	printf("Transmit completed at: %s",ctime(&now));
	fclose(yourFile);
}
