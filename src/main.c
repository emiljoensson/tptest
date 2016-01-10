#include <stdio.h>          // Declarations used in most input/output
#include <stdlib.h>         // Defines several general purpose function
#include <string.h>         // Defines variable type and various functions for manipulating arrays of characters
#include <unistd.h>         // Needed to use getopt() etc to handle command line arguments better
#include <time.h>           // Needed when checking Last-Modified (ctime())
#include <sys/types.h>      // Definitions of a number of data types used in system calls
#include <sys/socket.h>     // Includes a number of definitions of structures needed for sockets
#include <sys/stat.h>       // stat() to obtain information about file
#include <netinet/in.h>     // Constants and structures needed for internet domain addresses
#include <netdb.h>
#include <sys/time.h>

#include "error.h"
#include "send_data.h"

#define KILO 1024
#define MEGA (KILO*KILO)
#define MEGABYTES 100; // Configurable - how much data that should be sent (in MB) when testing throuhgput

const long int DATA_TO_TRANSFER = MEGA*MEGABYTES; // Do not change

/* Global variables needed to check time difference */
int returnTime;
struct timeval start_t, end_t;
struct timezone tzp;

/* Function to calculate time difference */
double timeDifference(struct timeval * start_time, struct timeval * end_time) {
    return ((end_time->tv_sec - start_time->tv_sec)*1000 + (end_time->tv_usec - start_time->tv_usec)/1000);
}

int main (int argc, char *argv[]) {

    /* Checking if user provided us with 5 arguments */
    if (argc < 6) {
        printf("usage:\n");
        printf("./tptest destination(IP) port(1-65535) protocol(tcp=6/udp=17) length(packet length) direction(1=upstream,2=downstream)\n");
        printf("example: ./tptest 192.168.1.2 1025 6 512 2\n");
        exit(0);
    }

    /* Putting arguments into more readable variable names */
    struct hostent *hp;
    hp = gethostbyname(argv[1]);
    const int PORT = atoi(argv[2]);
    const int PROTOCOL = atoi(argv[3]);
    const int LENGTH = atoi(argv[4]);
    const int DIRECTION = atoi(argv[5]);

    if (PORT < 1 || PORT > 65535) {
        printf("Invalid port. Valid range is 1-65535.\n");
        exit(0);
    }

    if (LENGTH < 2 || LENGTH > 9198) {
        printf("Invalid packet length. Valid range is 2-9198.\n");
        exit(0);
    }

    if (DIRECTION < 1 || DIRECTION > 2) {
        printf("Invalid direction. Use 1 for upstream or 2 for downstream.\n");
        exit(0);
    }

    /* Creating the sockets */
    int socketDescr;            // Socket descriptor
    struct sockaddr_in server;  // Socket info about the remote server, sending data
    struct sockaddr_in local;   // Socket info about local server, receiving data
    int len = sizeof(struct sockaddr_in);

    memset(&server, 0, sizeof(server));                 // Initializing (zeroing) the struct TCP ONLY
    server.sin_family = AF_INET;                        // Address family, we use AF_INET (TCP/IP)
    server.sin_port = htons(PORT);                      // Set the remote host port
    bcopy(hp->h_addr,&(server.sin_addr),hp->h_length);  // Set the remote host IP address

    local.sin_family = AF_INET;                         // Address family, we use AF_INET (TCP/IP)
    local.sin_addr.s_addr = htonl(INADDR_ANY);          // Set the local host port
    local.sin_port = htons(PORT);                       // Set the local host IP address

    /* Variables to used to check the amount of data */
    long int received = 0;
    int tmp = 0;
    char buf[LENGTH];

    /* Checking if we are going to use TCP or UDP */
    if (PROTOCOL == 17) { // if UDP
        if (DIRECTION == 2) { // If downstream
            socketDescr = socket(AF_INET,SOCK_DGRAM,0);    // Creating the socket, SOCK_DGRAM = UDP/
            sendto(socketDescr, buf, sizeof(buf), 0, (struct sockaddr *) &server, len);
            printf("Receiving data...\n");
            returnTime = gettimeofday(&start_t, &tzp);  // Check time when starting
            while (tmp != 1) {
                tmp = recvfrom(socketDescr, buf, sizeof(buf), 0, (struct sockaddr *) &server, &len);
                received = received + tmp;
            }
            returnTime = gettimeofday(&end_t, &tzp);    // Check time when finished
            double totalTime = timeDifference(&start_t, &end_t) / 1000; // Get time difference in milliseconds, make into seconds
            double mbits = ((received/MEGA)*8) / totalTime;             // Calculating speed in Mbit/s
            printf("Received %ld MB in %f seconds = %lf Mbit/s\n", received/MEGA, totalTime, mbits); //Printing results
        } else if (DIRECTION == 1) { // If upstream
            send_data(LENGTH, PORT, PROTOCOL);
        }

    } else if (PROTOCOL == 6) { // If TCP
        if (DIRECTION == 2) { // If downstream
            socketDescr = socket(AF_INET,SOCK_STREAM,0);    // Creating the socket, SOCK_STREAM = TCP/
            connect(socketDescr, (struct sockaddr *)&server, sizeof(struct sockaddr)); // Establishing TCP connection
            printf("TCP connection established. Receiving data...\n");
            returnTime = gettimeofday(&start_t, &tzp);  // Check time when starting
            while (received < DATA_TO_TRANSFER) {
                tmp = recv(socketDescr, buf, LENGTH, 0);
                received = received + tmp;
            }
            returnTime = gettimeofday(&end_t, &tzp);    // Check time when finished
            close(socketDescr);                             // Closing the socket
            double totalTime = timeDifference(&start_t, &end_t) / 1000; // Get time difference in milliseconds, make into seconds
            double mbits = ((received/MEGA)*8) / totalTime;             // Calculating speed in Mbit/s
            printf("Received %ld MB in %f seconds = %lf Mbit/s\n", received/MEGA, totalTime, mbits); //Printing results
        } else if (DIRECTION == 1) { // If upstream
            send_data(LENGTH, PORT, PROTOCOL);
        }

    } else {
        printf("Invalid protocol specified!\n");
        exit(0);
    }

    return 0;
}