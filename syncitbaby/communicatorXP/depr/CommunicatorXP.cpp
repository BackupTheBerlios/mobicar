#if defined(__WIN32__) || defined(_WIN32)
#include "stdafx.h"
#endif
#include "CommunicatorXP.h"

CommunicatorXP::CommunicatorXP() {
	
	// falls windows erstmal den winsock initialisieren
	#if defined(__WIN32__) || defined(_WIN32)
	this->rc = WSAStartup(MAKEWORD(2,0),&wsa);
	
	if(rc!=0){
		printf("[-] Error starting Winsock ! (code: %d)\n",rc);
	} else {
		printf("[+] Successfully started winsock !\n");
	}
	#endif

	// socket grabben
	this->sock=socket(AF_INET,SOCK_DGRAM,0);
	
  
	if(this->sock == -1) {
		#if defined(__WIN32__) || defined(_WIN32)
		printf("[-] Error ! Couldn't create socket! (code: %d)\n",WSAGetLastError());
		#else
		printf("[-] Error ! Couldn't create socket! (code: %d)\n",sock);
		#endif
	
	} else {
		; //printf("UDP Socket erstellt!\n");
	}
	memset((char *) &server, 0, sizeof(server)); // speicher für uns reservieren :)
	
	this->server.sin_family=AF_INET;
	

} // end Constructor

CommunicatorXP::~CommunicatorXP() {
	//close(sock);
}


long CommunicatorXP::senddata(unsigned char* data,int length) {

	// Those 2 thingies are here in order 2 change port/destination on the fly!
	
	this->server.sin_port=htons(this->port);
	//this->server.sin_addr.s_addr= inet_addr("192.168.123.126");
	//bind(sock, (sockaddr *)&server, sizeof(server));
  	this->server.sin_addr.s_addr=inet_addr(this->destination);
	if (data != NULL) {
		rc = sendto(sock,(char*) data,length,0,(sockaddr*)&server,sizeof(server));
	}
	return rc; // returns bytes sent (-1 if error)
}

void CommunicatorXP::setPort(unsigned short rport) {
	this->port = rport;
}

unsigned short CommunicatorXP::getPort() {
	return this->port;
}

void CommunicatorXP::setDestination(char* host) {
	this->destination = host;
}

char* CommunicatorXP::getDestination() {
	return this->destination;
}
