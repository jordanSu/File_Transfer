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
void tcp_client(int, char*, char*, int);
void tcp_server(int, int);
void udp_client(int, char*, char*, int);
void udp_server(int, int);
void requestCheck(char* argv[]);

void requestCheck(char* argv[]) {
	int socketfd, portno;
	if (strcmp(argv[1],"get") == 0) {		//if it is a server
		printf("Server Selected!\n");
		portno = atoi(argv[3]);		//format portno and assign it
		if (strcmp(argv[2],"TCP") == 0) {
			socketfd = socket(AF_INET,SOCK_STREAM,0);	//create socket and assign to socketfd with TCP protocol
			tcp_server(socketfd, portno);
		}
		else if (strcmp(argv[2],"UDP") == 0) {
			socketfd = socket(AF_INET,SOCK_DGRAM,0);
			udp_server(socketfd, portno);
		}
	}
	else if (strcmp(argv[1],"send") == 0) {		//if it is a client
		portno = atoi(argv[5]);
		printf("Client Selected!\n");
		if (strcmp(argv[4],"TCP") == 0) {
			socketfd = socket(AF_INET,SOCK_STREAM,0);
			tcp_client(socketfd, argv[2], argv[3], portno);
		}
		else if (strcmp(argv[4],"UDP") == 0) {
			socketfd = socket(AF_INET,SOCK_DGRAM,0);	//create socket and assign to socketfd with UDP protocol
			udp_client(socketfd, argv[2], argv[3], portno);
		}
	}
}

int main(int argc, char* argv[]) {
	requestCheck(argv);
}


