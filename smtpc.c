#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>

/*
 * Function: ssend
 *   sends to socket
 * Input parameters:
 *   expects socket, message to send
 * Output:
 *   returns 0 on success, exit 1 on failure
 */
int ssend(int* sockfd, char message[]) {
	char smessage[1000];
	sprintf(smessage, "%s\r\n", message);

	if(send(*sockfd, smessage, strlen(smessage), 0) < 0) {
		printf("Failed to send to the server.\n");
		exit(1);
	}
	return 0;
}

/*
 * Function: srecv
 *   recv from socket and check for expected status code
 * Input parameters:
 *   expects socket, expected status, debug flag
 * Output:
 *   returns 0 on success, exit 1 on failure
 */
int srecv(int* sockfd, int status, int debug) {
	char server_reply[2000];
	int recvStatus;
	memset(server_reply, 0x0, 2000);
	if(recv(*sockfd, server_reply, 2000, 0) < 0) {
		printf("Failed to recv from the server.\n");
		exit(1);
	}
	if(debug) {
		printf("Server: %s\n", server_reply);
	}
	sscanf(server_reply, "%d", &recvStatus);
	if(recvStatus != status) {
		printf("Unexpected status code. Exiting.\n");
		exit(1);
	}
	return 0;
}

/*
 * Function: main
 *   execution entrypoint
 * Input parameters:
 *   command line arguments
 * Output:
 *   returns 0 on success, 1 on failure
 */
int main(int argc, char **argv) {
	int sockfd;
	int debug = 0;
	unsigned int port = 25;
	char input[1000], finalmsg[1000];
	struct sockaddr_in server;
	struct hostent *hostptr;
	struct in_addr *ptr;

	/* runtime args */
	if (argc < 2 ) {
		printf("smtpc <host> <debug flag: 0 1>\n");
		return 1;
	}
	if (argc > 2) {
		debug = atoi(argv[2]);
	}
	
	/* get server address via dns */
	if ((hostptr = (struct hostent *) gethostbyname(argv[1])) == NULL) {
		printf("Failed to resolve hostname.\n");
	}

	/* connection settings */
	ptr = (struct in_addr *) *(hostptr->h_addr_list);
	memset((char *) &server, 0, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = ptr->s_addr;
	server.sin_port = htons(port);

	/* create socket */
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		printf("Failed to allocate socket.\n");
		return 1;
	}

	/* connect to remote */
	if (connect(sockfd, (struct sockaddr *)&server, sizeof(server)) < 0) {
		printf("Failed to connect to server.\n");
		return 1;
	}

	/* communicate with the server */
	srecv(&sockfd, 220, debug);

	ssend(&sockfd, "HELO loki");
	srecv(&sockfd, 250, debug);

	printf("Sender's email: ");
	scanf("%s", input);
	sprintf(finalmsg, "MAIL FROM: %s", input);
	ssend(&sockfd, finalmsg);
	srecv(&sockfd, 250, debug);

	printf("Receiver's email: ");
	scanf("%s", input);
	sprintf(finalmsg, "RCPT TO: %s", input);
	ssend(&sockfd, finalmsg);
	srecv(&sockfd, 250, debug);

	ssend(&sockfd, "DATA");
	srecv(&sockfd, 354, debug);

	printf("Subject: ");
	scanf("%s", input);
	sprintf(finalmsg, "Subject:%s", input);
	ssend(&sockfd, finalmsg);

	printf("Enter your message. Use \\n to end a line: ");
	scanf("%s", input);
	sprintf(finalmsg, "%s\r\n.", input);
	ssend(&sockfd, finalmsg);
	srecv(&sockfd, 250, debug);

	ssend(&sockfd, "QUIT");
	srecv(&sockfd, 221, debug);

	printf("User message was delivered successfully.\n");

	close(sockfd);
	return 0;
}
