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
#include <fstream>

// Include configurable constants for this project
#include "constants.h"
// Header defining structures for parsing DNS requests
#include "dns.h"
// function prototypes
#include "dns_server.h"
#include "exceptions.h"

using namespace std;

void usage() {
    cout << "Usage: ./dns-server [-p port] [-h host_file]" << endl;
}

int main(int argc, char* argv[]) {
    int port = DEFAULT_PORT;
    string host_file = DEFAULT_HOST_FILE;
    int c;
    while ((c = getopt (argc, argv, "f:p:")) != -1)
        switch (c)
        {
            case 'f':
                host_file = optarg;
                break;
            case 'p':
                port = atoi(optarg);
                if (port < 1) {
                    cerr << "Error: port must be a positive integer" << endl;
                    return 1;
                }
                break;
            default:
                usage();
                return 1;
        }
    DNS_Server server;

    server.read_hosts_file(host_file);

    server.listen_on_socket(port);
}

char *DNS_Server::get_queries_from_question_section(char *question_section, 
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
        dns_question *question_fields = (dns_question*) ++question_section;
        cout << "We seem to have gotten a request for " << address << endl;
        if (ntohs(question_fields->qtype) == 1 
                && ntohs(question_fields->qclass) == 1) {
            queries.push_back(address);
        }
        // After we have the address, move past the type and class fields.
        question_section += sizeof(dns_question);
    }
    return question_section;
}

char *DNS_Server::domain_to_components(string domain, int *size) {
    *size =2;
    char *str = new char[*size];
    str[0] = 0xC0;
    str[1] = 0x0C;
    return str;
/*
    // We have 1 extra byte byte per section, and we get rid of a '.' between all sections.
    // This means we have 1 extra char in the component form. We also need room for the trailing 0,
    // so length = length of string + 2.
    *size = domain.length()+2;
    vector<string> split = tokenize(domain, '.');
    char *str = new char[*size];
    unsigned int cur_index = 0;
    for (unsigned int i =0; i < split.size(); i++) {
        str[cur_index++] = split.at(i).size();
        strcpy(str + cur_index, split.at(i).c_str());
        cur_index += split.at(i).size();
    }
    return str;
*/
}

vector<string> inline DNS_Server::tokenize (const string &source, const char delimiter){
    vector<string> results;

    size_t prev = 0;
    size_t next = 0;

    while ((next = source.find_first_of(delimiter, prev)) != string::npos)
    {
        if (next - prev != 0)
        {
            results.push_back(source.substr(prev, next - prev));
        }
        prev = next + 1;
    }

    if (prev < source.size())
    {
        results.push_back(source.substr(prev));
    }

    return results;
}

char *DNS_Server::parse_dns_request(dns_header *message, vector<string> &domains){
    if (! message->qr == 0) {
        throw new FormatException("DNS message is not a query");
    }
    // We need a pointer to the question section of the DNS message.
    // This starts at the next byte after the end of the header.
    char *question = (char*)((void*)&message->ar_count) + sizeof(short);
    question = get_queries_from_question_section(question, 
            ntohs(message->qd_count),
            domains);

    return question;
}

void DNS_Server::read_hosts_file(string host_file_path) {
    ifstream host_file;
    host_file.open(host_file_path, ios::in);
    if (!host_file) {
        throw new FormatException("Error opening " + host_file_path + ": " + strerror(errno));
    }
    string line;
    string domain;
    string address;
    while (!host_file.eof()) {
        getline(host_file, line);
        // Trim out everything after # 
        line = line.substr(0,line.find("#",0));
        stringstream line_stream(line);
        // We should have exactly 2 fields, domain and address
        domain.clear(); 
        address.clear(); 
        line_stream >> domain;
        line_stream >> address;
        if (!address.empty()) {
            int address_int;
            if (inet_pton(AF_INET, address.c_str(), &address_int)){
                // TODO: check if key exists, first
                this->lookup_table[domain] = address_int;
            } else {
                throw new FormatException("Address " + address + " is not a valid IPv4 address");
            }
        }
    }
    host_file.close();
}

void DNS_Server::listen_on_socket(int port_number) {
    int sock;
    int bytes_read;
    // Buffer for data we've read from socket.
    char recv_data[1024];
    // Socket address for local and remote sockets.
    struct sockaddr_in server_addr , client_addr;

    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        cerr << "Failed to initialize socket: " << strerror(errno) << endl;
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port_number);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    bzero(&(server_addr.sin_zero),8);

    if (bind(sock,(struct sockaddr *)&server_addr,
                sizeof(struct sockaddr)) == -1)
    {
        cerr << "Failed to bind socket to port " << port_number << ": " << strerror(errno) << endl;;
        exit(2);
    }

    // Length of a socket address.
    socklen_t addr_len;
    addr_len = sizeof(struct sockaddr);

    printf("\nUDPServer Waiting for client on port %d\n", port_number);
    // Loop forever, waiting for DNS queries.
    while (1) {
        bytes_read = recvfrom(sock,&recv_data,BUFFER_SIZE-1,0,
                (struct sockaddr *)&client_addr, &addr_len);
        if( bytes_read == -1) {
            cerr << "Error reading from socket: " << strerror(errno) << endl;
            continue;
        }

        recv_data[bytes_read] = '\0';
        dns_header *header = (dns_header*) &recv_data;
        try{
            vector<string> domains;
            char *end_of_message = parse_dns_request(header, domains);
            header->qr = 1;
            header->aa = 1;
            header->tc =0;
            header->ra =0;
            header->rcode =0;
            header->an_count = 0;
            int size_of_message_so_far = 0;
            for (size_t i =0; i < domains.size(); i++) {
                int address_response = htonl(lookup_table[domains[i]]);
                header->an_count++;
                dns_rrhdr answer_header;
                int size = 0;
                char *components = domain_to_components(domains[i], &size);
                answer_header.type = htons(1);
                answer_header._class = htons(1);
                // Tell clients that this reply is good for 24 hours.
                answer_header.ttl=  htonl(86400);
                answer_header.data_len = htons(4);
                answer_header.data = htonl(address_response);
                size_of_message_so_far = end_of_message -  (char*)header;
                end_of_message = (char*) header  + size_of_message_so_far;
                size_of_message_so_far += sizeof(dns_rrhdr);
                memcpy(end_of_message, components, size);
                end_of_message += size;
                size_of_message_so_far+=2;
                memcpy(end_of_message, &answer_header, sizeof(dns_rrhdr));
                delete[] components;
            }
            header->an_count = htons(header->an_count);
            sendto(sock, header, size_of_message_so_far, 0, (sockaddr*) &client_addr, addr_len);
        }catch (FormatException e) {
            // TODO: return dns error message.
            cerr << "Could not parse dns request: " << e.what() << endl;
        }
    }
}
