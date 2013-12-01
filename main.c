#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>

/* Function to calculate time difference */
double timeDifference(struct timeval * start_time, struct timeval * end_time) {
	return ((end_time->tv_sec - start_time->tv_sec)*1000 + (end_time->tv_usec - start_time->tv_usec)/1000);
}

int main (int argc, char *argv[]) {
    
	/* Checking if user provided us with 5 arguments */
	if (argc < 2) {
		printf("usage:\n", NULL);
		printf("./tptest destination(IP) port(1-65535) protocol(tcp=6/udp=17) length(packet length)\n", NULL);
		printf("example: ./tptest 192.168.1.2 80 6 512\n", NULL);
	}
    
	/* Putting arguments into more readable variable names */
	struct hostent *hp;
	hp = gethostbyname(argv[1]);
	const int PORT = atoi(argv[2]);
	const int PROTOCOL = atoi(argv[3]);
	const int LENGTH = atoi(argv[4]);
    
	/* Variables needed to check time difference */
	int returnTime;
	struct timeval start_t, end_t;
	struct timezone tzp;
    
	int socketDescr;			// Socket descriptor
	struct sockaddr_in server;	// Socket info about the remote server
	char buffer[LENGTH]; 		// TCP only
    
	memset(&server, 0, sizeof(server));					// Initializing (zeroing) the struct TCP ONLY
	server.sin_family = AF_INET;						// Address family, we use AF_INET (TCP/IP)
	server.sin_port = htons(PORT);						// Set the destination port
	bcopy(hp->h_addr,&(server.sin_addr),hp->h_length);	// Setting server's IP address
	
	/* Checking if we are going to use TCP or UDP */
	if (PROTOCOL == 17) { // TCP
		socketDescr = socket(AF_INET,SOCK_DGRAM,0);  	// Creating the socket, SOCK_DGRAM = UDP
        
		/* Sending datagrams */
		int i;
		for(i=0;i<100;i++) {
			sendto(socketDescr, "HELLO", 1024, 0, (struct sockaddr *) &server, sizeof(server));
		}
        
	} else if (PROTOCOL == 6) { // TCP
		socketDescr = socket(AF_INET,SOCK_STREAM,0);	// Creating the socket, SOCK_STREAM = TCP/
		
		connect(socketDescr, (struct sockaddr *)&server, sizeof(struct sockaddr)); // Establishing TCP connection
        
		/* Variables to used to check the amount of data */
		long int mega = 1024*1024;
		long int dataToGet = mega*100;
		long int received = 0;
		int tmp = 0;
		
		/* Receiving TCP packet stream from remote server */
		printf("Receiving %d byte packets, waiting to get 100MB.\n", LENGTH);
		returnTime = gettimeofday(&start_t, &tzp); 	// Check time when starting
		while (received < dataToGet) {
			tmp = recv(socketDescr, buffer, LENGTH, 0);
			received = received + tmp;
		}
		returnTime = gettimeofday(&end_t, &tzp);	// Check time when finished
        
		double totalTime = timeDifference(&start_t, &end_t)/1000;	// Get time difference in milliseconds, make into seconds
		double mbits = ((received/mega)*8) / totalTime;				// Calculating speed in Mbit/s
		printf("Received %ld MB in %f seconds - %lf Mbit/s\n", received/mega, totalTime, mbits); //Printing results
        
	} else {
		printf("Invalid protocol specified!\n", NULL);
		exit(0);
	}
    
	close(socketDescr);	/* closing the socket */
    
	return 0;
}
