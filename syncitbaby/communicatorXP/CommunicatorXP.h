// falls windows...
#if defined(__WIN32__) || defined(_WIN32)
#include "stdafx.h"
#include "winsock.h"
#pragma comment(lib,"ws2_32")
#else // linux momentan
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#endif


class CommunicatorXP {

public:
	CommunicatorXP();
	~CommunicatorXP();
	
	long senddata(unsigned char* data, int length);
	
	void setPort(unsigned short port);
	unsigned short getPort();
	
	void setDestination(char* host);
	char* getDestination();

private:

	long rc;	// bytes sent
	int sock;	// socket
    struct sockaddr_in server; /* server address info */
    unsigned char* buffer;	   /* just another buffer where data shall be stored */
	unsigned short port;	   /* port of remote server to send to */
	char* destination;		   /* host/ip of destination */
	
	#if defined(__WIN32__) || defined(_WIN32)
	WSADATA wsa; // necessary for win
	#endif
	int startWinsock(void);
};
