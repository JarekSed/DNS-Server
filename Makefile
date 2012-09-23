CC = gcc
CFLAGS  = -Wall -g -Wextra 

all: dns_server

dns_server: dns_server.o
	$(CC) $(CFLAGS) -o dns_server dns_server.o

dns_server.o: dns_server.c dns.h constants.h
	$(CC) $(CFLAGS) -c dns_server.c
clean:
	rm -f dns_server
	rm -f *.o

