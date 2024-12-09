#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>

#define SERV_TCP_PORT 23 /* server's port */
#define MAX_SIZE 80

static void* readSockFd(void *arg)
{
	int *sockfd = (int *) arg;

	for(;;)
	{
		char buf[MAX_SIZE];
		int len;

		len = read(*sockfd, buf, MAX_SIZE);

		if (len <= 0)
		{
			perror("The server shut down");
			exit(0);
		}
		else{
			buf[len] = 0;
            printf("\x1b[2K"); // Clear entire line
            printf("\r"); // Move the cursor to the beginning of the line
			printf("%s", buf);
            printf(" You: ");
            fflush(stdout);
		}
	}
}

int main(int argc, char *argv[])
{
    int sockfd, returnsockfd, newsockfd;
    char *serv_host = "localhost";
    struct hostent *host_ptr;
    int port, len, s;
    int buff_size = 0;
    int opt = 1;
	char string[MAX_SIZE];
    struct sockaddr_in cli_addr, serv_addr;
	pthread_t thread1;

    /* command line: client [host [port]]*/

	//1. get host
	if(argc >= 2)
        serv_host = argv[1]; /* read the host if provided */

	printf("selected host: %s \n", serv_host);

	//2. get port
    if(argc == 3)
        sscanf(argv[2], "%d", &port); /* read the port if provided */
    else
        port = SERV_TCP_PORT;

	printf("Selected port: %d \n", port);

    // 3. get the address of the host
    if((host_ptr = gethostbyname(serv_host)) == NULL)
    {
        perror("gethostbyname error");
        exit(1);
    }

    if(host_ptr->h_addrtype !=  AF_INET)
    {
        perror("unknown address type");
        exit(1);
    }

	//4. bind server address
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = ((struct in_addr *)host_ptr->h_addr_list[0])->s_addr;
    serv_addr.sin_port = htons(port);

    //5. open a TCP socket
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("can't open stream socket");
        exit(1);
    }

    //6. connect to the server
    //arg1: sockfd
    //arg2: server address
    //arg3: length
    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("can't connect to server");
        exit(1);
    }


	//separate thread waits for messages from the client socket
	//allocate memory
	printf("attempting to create thread: %d\n", sockfd);

	int *arg = malloc(sizeof(*arg));
	if ( arg == NULL ) {
		fprintf(stderr, "Couldn't allocate memory for thread arg.\n");
		exit(EXIT_FAILURE);
	}
	*arg = sockfd;



    // first message sent to server is name
    char name[MAX_SIZE/4];
    printf(" enter name: ");
    fgets(name, sizeof(name), stdin);
    write(sockfd, name, sizeof(name));

    //create thread
	s = pthread_create(&thread1, NULL, readSockFd, arg);
	if (s != 0)
	{
		perror("Thread create error");
	}
    //writing messages to server
    printf("input !close! to close!\n");
	for(;;)
	{
		char msg[MAX_SIZE];
		printf(" You: ");
		fgets(msg, sizeof(msg), stdin);
		if(strcmp(msg,"!close!\n")==0){
            close(sockfd);
            return 0;
		}
		else{
		write(sockfd, msg, sizeof(msg));
        }
	}
}
