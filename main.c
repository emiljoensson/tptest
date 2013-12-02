#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <sys/types.h>
#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netdb.h>
#include <string.h>
#include <assert.h>
#include <errno.h>

/* Configurable - how much data that should be sent (in MB) when testing throuhgput */
const int MEGABYTES = 100;

/* Do not change */
const long int MEGA = 1024*1024;
const long int DATA_TO_TRANASFER = MEGA*MEGABYTES;

// Global variables needed to check time difference */
int returnTime;
struct timeval start_t, end_t;
struct timezone tzp;

/* Function to calculate time difference */
double timeDifference(struct timeval * start_time, struct timeval * end_time) {
	return ((end_time->tv_sec - start_time->tv_sec)*1000 + (end_time->tv_usec - start_time->tv_usec)/1000);
}

/* Function to receive TCP streams */
void receiveTCP(int length, int socketDescr) {
    
    /* Variables to used to check the amount of data */
    long int received = 0;
    int tmp = 0;
    char buffer[length]; 		// TCP only???
    
    /* Receiving TCP packet stream from remote server */
    printf("Receiving %d byte packets, waiting to get 100MB.\n", length);
    returnTime = gettimeofday(&start_t, &tzp); 	// Check time when starting
    while (received < DATA_TO_TRANASFER) {
        tmp = recv(socketDescr, buffer, length, 0);
        received = received + tmp;
    }
    returnTime = gettimeofday(&end_t, &tzp);	// Check time when finished
    
    double totalTime = timeDifference(&start_t, &end_t) / 1000;	// Get time difference in milliseconds, make into seconds
    double mbits = ((received/MEGA)*8) / totalTime;				// Calculating speed in Mbit/s
    printf("Received %ld MB in %f seconds = %lf Mbit/s\n", received/MEGA, totalTime, mbits); //Printing results
    
}

void sendTCP(int length, int socketDescr) {
    socketDescr = socket(AF_INET,SOCK_STREAM,0);	// Creating the socket, SOCK_STREAM = TCP/
    
}

int main (int argc, char *argv[]) {
    
	/* Checking if user provided us with 5 arguments */
	if (argc < 2) {
		printf("usage:\n", NULL);
		printf("./tptest destination(IP) port(1-65535) protocol(tcp=6/udp=17) length(packet length) direction(1=up,2=down,3=both)\n", NULL);
		printf("example: ./tptest 192.168.1.2 80 6 512 3\n", NULL);
	}
    
	/* Putting arguments into more readable variable names */
	struct hostent *hp;
	hp = gethostbyname(argv[1]);
	const int PORT = atoi(argv[2]);
	const int PROTOCOL = atoi(argv[3]);
	const int LENGTH = atoi(argv[4]);
    const int DIRECTION = atoi(argv[5]);
    
    /* Creating the sockets */
	int socketDescr;			// Socket descriptor
	struct sockaddr_in server;	// Socket info about the remote server, sending data
    struct sockaddr_in local;   // Socket info about local server, receiving data
    
	memset(&server, 0, sizeof(server));					// Initializing (zeroing) the struct TCP ONLY
	server.sin_family = AF_INET;						// Address family, we use AF_INET (TCP/IP)
	server.sin_port = htons(PORT);						// Set the remote host port
	bcopy(hp->h_addr,&(server.sin_addr),hp->h_length);	// Set the remote host IP address
    
    local.sin_family = AF_INET;                         // Address family, we use AF_INET (TCP/IP)
    local.sin_addr.s_addr = htonl(INADDR_ANY);          // Set the local host port
    local.sin_port = htons(PORT);                       // Set the local host IP address
	
	/* Checking if we are going to use TCP or UDP */
	if (PROTOCOL == 17) { // if UDP
        
        if (DIRECTION == 2 || DIRECTION == 3) { // If downstream or both
            // receiving UDP packets goes here
        } else if (DIRECTION == 1 || DIRECTION == 3) { // If upstream or both
            // sending UDP packets goes here
        }
        
	} else if (PROTOCOL == 6) { // If TCP
        if (DIRECTION == 2 || DIRECTION == 3) { // If downstream or both
            socketDescr = socket(AF_INET,SOCK_STREAM,0);	// Creating the socket, SOCK_STREAM = TCP/
            connect(socketDescr, (struct sockaddr *)&server, sizeof(struct sockaddr)); // Establishing TCP connection
            receiveTCP(LENGTH, socketDescr);                // Calling receiveTCP function, see above
            close(socketDescr);                             // Closing the socket */
        } else if (DIRECTION == 1 || DIRECTION == 3) { // If upstream or both
            // sending TCP packets goes here
        }
        
	} else {
		printf("Invalid protocol specified!\n", NULL);
		exit(0);
	}
    
	return 0;
}
