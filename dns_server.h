#include <vector>
#include <unordered_map>
#include "exceptions.h"
#pragma once
using namespace std;


class DNS_Server {

public:
    DNS_Server() {}

    /**
     * Reads a hosts file. May throw exception if the hosts file is badly formatted or unreadable.
     * file_path: path to file to read.
     * lookup_table: hash_map to be populated with map of  domains to ints (1 byte per octet).
     *
     * On errors, throws exception
     */
    void read_hosts_file(string host_file_path) throw(FormatException);

    /* Listens on the given port for DNS requests, and responds to them.
     * This blocks and loops indefinitely.
     *
     * port_number: which port to listen on.
     *
     * On errors, prints the error to stderr, and exits the program
     */
    void listen_on_socket(int port_number);

private:
    /* This parses dns query messages and builds the return message
     * dns_header will be modified to contain the answer section of a DNS message.
     *
     * message: pointer to the start of the dns messege.
     *     Caller maintains ownership of *message.
     * domains: vector to be populated with the domains that are being queried for
     *      in this message.
     *
     * returns a pointer to the end of the dns message (where data can
     *      be appended to be sent as a response)
     *
     *  This may throw a FormatException if the message is malformated
     *      or is not a query
     */
    char *parse_dns_request(dns_header *message, vector<string> &domains) throw (FormatException);

    /* This reads out the addresses that need IP addresses from the question
     * section of a dns request.
     *
     * question_section: pointer to the start of the question section.
     *  Caller maintains ownership of question_section.
     * num_queries: how many queries are contained in the question section.
     * queries: vector that will be populated with the domains from the question section.
     */
    char *get_queries_from_question_section(char *question_section,
                                            const int num_queries,
                                            vector<string> &queries) throw(FormatException);

    /* Convert domain name to component form.
     * size will hold the size of the char array that
     * is returned.
     */
    char* domain_to_components(string domain, int *size);

    /* Helper function to tokenize strings.
     * From http://stackoverflow.com/questions/10051679/c-tokenize-string
     *  source: string to be tokenized
     *  delim: deliminter
     *
     *  returns a vector containing source split by delim
     */
    vector<string> inline tokenize(const string &source, const char delim);

    unordered_map<string, int> lookup_table;
};
