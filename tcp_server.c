
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>

#define SERV_TCP_PORT 23 // server's port number
#define MAX_SIZE 80
//compile with -lpthread
//use sudo when running
int sockTable[10];

void writeAll(char msg[], int size, int ogsocket)
{
	printf("msg: %s\n", msg);
	for (int i = 4; i < size; i++)
	{

	    if(ogsocket!=i){
            write(i, msg, size);
	    }
	}
}

static void * readSockFd(void* arg)
{
	int *clisockfd = (int *) arg;
    bool unnamed =true;
	printf("now reading socket fd %d\n", *clisockfd);

	for (;;)
    {
        char string[MAX_SIZE];
        char msg[MAX_SIZE+(MAX_SIZE/4)+3];
        char name[MAX_SIZE/4];
		int len;

		//6. READ MESSAGE
        /* read message from client*/
        //read newsockfd into string
        len = read(*clisockfd, string, MAX_SIZE);

		//printf("len: %d\n", len);

		if(len <= 0)
		{
			printf("User %d has disconnected\n", *clisockfd);
			//sockTable[*clisockfd] = NULL;
			close(*clisockfd);
			pthread_exit(0);
		}
		if(unnamed){
         strncpy(name, string, strlen(string)-1);
         unnamed=false;
		}
		else
		{
			/* make sure its a proper string */
			string[len] = 0;

            sprintf(msg, "%s: ", name);

			strcat(msg,string);
			printf("%s\n", msg);


			//send message to other clients
			printf("sending message to other clients \n");
			writeAll(msg, sizeof(string),*clisockfd);
		}
    }
}

int main(int argc, char *argv[])
{
    int sockfd, newsockfd, clilen;
    struct sockaddr_in cli_addr, serv_addr;
    int port;
    char string[MAX_SIZE];
    int len;
    int opt = 1;

    /* command line: server [port_number]*/

    //1. CHOOSING PORT
    if(argc == 2)
        //read port is provided, then use the port number
        sscanf(argv[1], "%d", &port);
    else
        //if port is not provided, use port 23
        port = SERV_TCP_PORT;

	printf("Selected port: %d \n", port);


    //2. OPENING SOCKET
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 )
    {
        perror("can't open stream socket");
        exit(1);
    }

    // Set socket options
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(1);
    }

	printf("opened socket: %d \n" , sockfd);

    //3. BINDING ADDRESS
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(port);

    if(bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("can't bind local address");
        exit(1);
    }

    //4. LISTEN TO SOCKET
    listen(sockfd, 5);

	//clilen = sizeof(cli_addr);
	//newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);

	//5. ACCEPT NEW CONNECTIONS
	for(;;)
	{
		struct sockaddr_in cli_addr;
		int clisockfd, clilen, s;
		pthread_t thread1;

		//accept
		clilen = sizeof(cli_addr);
		clisockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
		sockTable[sockfd] = sockfd;

		printf("User %d has joined\n", clisockfd);

		//separate thread waits for messages from the client socket
		int *arg = malloc(sizeof(*arg));
        if ( arg == NULL ) {
            fprintf(stderr, "Couldn't allocate memory for thread arg.\n");
            exit(EXIT_FAILURE);
        }
		*arg = clisockfd;

		s = pthread_create(&thread1, NULL, readSockFd, arg);

		if (s != 0)
		{
			perror("Thread create error");
		}
	}
}
