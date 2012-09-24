#include <vector>
using namespace std;

/* Listens on the given port for DNS requests, and responds to them.
 * This blocks and loops indefinitely.
 *
 * port_number: which port to listen on.
 *
 * On errors, prints the error to stderr, and exits the program
 */
void listen_on_socket(int port_number);

/* This parses dns query messages and builds the return message
 * dns_header will be modified to contain the answer section of a DNS message.
 *
 * message: pointer to the start of the dns messege.
 *     Caller maintains ownership of *message.
 *
 * returns NO_ERROR on success, ERR_NOT_DNS_QUERY if message is not a dns query,
 *  and ERR_MESSAGE_BUFFER_TOO_SMALL if adding the answer section causes
 *  *message to grow to more than BUFFER_SIZE bytes.
 */
ErrorCode parse_dns_request(dns_header *message);

/* This reads out the addresses that need IP addresses from the question 
 * section of a dns request. 
 *
 * question_section: pointer to the start of the question section.
 * num_queries: how many queries are contained in the question section.
 * queries: vector that will be populated with the domains from the question section.
 */
char *get_queries_from_question_section(char *question_section, const int num_queries, vector<string> &queries);
