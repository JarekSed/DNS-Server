/*
	DNS structures
*/

/* dns header - see rfc1035 */
/* this is the main header of a DNS message */
/* it is followed by zero or more questions, answers, authorities, and additional sections */
/* the last four count fields tell you how many of each to expect */
#pragma once

typedef struct {
    unsigned short id;
    unsigned char rd:1;
    unsigned char tc:1;
    unsigned char aa:1;
    unsigned char opcode:4;
    unsigned char qr:1;
    unsigned char rcode:4;
    unsigned char cd:1;
    unsigned char ad:1;
    unsigned char z:1;
    unsigned char ra:1;
    unsigned short qd_count;
    unsigned short an_count;
    unsigned short ns_count;
    unsigned short ar_count;
} dns_header;

/* dns question section format. This is prepended with a name */
/* check the specs for the format of a name. Instead of components */
/* separated by dots, each component is prefixed with a byte containing */
/* the length of that component */

typedef struct {
    unsigned short qtype;
    unsigned short qclass;
} dns_question;

/* DNS resource record format */
/* The answer, authority, and additional sections all share this format. */
/* It is prepended with a name and suffixed with additional data */
typedef struct __attribute__ ((__packed__)) {
    unsigned short type;
    unsigned short _class;
    unsigned int ttl;
    unsigned short data_len;
    unsigned int data;
} dns_rrhdr;
