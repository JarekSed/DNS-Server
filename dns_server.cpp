// C includes (for socket operations)
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
// C++ Includes 
#include <sstream>
#include <iostream>

// Include configurable constants for this project
#include "constants.h"
// Header defining structures for parsing DNS requests
#include "dns.h"
// function prototypes
#include "dns_server.h"

using namespace std;

int main() {
    listen_on_socket(DEFAULT_PORT);
}

char *get_queries_from_question_section(char *question_section, 
                                        const int num_queries,
                                        vector<string> &queries) {
    // Loop through all of our queries.
    for(int i =0; i < num_queries; i++){
        printf("We are looking at query %d\n",i);
        // We use a stringstream to build the address char by char.
        stringstream query_name;

        // We are going to process each section of the network address.
        // This loops runs until it finds a section with length 0, 
        // signifying the end of this address.
        while(*question_section != '\0') {
            // Length of the this section of the network address is the first byte.
            char length = *question_section++; 
            // This loop grabs all chars in this section of the address, and
            // adds them to the stringstream. After it ends, *question_section is pointing
            // at the length of the next section.
            while(length-- > 0) {
                query_name << *question_section++;
            }
            // Sections are separated by '.'
            // We add these after every section, so we'll need to trim the last one.
            query_name << '.';
        }
        string address = query_name.str();
        // If the address had at least one section, we have a trailing period
        // that needs to be trimmed.
        if (address.length() > 0){
            address.resize(address.length() -1 );
        }
        cout << "We seem to have gotten a request for " << address << endl;
        queries.push_back(address);
    }
    return question_section;
}

ErrorCode parse_dns_request(dns_header *message){
    if (! message->qr == 0) {
        return NOT_DNS_QUERY;
    }
    // We need a pointer to the question section of the DNS message.
    // This starts at the next byte after the end of the header.
    char *question = (char*)((void*)&message->ar_count) + sizeof(short);
    vector<string> domains;
    question = get_queries_from_question_section(question, 
                                                 ntohs(message->qd_count),
                                                 domains);
    
   return NO_ERROR;
}

void listen_on_socket(int port_number) {
    int sock;
    int bytes_read;
    // Buffer for data we've read from socket.
    char recv_data[1024];
    // Socket address for local and remote sockets.
    struct sockaddr_in server_addr , client_addr;

    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        fprintf(stderr, "Failed to initialize socket\n");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port_number);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    bzero(&(server_addr.sin_zero),8);

    if (bind(sock,(struct sockaddr *)&server_addr,
             sizeof(struct sockaddr)) == -1)
    {
        fprintf(stderr, "Failed to bind socket to port %d\n", port_number);
        exit(2);
    }

    // Length of a socket address.
    socklen_t addr_len;
    addr_len = sizeof(struct sockaddr);

    printf("\nUDPServer Waiting for client on port %d\n", port_number);
    // Loop forever, waiting for DNS queries.
    while (1) {
        bytes_read = recvfrom(sock,recv_data,BUFFER_SIZE-1,0,
                              (struct sockaddr *)&client_addr, &addr_len);
        if( bytes_read == -1) {
            fprintf(stderr, "Error reading from socket: %s\n",strerror(errno));
            continue;
        }

        recv_data[bytes_read] = '\0';
        dns_header *header = (dns_header*) recv_data;
        ErrorCode err = parse_dns_request(header);

        switch(err) {
            case NOT_DNS_QUERY:
                fprintf(stderr, "Recieved something that was not a query\n");
                break;
            // TODO: send response.
            case NO_ERROR:
                break;
        }
    }
}
