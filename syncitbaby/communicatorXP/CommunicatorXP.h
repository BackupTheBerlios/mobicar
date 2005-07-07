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

#define RECV_BUFFER_SIZE 204800 // 200kb enough?


class CommunicatorXP {

public:
	CommunicatorXP();
	~CommunicatorXP();
	
	bool initUDPSender();
	bool initUDPReceiver();

	long sendData(unsigned char* data, int length);
	void receiveData();	// its void because uska gets refernce to obj
	
	void setPort(unsigned short port);
	unsigned short getPort();
	
	void setDestination(char* host);
	char* getDestination();
	
	void setLocalPort(unsigned short port);
	unsigned short getLocalPort();

	unsigned short getRTPSequenceNumber(unsigned char* buf);

private:

	bool initializedReceiver;
	bool initializedSender;

	long rc;	// bytes sent
	int sock;	// socket
    struct sockaddr_in server; /* server address info better its called client,,, but still on the "server" */

	unsigned char* buffer;	   /* just another buffer where data shall be stored */
	unsigned short port;	   /* port of remote server to send to */
	unsigned short localport;

	char* destination;		   /* host/ip of destination */
	
	#if defined(__WIN32__) || defined(_WIN32)
	WSADATA wsa; // necessary for win
	int startWinsock(void);
	#endif

// ######### Variables for the udpserver on the proxyside
	int sockudprecv;                 
   	 struct sockaddr_in udpserver;      
	unsigned char* recvdata;
	unsigned int recvlength;

};
