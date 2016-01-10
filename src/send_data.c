#include <stdio.h>          // Declarations used in most input/output
#include <stdlib.h>         // Defines several general purpose function
#include <string.h>         // Defines variable type and various functions for manipulating arrays of characters
#include <unistd.h>         // Needed to use getopt() etc to handle command line arguments better
#include <sys/types.h>      // Definitions of a number of data types used in system calls
#include <sys/socket.h>     // Includes a number of definitions of structures needed for sockets
#include <sys/stat.h>       // stat() to obtain information about file
#include <netinet/in.h>     // Constants and structures needed for internet domain addresses
#include <errno.h>

#include "error.h"

#define KILO 1024
#define MEGA (KILO*KILO)
#define MEGABYTES 100; // Configurable - how much data that should be sent (in MB) when testing throuhgput

const long int DATA_TO_TRANSFER = MEGA*MEGABYTES; // Do not change

void send_data(int length, int port, int protocol) {
    long int size;
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
    } else {
        returnVal = -1;
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
            printf("Incoming connection from %d - sending data\n", inet_ntoa(client.sin_addr));
            long int count;
            for (count=0;count<DATA_TO_TRANSFER;count+sizeof(buf)) {
                send(consocket, buf, length, 0);
            }
            consocket = accept(socketDescr, (struct sockaddr *)&client, &socksize);
        }
    } else if (protocol == 17) {
        printf("Listening for connection to send UDP...\n", NULL);
        recvfrom(socketDescr, buf, sizeof(buf), 0, (struct sockaddr *) &client, &len);
        printf("Incoming connection from %d - sending data\n", inet_ntoa(client.sin_addr));
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