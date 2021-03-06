#include "CommunicatorXP.h"

CommunicatorXP::CommunicatorXP() {
} // end Constructor

CommunicatorXP::~CommunicatorXP() {
	#if defined(__WIN32__) || defined(_WIN32)
	WSACleanup();
	#endif
	// close(sock); well - usually cleanup thingies,, but doesnt work well :( missing include
}

void CommunicatorXP::setPort(unsigned short rport) { // port to send data to
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

unsigned short CommunicatorXP::getRTPSequenceNumber(unsigned char* buf){ // parse the current rtp-sequence number
	unsigned short sequencenumber=0;
	// bytes 3&4 represent the sequencenumber
	sequencenumber = buf[2];
	sequencenumber = sequencenumber<<8;
	sequencenumber +=buf[3];
	
	return sequencenumber;
}

bool CommunicatorXP::initUDPSender() {
		// falls windows erstmal den winsock initialisieren
	long rc;	// bytes sent

    #if defined(__WIN32__) || defined(_WIN32)
	rc = WSAStartup(MAKEWORD(2,0),&wsa);
	
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
	memset((char *) &server, 0, sizeof(server)); // speicher f�r uns reservieren :)
	
	this->server.sin_family=AF_INET;
	
	initializedSender=1;
	return initializedSender;
}

bool CommunicatorXP::initUDPReceiver() {
  
	long rc;	// bytes sent

	#if defined(__WIN32__) || defined(_WIN32)
	rc = WSAStartup(MAKEWORD(2,0),&wsa);
	
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

void CommunicatorXP::receiveData() {

    struct sockaddr_in remote_addr;      /* 2 Adressen      */
    int remote_addr_size = sizeof(remote_addr);   /* fuer recvfrom() */
    long recvsize=0;
	
	recvdata = (unsigned char*) malloc(RECV_BUFFER_SIZE); //fixx into sizeofbuffer?
    if(initializedReceiver) {
  
		#if defined(__WIN32__) || defined(_WIN32)
		recvsize = recvfrom(sockudprecv, (char*)recvdata, RECV_BUFFER_SIZE, 0,
                                  (struct sockaddr *)&remote_addr, &remote_addr_size);
		#else
		recvsize = recvfrom(sockudprecv, recvdata, RECV_BUFFER_SIZE, 0,
	                   (struct sockaddr *)&remote_addr, (socklen_t*)&remote_addr_size);
        #endif

		if (recvsize > 0) {
            //      printf("Getting Data from %s\n",
            //	     inet_ntoa(remote_addr.sin_addr.s_addr) );
	        printf("Data : %s\n", recvdata); // data has to be worked with
		}
	} // end if initialized
    
}

long CommunicatorXP::sendData(unsigned char* data,int length) {

	long rc;	// bytes sent

	// Those 2 thingies are here in order 2 change port/destination on the fly!
	if (port == NULL) {
		printf("[-] Error! No Destination Port specified! break...\n");
		return -1;
	}
	this->server.sin_port=htons(this->port);
  	this->server.sin_addr.s_addr=inet_addr(this->destination);
	
	if (data != NULL) {
		rc = sendto(sock,(char*) data,length,0,(struct sockaddr*)&server,sizeof(server));
	}
	return rc; // returns bytes sent (-1 if error)
}


/*
int _tmain(int argc, _TCHAR* argv[])
{
}
*/
