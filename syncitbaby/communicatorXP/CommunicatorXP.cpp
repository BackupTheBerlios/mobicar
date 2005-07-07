#include "stdafx.h"
#include "CommunicatorXP.h"

CommunicatorXP::CommunicatorXP() {
} // end Constructor

CommunicatorXP::~CommunicatorXP() {
	//close(sock);
}

bool CommunicatorXP::initUDPSender() {
		// falls windows erstmal den winsock initialisieren
	#if defined(__WIN32__) || defined(_WIN32)
	this->rc = WSAStartup(MAKEWORD(2,0),&wsa);
	
	if(rc!=0){
		printf("[-] Error starting Winsock ! (code: %d)\n",rc);
		initializedSender =0;
		return initializedSender;
	} else {
		printf("[+] Successfully started winsock !\n");
	}
	#endif

	// socket grabben
	this->sock=socket(AF_INET,SOCK_DGRAM,0);
  
	if(this->sock == -1) {
		#if defined(__WIN32__) || defined(_WIN32)
		printf("[-] Error ! Couldn't create socket! (code: %d)\n",WSAGetLastError());
		initializedSender =0;
		return initializedSender;
		#else
		printf("[-] Error ! Couldn't create socket! (code: %d)\n",sock);
		initializedSender =0;
		return initializedSender;
		#endif
	
	} else {
		printf("UDP Socket erstellt!\n");
	}
	memset((char *) &server, 0, sizeof(server)); // speicher für uns reservieren :)
	
	this->server.sin_family=AF_INET;
	
	initializedSender=1;
	return initializedSender;
}

bool CommunicatorXP::initUDPReceiver() {
  
	#if defined(__WIN32__) || defined(_WIN32)
	this->rc = WSAStartup(MAKEWORD(2,0),&wsa);
	
	if(rc!=0){
		printf("[-] Error starting Winsock ! (code: %d)\n",rc);
	} else {
		printf("[+] Successfully started winsock !\n");
	}
	#endif

    if ((sockudprecv = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
    {
      fprintf(stderr, "Error: socket()\n");
      exit(1);
    }

    udpserver.sin_family      = AF_INET;
    udpserver.sin_addr.s_addr = htonl(INADDR_ANY); // fiXX to a specific local addy
    udpserver.sin_port        = htons(localport);

    if (bind(sockudprecv, (struct sockaddr *)&udpserver, sizeof(udpserver)) < 0) {
      fprintf(stderr, "Error: bind()\n");
	  initializedReceiver = 0;
	  return initializedReceiver;
    }
	initializedReceiver = 1;
	printf("[+] Initialize socket successfully ! \n");
	return initializedReceiver; // successfully initialized receiver sock
}

unsigned char* CommunicatorXP::buf2recvdata(char buf[]) {
	int i=0;
	unsigned char* tmpbuf=NULL;

	while(i < RECV_BUFFER_SIZE-1) {
		if (buf[i] == 0xFFFFFFFFFFFFFFCC) break;
		tmpbuf += buf[i]; // dunno if this fuckin works - dont really think so :( fiXX
		i++;
	}
	return tmpbuf;
}

void CommunicatorXP::receiveData() {

    struct sockaddr_in remote_addr;      /* 2 Adressen      */
    int remote_addr_size = sizeof(remote_addr);   /* fuer recvfrom() */
    char buf[1024]; 

	//char buf [RECV_BUFFER_SIZE];

	if(initializedReceiver) {

		printf("[*] Waiting for incoming data... \n");
		while(1) {

			if (recvfrom(sockudprecv,buf, sizeof(buf), 0,
	       (struct sockaddr *)&remote_addr, &remote_addr_size) > 0)
    {
      printf("Getting Data from %s\n",
	  inet_ntoa(remote_addr.sin_addr) );
   //   printf("Data : %s\n", buf);
	  recvdata = buf2recvdata(buf); // put buffer content into unsigned char ;)
	
	}

		}
	} // end if initialized
}

long CommunicatorXP::sendData(unsigned char* data,int length) {

	// Those 2 thingies are here in order 2 change port/destination on the fly!
	this->server.sin_port=htons(this->port);
  	this->server.sin_addr.s_addr=inet_addr(this->destination);
	
	if (data != NULL) {
		rc = sendto(sock,(char*) data,length,0,(SOCKADDR*)&server,sizeof(server));
	}
	return rc; // returns bytes sent (-1 if error)
}


void CommunicatorXP::setPort(unsigned short rport) {
	this->port = rport;
}

unsigned short CommunicatorXP::getPort() {
	return this->port;
}

void CommunicatorXP::setLocalPort(unsigned short rport) {
	this->localport = rport;
}
unsigned short CommunicatorXP::getLocalPort() {
	return this->localport;
}

void CommunicatorXP::setDestination(char* host) {
	this->destination = host;
}

char* CommunicatorXP::getDestination() {
	return this->destination;
}
