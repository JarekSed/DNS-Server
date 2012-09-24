CC = g++
CFLAGS  = -Wall -g -Wextra 

all: dns_server

dns_server: dns_server.o
	$(CC) $(CFLAGS) -o dns_server dns_server.o

dns_server.o: dns_server.cpp dns.h constants.h dns_server.h
	$(CC) $(CFLAGS) -c dns_server.cpp
clean:
	rm -f dns_server
	rm -f *.o

