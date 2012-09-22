#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

// Include configurable constants for this project
#include "constants.h"
#include "dns.h"

int main() {
    int sock;
    int bytes_read;
    // Buffer for data we've read from socket.
    char recv_data[1024];
    // Socket address for local and remote sockets.
    struct sockaddr_in server_addr , client_addr;


    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        perror("Socket");
        exit(1);
    }

    int port = DEFAULT_PORT;

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    bzero(&(server_addr.sin_zero),8);

    if (bind(sock,(struct sockaddr *)&server_addr,
             sizeof(struct sockaddr)) == -1)
    {
        perror("Bind");
        exit(1);
    }

    // Length of a socket address.
    socklen_t addr_len;
    addr_len = sizeof(struct sockaddr);

    printf("\nUDPServer Waiting for client on port 5000\n");
    // Loop forever, waiting for DNS queries.
    while (1) {
        bytes_read = recvfrom(sock,recv_data,BUFFER_SIZE-1,0,
                              (struct sockaddr *)&client_addr, &addr_len);
        if( bytes_read == -1) {
            printf("Error reading from socket: %s\n",strerror(errno));
            continue;
        }

        recv_data[bytes_read] = '\0';
        dns_header *header = (dns_header*) recv_data;
        printf("Got a request\n");
        printf("qr code is: %u\n",header->qr);
        printf("rd code is: %u\n",header->rd);
        printf("qd_count code is: %d\n",ntohs(header->qd_count));
        printf("an_count code is: %d\n",ntohs(header->an_count));
        printf("ns_count code is: %d\n",ntohs(header->ns_count));
        printf("ar_count code is: %d\n",ntohs(header->ar_count));
        int i;
        for( i =0; i < bytes_read; i++) {
            printf("%c",recv_data[i]);
        }
        printf("\n\n");
    }
    return 0;
}
