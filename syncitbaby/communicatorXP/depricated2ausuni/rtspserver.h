/******************************************************
**
** Video at Home; SEP SS 2005
** (c) Insitut fuer Betriebssysteme und Rechnerverbund
** TU Braunschweig
**
**
**
** This program is free software; you can redistribute it and/or
** modify it under the terms of the GNU General Public License
** as published by the Free Software Foundation; either version 2
** of the License, or (at your option) any later version.
**
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
** Or, point your browser to http://www.gnu.org/copyleft/gpl.html
**		   
**
**
** Description:
** This Class is a full-RTSP-Server 
**
**
**
**
** Author: M. Klama(Communication)
** Date:   Jul. 04, 2005
**
** 
** Modifications:
** Date:           Name:          Changes:           
** Jul. 04, 2005   M. Klama     Started this.
** Jul. 05, 2005   M. Klama     added handle_describe
** Jul. 06, 2005   M. Klama     bugfixxes - debugging stuff
** Jul. 07, 2005   M. Klama     added setup handler
** Jul. 08, 2005   M. Klama     ported actual work to linux
**
** TODO:
** - search for fiXX and fiXX ;) 
** - dynamic session management
** - check if all RECV/SEND BUFFER sizes are correct
** - handle teardown better -> dont terminate just close connection
** - testing with clients other then vlc
** - support other streams then UDP/RTP see handle fun
******************************************************/


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

#include "time.h"

//#define DEBUG // enables debug ;)
//#define DEBUGRECEIVE // shows all data received from client

#define ALLOWEDCOMMANDS "OPTIONS, DESCRIBE, SETUP, TEARDOWN, PLAY, PAUSE" // MANDADORY add 
#define RTSP_BUFFER_SIZE 100000 // for incoming requests, and outgoing responses
#define PARAM_STRING_MAX 100

class RtspServer {

public:
	RtspServer();
	~RtspServer();
	
	void setLocalIP(char* ip);
	void setRTPAudioPort(unsigned short p);
	void setRTPVideoPort(unsigned short p);
	void setRtspPort(unsigned short p);
	void setSessionName(char* sessionname);
	void setSessionDescription(char* sd);
	void setStreamOrigin(char* file);

	void tcplisten();

private:

	#if defined(__WIN32__) || defined(_WIN32)
	WSADATA wsa;
	#endif
	
	struct sockaddr_in tcpserver;
    struct sockaddr_in remote_addr;
    int size;

	unsigned short rtpAudioPort;
	unsigned short rtpVideoPort;
	char* localIP;
	char* remoteIP;

    #if defined(__WIN32__) || defined(_WIN32)
	SOCKET sock;
	SOCKET remote_s;
	#else
	int sock;
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
	char* sessionName;
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
	void handle_notSupported_cmd(char const* cseq); // handle not supported
	void handle_withinSession_cmd(char const* cmdName, char const* urlPreSuffix, char const* urlSuffix,
								  char const* cseq, char const* fullRequestStr);
	void handle_PLAY_cmd(char const* cseq, char const* fullRequestStr); // handle play
	void handle_PAUSE_cmd(char const* cseq); // handle pause
    void handle_TEARDOWN_cmd(char const* cseq); // handle teardown,,,
	void handle_GET_PARAMETER_cmd(char const* cseq,char const* /*fullRequestStr*/);


};
