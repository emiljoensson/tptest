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

void sendData(int length, int port, int protocol) {
    long int size;
    int returnValue;
    size = MEGA;

    int socketDescr;  // socket descriptor
    char buf[length];  // contains the memory address to store the data
    memset(buf, '$', length); // filling the space
    char terminate[1];
    memset(terminate, '$', 1); // filling the space
    socklen_t socksize = sizeof(struct sockaddr_in); // tcp only

    struct sockaddr_in client;    // needed for TCP, socket info about the client connecting to this server
    struct sockaddr_in server;    // socket info about this server
    int len = sizeof(struct sockaddr_in);
    memset(&server, 0, sizeof(server));       // initializing the struct before filling it (zeroing)
    server.sin_family = AF_INET;          // address family, always AF_INET (TCP/IP)
    server.sin_addr.s_addr = htonl(INADDR_ANY);   // set our listen address to any interface
    server.sin_port = htons(port);      // set server port number, as defined above

    /* Creating the socket */
    if (protocol == 6) { // If TCP
        socketDescr = socket(AF_INET,SOCK_STREAM,0);
    } else if (protocol == 17) { // If UDP
        socketDescr = socket(AF_INET,SOCK_DGRAM,0);
    }

    /* Binding server information to the socket */
    int returnVal;
    if (protocol == 6) {
        returnVal = bind(socketDescr, (struct sockaddr *)&server, sizeof(struct sockaddr));
    } else if (protocol == 17) {
        returnVal = bind(socketDescr, (struct sockaddr *)&server, sizeof(server));
    }

    if (returnVal != 0) {
        printf("Socket bind failed = %d\n", errno);
        exit(0);
    }

    int consocket;
    if (protocol == 6) {
        printf("Listening for connection to send TCP...\n", NULL);
        listen(socketDescr, 1);
        consocket = accept(socketDescr, (struct sockaddr *)&client, &socksize);
        while(consocket) {
            printf("Incoming connection from %s - sending data\n", inet_ntoa(client.sin_addr));
            long int count;
            for (count=0;count<DATA_TO_TRANSFER;count+sizeof(buf)) {
                send(consocket, buf, length, 0);
            }
            consocket = accept(socketDescr, (struct sockaddr *)&client, &socksize);
        }
    } else if (protocol == 17) {
        printf("Listening for connection to send UDP...\n", NULL);
        recvfrom(socketDescr, buf, sizeof(buf), 0, (struct sockaddr *) &client, &len);
        printf("Incoming connection from %s - sending data\n", inet_ntoa(client.sin_addr));
        long int count;
        for (count=0;count<DATA_TO_TRANSFER;count+=length) {
            sendto(socketDescr, buf, sizeof(buf), 0, (struct sockaddr *) &client, len);
        }
        sendto(socketDescr, terminate, sizeof(terminate), 0, (struct sockaddr *) &client, len);
    }

    // Only TCP
    if (protocol == 6) {
        close(consocket);
    }

    // TCP and UDP:
    close(socketDescr);

}

int main (int argc, char *argv[]) {

    /* Checking if user provided us with 5 arguments */
    if (argc < 6) {
        printf("usage:\n", NULL);
        printf("./tptest destination(IP) port(1-65535) protocol(tcp=6/udp=17) length(packet length) direction(1=upstream,2=downstream)\n", NULL);
        printf("example: ./tptest 192.168.1.2 1025 6 512 2\n", NULL);
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
        printf("Invalid port. Valid range is 1-65535.\n", NULL);
        exit(0);
    }

    if (LENGTH < 2 || LENGTH > 9198) {
        printf("Invalid packet length. Valid range is 2-9198.\n", NULL);
        exit(0);
    }

    if (DIRECTION < 1 || DIRECTION > 2) {
        printf("Invalid direction. Use 1 for upstream or 2 for downstream.\n", NULL);
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
            printf("Receiving data...\n", NULL);
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
            sendData(LENGTH, PORT, PROTOCOL);
        }

    } else if (PROTOCOL == 6) { // If TCP
        if (DIRECTION == 2) { // If downstream
            socketDescr = socket(AF_INET,SOCK_STREAM,0);    // Creating the socket, SOCK_STREAM = TCP/
            connect(socketDescr, (struct sockaddr *)&server, sizeof(struct sockaddr)); // Establishing TCP connection
            printf("TCP connection established. Receiving data...\n", NULL);
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
            sendData(LENGTH, PORT, PROTOCOL);
        }

    } else {
        printf("Invalid protocol specified!\n", NULL);
        exit(0);
    }

    return 0;
}