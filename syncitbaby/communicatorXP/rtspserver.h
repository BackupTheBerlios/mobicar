#if defined(__WIN32__) || defined(_WIN32)
//#include "stdafx.h"
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

#include "time.h"


#define RTSP_BUFFER_SIZE 10000 // for incoming requests, and outgoing responses
#define PARAM_STRING_MAX 100

class RtspServer {

public:
	RtspServer();
	~RtspServer();
	
	void startServer();
	void setRtspPort(unsigned short p);
	void setSessionDescription(char* sd);
	void setStreamOrigin(char* file);
	void tcplisten();

private:

	WSADATA wsa;
	struct sockaddr_in tcpserver;
    struct sockaddr_in remote_addr;
    int size;
   
    #if defined(__WIN32__) || defined(_WIN32)
	SOCKET sock;
	#else
	int sock;
	#endif
    #if defined(__WIN32__) || defined(_WIN32)
	SOCKET remote_s;
	#else
	int remote_s;
	#endif

	char buffer[1024];

	//rtsp fun
	typedef enum StreamingMode {
	  RTP_UDP,
	  RTP_TCP,
	  RAW_UDP
	};



	char const* allowedCommandNames;
	unsigned char fBuffer[RTSP_BUFFER_SIZE];
	unsigned char fResponseBuffer[RTSP_BUFFER_SIZE];

	char* sessionDescription;
	char* filename;
	char* streamservername;
	unsigned char streamcount; // how many subsessions exist 0 = error
    unsigned short rtspPort;
    unsigned char tmpstring[1000];


// Methoden .........
	bool initSocket();

	char* strDup(char const* str);
	char* strDupSize(char const* str);
	char* generateSDPDescription();
	char* generate_RTSPURL();
    void parseTransportHeader(char const* buf,
				 StreamingMode& streamingMode,
				 char*& streamingModeString,
				 char*& destinationAddressStr,
				 unsigned& destinationTTL,
				 unsigned short& rtpPort, // if UDP
				 unsigned short& rtcpPort, // if UDP
				 unsigned char& rtpChannelId, // if TCP
				 unsigned char& rtcpChannelId // if TCP
				 );
	bool parseRequestString(char const* reqStr,
		       unsigned reqStrSize,
		       char* resultCmdName,
		       unsigned resultCmdNameMaxSize,
		       char* resultURLPreSuffix,
		       unsigned resultURLPreSuffixMaxSize,
		       char* resultURLSuffix,
		       unsigned resultURLSuffixMaxSize,
		       char* resultCSeq,
		       unsigned resultCSeqMaxSize);
    bool parseRangeHeader(char const* buf, float& rangeStart, float& rangeEnd);
	bool parseScaleHeader(char const* buf, float& scale);

	char const* dateHeader();

	void handle_INCOMING_request(unsigned char* daten,int totalBytes);
	void handle_OPTIONS_cmd(char const* cseq);
    void handle_DESCRIBE_cmd(char const* cseq, char const* urlSuffix, char const* fullRequestStr);
	void handle_SETUP_cmd(char const* cseq, char const* urlPreSuffix, char const* urlSuffix, char const* fullRequestStr);
	void handle_bad_cmd(char const* /*cseq*/);
	void handle_notSupported_cmd(char const* cseq);
	void handle_withinSession_cmd(char const* cmdName, char const* urlPreSuffix, char const* urlSuffix,
								  char const* cseq, char const* fullRequestStr);
	void handle_PLAY_cmd(char const* cseq, char const* fullRequestStr); 
	void handle_PAUSE_cmd(char const* cseq);
    void handle_TEARDOWN_cmd(char const* cseq);
	void handle_GET_PARAMETER_cmd(char const* cseq,char const* /*fullRequestStr*/);


};