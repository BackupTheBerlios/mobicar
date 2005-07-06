// rtspserver.cpp : Definiert den Einstiegspunkt f�r die Konsolenanwendung.
//
//#include "stdafx.h"
#include "rtspserver.h"

RtspServer::RtspServer() {
	streamservername = "Sky streaming media server ver 0.1alpha";			 // Server-Name & Version :)
	sessionDescription = "TEST-STREAM by SKYRAVER::COMMUNICATION";
	filename="test.mpg";
	streamcount='b';
	allowedCommandNames = "OPTIONS, DESCRIBE, SETUP, TEARDOWN, PLAY, PAUSE"; // Allowed commands on this Server
	setRtspPort(554);
	setStreamOrigin("test.mpg");
	setSessionDescription("Session streamed by Video@Home::Communication-Team");
	initSocket();

}

RtspServer::~RtspServer() {
}

void RtspServer::setRtspPort(unsigned short p) {
	rtspPort = p;
}

void RtspServer::setStreamOrigin(char* file) {
	filename = file;
}

void RtspServer::setSessionDescription(char* sd) {
	sessionDescription = sd;
}


char* RtspServer::strDup(char const* str) {
  if (str == NULL) return NULL;
  size_t len = strlen(str) + 1;
  char* copy = new char[len];

  if (copy != NULL) {
    memcpy(copy, str, len);
  }
  return copy;
}

char* RtspServer::strDupSize(char const* str) {
  if (str == NULL) return NULL;
  size_t len = strlen(str) + 1;
  char* copy = new char[len];
  return copy;
}


char* RtspServer::generateSDPDescription() {
  //  ourIPAddress.s_addr = ourSourceAddressForMulticast(envir());
  char* const ourIPAddressStr = "192.168.0.2";//strDup(inet_ntoa(tcpserver.sin_addr));
  unsigned ourIPAddressStrSize = strlen(ourIPAddressStr);

  char* sourceFilterLine;
  
  sourceFilterLine = "";
  
  char* rangeLine = NULL; // for now

  // Count the lengths of each subsession's media-level SDP lines.
  // (We do this first, because the call to "subsession->sdpLines()"
  // causes correct subsession 'duration()'s to be calculated later.)
  unsigned sdpLength = 0;

  rangeLine = strDup("a=range:npt=0-\r\n");

  char* streams;
  switch(streamcount) { // DYNAMISCH !!!!!!!!!!!!!!!!!!!1
	  case 'a': 
			   streams = "m=audio 5010 RTP/AVP 14\r\n"
				         "c=IN IP4 127.0.0.1/7\r\n"
						 "a=control:track1\r\n";
		       break;
	  case 'v':
			   streams = "m=video 5020 RTP/AVP 32\r\n"
						 "c=IN IP4 127.0.0.1/7\r\n"
						 "a=control:track2\r\n";
	  		   break;
	  case 'b':
			   streams = "m=audio 5010 RTP/AVP 14\r\n"
				         "c=IN IP4 127.0.0.1/7\r\n"
						 "a=control:track1\r\n"
						 "m=video 5020 RTP/AVP 32\r\n"
						 "c=IN IP4 127.0.0.1/7\r\n"
						 "a=control:track2\r\n";
	  		   break;
  }

  // Generate the SDP prefix (session-level lines):
   sprintf((char*)tmpstring,"v=0\r\n"
                "o=- %ld%06ld %d IN IP4 %s\r\n"
                "s=%s\r\n"
                "i=%s\r\n"
                "t=0 0\r\n"
                "a=tool:%s\r\n"
                "a=type:broadcast\r\n"
                "a=control:*\r\n"
                "%s"
                "a=x-qt-text-nam:%s\r\n"
                "a=x-qt-text-inf:%s\r\n"
				"%s",
			    12345, 67890, // o= <session id>
				"1", // o= <version> // (needs to change if params are modified)
				ourIPAddressStr, // o= <address>
				sessionDescription, // s= <description>
				filename, // i= <info>
				streamservername, // a=tool:
				rangeLine, // a=range: line
				sessionDescription, // a=x-qt-text-nam: line
				filename,// a=x-qt-text-inf: line
				streams); // audio/video tracks...

  
  return (char*)tmpstring;
 
}



char* RtspServer::generate_RTSPURL() {
  
	char const* sessionName = "testitMate";
	unsigned sessionNameLength = strlen(sessionName);

  char* urlBuffer = new char[100 + sessionNameLength];
  char* resultURL;
  
  if (rtspPort == 554 /* the default port number */) {
    sprintf(urlBuffer, "rtsp://%s/%s", "192.168.0.2",		// fiXX
	    sessionName);
  } else {

    sprintf(urlBuffer, "rtsp://%s:%hu/%s",
		"192.168.0.2", rtspPort,							// fiXX
	    sessionName);
  }

  resultURL = strDup(urlBuffer);
  delete[] urlBuffer;
  return resultURL;
}

bool RtspServer::parseRequestString(char const* reqStr,
		       unsigned reqStrSize,
		       char* resultCmdName,
		       unsigned resultCmdNameMaxSize,
		       char* resultURLPreSuffix,
		       unsigned resultURLPreSuffixMaxSize,
		       char* resultURLSuffix,
		       unsigned resultURLSuffixMaxSize,
		       char* resultCSeq,
		       unsigned resultCSeqMaxSize) {
  // This parser is currently rather dumb; it should be made smarter #####

  // Read everything up to the first space as the command name:
  bool parseSucceeded = 0;
  unsigned i;
  for (i = 0; i < resultCmdNameMaxSize-1 && i < reqStrSize; ++i) {
    char c = reqStr[i];
    if (c == ' ' || c == '\t') {
      parseSucceeded = 1;
      break;
    }

    resultCmdName[i] = c;
  }
  resultCmdName[i] = '\0';
  if (!parseSucceeded) return 0;
      
  // Skip over the prefix of any "rtsp://" or "rtsp:/" URL that follows:
  unsigned j = i+1;
  while (j < reqStrSize && (reqStr[j] == ' ' || reqStr[j] == '\t')) ++j; // skip over any additional white space
  for (j = i+1; j < reqStrSize-8; ++j) {
    if ((reqStr[j] == 'r' || reqStr[j] == 'R')
	&& (reqStr[j+1] == 't' || reqStr[j+1] == 'T')
	&& (reqStr[j+2] == 's' || reqStr[j+2] == 'S')
	&& (reqStr[j+3] == 'p' || reqStr[j+3] == 'P')
	&& reqStr[j+4] == ':' && reqStr[j+5] == '/') {
      j += 6;
      if (reqStr[j] == '/') {
	// This is a "rtsp://" URL; skip over the host:port part that follows:
	++j;
	while (j < reqStrSize && reqStr[j] != '/' && reqStr[j] != ' ') ++j;
      } else {
	// This is a "rtsp:/" URL; back up to the "/":
	--j;
      }
      i = j;
      break;
    }
  }

  // Look for the URL suffix (before the following "RTSP/"):
  parseSucceeded = 0;
  for (unsigned k = i+1; k < reqStrSize-5; ++k) {
    if (reqStr[k] == 'R' && reqStr[k+1] == 'T' &&
	reqStr[k+2] == 'S' && reqStr[k+3] == 'P' && reqStr[k+4] == '/') {
      while (--k >= i && reqStr[k] == ' ') {} // go back over all spaces before "RTSP/"
      unsigned k1 = k;
      while (k1 > i && reqStr[k1] != '/' && reqStr[k1] != ' ') --k1;
      // the URL suffix comes from [k1+1,k]

      // Copy "resultURLSuffix":
      if (k - k1 + 1 > resultURLSuffixMaxSize) return 0; // there's no room
      unsigned n = 0, k2 = k1+1;
      while (k2 <= k) resultURLSuffix[n++] = reqStr[k2++];
      resultURLSuffix[n] = '\0';

      // Also look for the URL 'pre-suffix' before this:
      unsigned k3 = --k1;
      while (k3 > i && reqStr[k3] != '/' && reqStr[k3] != ' ') --k3;
      // the URL pre-suffix comes from [k3+1,k1]

      // Copy "resultURLPreSuffix":
      if (k1 - k3 + 1 > resultURLPreSuffixMaxSize) return 0; // there's no room
      n = 0; k2 = k3+1;
      while (k2 <= k1) resultURLPreSuffix[n++] = reqStr[k2++];
      resultURLPreSuffix[n] = '\0';

      i = k + 7; // to go past " RTSP/"
      parseSucceeded = 1;
      break;
    }
  }
  if (!parseSucceeded) return 0;

  // Look for "CSeq:", skip whitespace,
  // then read everything up to the next \r or \n as 'CSeq':
  parseSucceeded = 0;
  for (j = i; j < reqStrSize-5; ++j) {
    if (reqStr[j] == 'C' && reqStr[j+1] == 'S' && reqStr[j+2] == 'e' &&
	reqStr[j+3] == 'q' && reqStr[j+4] == ':') {
      j += 5;
      unsigned n;
      while (j < reqStrSize && (reqStr[j] ==  ' ' || reqStr[j] == '\t')) ++j;
      for (n = 0; n < resultCSeqMaxSize-1 && j < reqStrSize; ++n,++j) {
	char c = reqStr[j];
	if (c == '\r' || c == '\n') {
	  parseSucceeded = 1;
	  break;
	}

	resultCSeq[n] = c;
      }
      resultCSeq[n] = '\0';
      break;
    }
  }
  if (!parseSucceeded) return 0;

  return 1;
}

char const* RtspServer::dateHeader() {
  static char buf[200];
  time_t tt = time(NULL);
  strftime(buf, sizeof buf, "Date: %a, %b %d %Y %H:%M:%S GMT\r\n", gmtime(&tt));
  return buf;
}
// ################################################################################


void RtspServer::handle_INCOMING_request(unsigned char* daten,int totalBytes) {
  
  // Parse the request string into command name and 'CSeq',
  // then handle the command:
  char cmdName[PARAM_STRING_MAX];
  char urlPreSuffix[PARAM_STRING_MAX];
  char urlSuffix[PARAM_STRING_MAX];
  char cseq[PARAM_STRING_MAX];
  
  if (!parseRequestString((char*)daten, totalBytes,
			  cmdName, sizeof cmdName,
			  urlPreSuffix, sizeof urlPreSuffix,
			  urlSuffix, sizeof urlSuffix,
			  cseq, sizeof cseq)) {
   printf("Bad Command received (%s) : \n%s\n",cmdName,daten);
   handle_bad_cmd(cseq);
   size = send(remote_s,(char*)fResponseBuffer,strlen((char*)fResponseBuffer),0);
   if (size >0) printf("Sent %d bytes \n",size); else printf("ERROR SENDING...\n");

	} else {
    if (strcmp(cmdName, "OPTIONS") == 0) {
      handle_OPTIONS_cmd(cseq);
	size = send(remote_s,(char*)fResponseBuffer,strlen((char*)fResponseBuffer),0);
	if (size >0) printf("Sent %d bytes \n",size); else printf("ERROR SENDING...\n");

	} else if (strcmp(cmdName, "DESCRIBE") == 0) {
		handle_DESCRIBE_cmd(cseq, urlSuffix, (char const*)daten);
		size = send(remote_s,(char*)fResponseBuffer,strlen((char*)fResponseBuffer),0);
		if (size >0) printf("Sent %d bytes \n",size); else printf("ERROR SENDING...\n");
    
	} else if (strcmp(cmdName, "SETUP") == 0) {
      handle_SETUP_cmd(cseq, urlPreSuffix, urlSuffix, (char const*)fBuffer);
   	  size = send(remote_s,(char*)fResponseBuffer,strlen((char*)fResponseBuffer),0);
	  if (size >0) printf("Sent %d bytes \n",size); else printf("ERROR SENDING...\n");

    } else if (strcmp(cmdName, "TEARDOWN") == 0
	       || strcmp(cmdName, "PLAY") == 0
	       || strcmp(cmdName, "PAUSE") == 0
	       || strcmp(cmdName, "GET_PARAMETER") == 0) {
      
	   handle_withinSession_cmd(cmdName, urlPreSuffix, urlSuffix, cseq,(char const*)fBuffer);
   	  size = send(remote_s,(char*)fResponseBuffer,strlen((char*)fResponseBuffer),0);
	  if (size >0) printf("Sent %d bytes \n",size); else printf("ERROR SENDING...\n");
		   
	} else {
     handle_notSupported_cmd(cseq);
	 size = send(remote_s,(char*)fResponseBuffer,strlen((char*)fResponseBuffer),0);
	 if (size >0) printf("Sent %d bytes \n",size); else printf("ERROR SENDING...\n");
    }
  }
}

void RtspServer::handle_OPTIONS_cmd(char const* cseq) {
  sprintf((char*)fResponseBuffer,
	   "RTSP/1.0 200 OK\r\nCSeq: %s\r\n%sPublic: %s\r\n\r\n",
	   cseq, dateHeader(), allowedCommandNames);
}

void RtspServer::handle_DESCRIBE_cmd(char const* cseq, char const* urlSuffix,char const* fullRequestStr) {
	
  // Also, generate our RTSP URL, for the "Content-Base:" header
  // (which is necessary to ensure that the correct URL gets used in
  // subsequent "SETUP" requests).

	unsigned sdpDescriptionSize = strlen(generateSDPDescription());

    sprintf((char*)fResponseBuffer,
	     "RTSP/1.0 200 OK\r\nCSeq: %s\r\n"
	     "%s"
	     "Content-Base: %s/\r\n"
	     "Content-Type: application/sdp\r\n"
	     "Content-Length: %d\r\n\r\n"
	     "%s",
	     cseq,
	     dateHeader(),
	     generate_RTSPURL(),
	     sdpDescriptionSize,
	     generateSDPDescription());
//printf("DESCRIBE: \n%s\n",fResponseBuffer);

}


void RtspServer::parseTransportHeader(char const* buf,
				 StreamingMode& streamingMode,
				 char*& streamingModeString,
				 char*& destinationAddressStr,
				 unsigned& destinationTTL,
				 unsigned short& rtpPort, // if UDP
				 unsigned short& rtcpPort, // if UDP
				 unsigned char& rtpChannelId, // if TCP
				 unsigned char& rtcpChannelId // if TCP
				 ){
  
  // Initialize the result parameters to default values:
  streamingMode = RTP_UDP;
  streamingModeString = NULL;
  destinationAddressStr = NULL;

  unsigned ttl, rtpCid, rtcpCid;
  unsigned short p1,p2;

  // First, find "Transport:"
  while (1) {
    if (*buf == '\0') return; // not found
    if (_strnicmp(buf, "Transport: ", 11) == 0) break;
    ++buf;
  }

  // Then, run through each of the fields, looking for ones we handle:
  char const* fields = buf + 11;
  char* field = strDupSize(fields);
  
  while (sscanf(fields, "%[^;]", field) == 1) {
  
	if (strcmp(field, "RTP/AVP/TCP") == 0) {
      streamingMode = RTP_TCP;
    } else if (strcmp(field, "RAW/RAW/UDP") == 0 ||
	       strcmp(field, "MP2T/H2221/UDP") == 0) {
      streamingMode = RAW_UDP;
      streamingModeString = strDup(field);
    } else if (_strnicmp(field, "destination=", 12) == 0) {
      delete[] destinationAddressStr;
      destinationAddressStr = strDup(field+12);
    } else if (sscanf(field, "ttl%u", &ttl) == 1) {
      destinationTTL = ttl;
    } else if (sscanf(field, "client_port=%hu-%hu", &p1, &p2) == 2) {
	rtpPort = p1;
	rtcpPort = p2;
    } else if (sscanf(field, "client_port=%hu", &p1) == 1) {
	rtpPort = p1;
	rtcpPort = streamingMode == RAW_UDP ? 0 : p1 + 1;
    }

    fields += strlen(field);
    while (*fields == ';') ++fields; // skip over separating ';' chars
    if (*fields == '\0' || *fields == '\r' || *fields == '\n') break;
  }
  delete[] field;
}


void RtspServer::handle_SETUP_cmd(char const* cseq,
		  char const* urlPreSuffix, char const* urlSuffix,
		  char const* fullRequestStr) {
  // "urlPreSuffix" should be the session (stream) name, and
  // "urlSuffix" should be the subsession (track) name.
  char const* streamName = urlPreSuffix;
  char const* trackId = urlSuffix;

  char* streamingModeString;
  char* destinationAddressStr;
  unsigned int destinationTTL;
  unsigned short rtpPort;
  unsigned short rtcpPort;

  StreamingMode streamingMode;
  char* clientsDestinationAddressStr;
  unsigned char rtpChannelId, rtcpChannelId;

  // Look for a "Transport:" header in the request string,
  // to extract client parameters:
  unsigned int tmpbla=255;
  unsigned char bla1='1';
  unsigned char bla2='1';


  parseTransportHeader(fullRequestStr, streamingMode, streamingModeString,
		       clientsDestinationAddressStr, tmpbla,
		       rtpPort, rtcpPort,
		       bla1, bla2);

  delete[] clientsDestinationAddressStr;

  switch (streamingMode) {
    case RTP_UDP: {
      _snprintf((char*)fResponseBuffer, sizeof fResponseBuffer,
	       "RTSP/1.0 200 OK\r\n"
	       "CSeq: %s\r\n"
	       "%s"
	       "Transport: RTP/AVP;unicast;destination=%s;client_port=%d-%d;server_port=%d-%d\r\n"
	       "Session: %d\r\n\r\n",
	       cseq,
	       dateHeader(),
	       "192.168.0.2", 5010, 5011, 5020, 5021, // fiXX by SkyRaVeR gotta by dynamicaly
	       "1234567890");//fiXX
      break;
    }
    case RAW_UDP: {
      _snprintf((char*)fResponseBuffer, sizeof fResponseBuffer,
	       "RTSP/1.0 200 OK\r\n"
	       "CSeq: %s\r\n"
	       "%s"
	       "Transport: %s;unicast;destination=%s;client_port=%d;server_port=%d\r\n"
	       "Session: %d\r\n\r\n",
	       cseq,
	       dateHeader(),
	       streamingModeString, "192.128.0.2", ntohs(rtpPort), ntohs(rtcpPort),
	       "192.168.0.2");
      delete[] streamingModeString;
      break;
    }
  }

  //printf("SETUP-String: \n%s\n",fResponseBuffer);		  
}

void RtspServer::handle_withinSession_cmd(char const* cmdName,
			  char const* urlPreSuffix, char const* urlSuffix,
			  char const* cseq, char const* fullRequestStr) {
  // This will either be:
  // - a non-aggregated operation, if "urlPreSuffix" is the session (stream)
  //   name and "urlSuffix" is the subsession (track) name, or
  // - a aggregated operation, if "urlSuffix" is the session (stream) name,
  //   or "urlPreSuffix" is the session (stream) name, and "urlSuffix"
  //   is empty.
  // First, figure out which of these it is:
/*
   if (fOurServerMediaSession == NULL) { // There wasn't a previous SETUP!
    handleCmd_notSupported(cseq);
    return;
  }
*/
  if (strcmp(cmdName, "TEARDOWN") == 0) {
    handle_TEARDOWN_cmd(cseq);
  } else if (strcmp(cmdName, "PLAY") == 0) {
    handle_PLAY_cmd(cseq, fullRequestStr);
  } else if (strcmp(cmdName, "PAUSE") == 0) {
    handle_PAUSE_cmd(cseq);
  } else if (strcmp(cmdName, "GET_PARAMETER") == 0) {
    handle_GET_PARAMETER_cmd(cseq, fullRequestStr);
  }
}


// ####################### PLAY #########################################
bool RtspServer::parseRangeHeader(char const* buf, float& rangeStart, float& rangeEnd) {
  // Initialize the result parameters to default values:
  rangeStart = rangeEnd = 0.0;

  // First, find "Range:"
  while (1) {
    if (*buf == '\0') return 0; // not found
    if (_strnicmp(buf, "Range: ", 7) == 0) break;
    ++buf;
  }

  // Then, run through each of the fields, looking for ones we handle:
  char const* fields = buf + 7;
  while (*fields == ' ') ++fields;
  float start, end;
  if (sscanf(fields, "npt = %f - %f", &start, &end) == 2) {
    rangeStart = start;
    rangeEnd = end;
  } else if (sscanf(fields, "npt = %f -", &start) == 1) {
    rangeStart = start;
  } else {
    return 0; // The header is malformed
  }

  return 1;
}

bool RtspServer::parseScaleHeader(char const* buf, float& scale) {
  // Initialize the result parameter to a default value:
  scale = 1.0;

  // First, find "Scale:"
  while (1) {
    if (*buf == '\0') return 0; // not found
    if (_strnicmp(buf, "Scale: ", 7) == 0) break;
    ++buf;
  }

  // Then, run through each of the fields, looking for ones we handle:
  char const* fields = buf + 7;
  while (*fields == ' ') ++fields;
  float sc;
  if (sscanf(fields, "%f", &sc) == 1) {
    scale = sc;
  } else {
    return 0; // The header is malformed
  }

  return 1;
}

void RtspServer::handle_PLAY_cmd(char const* cseq,
							     char const* fullRequestStr) {
  
  char buf[100];
  char* scaleHeader;

  //// Parse the client's "Scale:" header, if any: 
  float scale;
  bool sawScaleHeader = parseScaleHeader(fullRequestStr, scale);

  buf[0] = '\0'; // Because we didn't see a Scale: header, don't send one back
  sprintf(buf, "Scale: %f\r\n", 0);
  scaleHeader = strDup(buf);

  //// Parse the client's "Range:" header, if any: 
  float rangeStart, rangeEnd;
  float duration;
  bool sawRangeHeader = parseRangeHeader(fullRequestStr, rangeStart, rangeEnd);

  // Use this information, plus the stream's duration (if known), to create
  // our own "Range:" header, for the response:
  duration = 0.0;

  if (rangeEnd < 0.0 || rangeEnd > duration) rangeEnd = duration;
  if (rangeStart < 0.0) {
    rangeStart = 0.0;
  } else if (rangeEnd > 0.0 && scale > 0.0 && rangeStart > rangeEnd) {
    rangeStart = rangeEnd;
  }

  char* rangeHeader;
  if (!sawRangeHeader) {
    buf[0] = '\0'; // Because we didn't see a Range: header, don't send one back
  } else if (rangeEnd == 0.0 && scale >= 0.0) {
    sprintf(buf, "Range: npt=%.3f-\r\n", rangeStart);
  } else {
    sprintf(buf, "Range: npt=%.3f-%.3f\r\n", rangeStart, rangeEnd);
  }
  rangeHeader = strDup(buf);

  // Create a "RTP-Info:" line.  It will get filled in from each subsession's state:
  char const* rtpInfoFmt =
    "%s" // "RTP-Info:", plus any preceding rtpInfo items
    "%s" // comma separator, if needed 
    "url=%s/%s"
    ";seq=%d";

  unsigned rtpInfoFmtSize = strlen(rtpInfoFmt);
  char* rtpInfo = strDup("RTP-Info: ");
  unsigned i, numRTPInfoItems = 0;

  rtpInfo[0] = '\0';

  // Fill in the response:
  _snprintf((char*)fResponseBuffer, sizeof fResponseBuffer,
	   "RTSP/1.0 200 OK\r\n"
	   "CSeq: %s\r\n"
	   "%s"
	   "%s"
	   "%s"
	   "Session: %d\r\n"
	   "%s\r\n",
	   cseq,
	   dateHeader(),
	   scaleHeader,
	   rangeHeader,
	   "1234567890", // FiXXX gotta by dynamic
	   rtpInfo);
  delete[] rtpInfo; delete[] rangeHeader;
  delete[] scaleHeader;

  printf("Play : \n%s\n",fResponseBuffer);
}

void RtspServer::handle_PAUSE_cmd(char const* cseq) {
  /* falls mal vom file - file anhalten,,, */
	_snprintf((char*)fResponseBuffer, sizeof fResponseBuffer,
	   "RTSP/1.0 200 OK\r\nCSeq: %s\r\n%sSession: %d\r\n\r\n",
	   cseq, dateHeader(), "1234567890"); // sessionid... fixx
}


void RtspServer::handle_TEARDOWN_cmd(char const* cseq) {
  _snprintf((char*)fResponseBuffer, sizeof fResponseBuffer,
	   "RTSP/1.0 200 OK\r\nCSeq: %s\r\n%s\r\n",
	   cseq, dateHeader());
  //printf("teardown : \n%s\n"),fResponseBuffer);
}


void RtspServer::handle_GET_PARAMETER_cmd(char const* cseq,char const* /*fullRequestStr*/) {
  // We implement "GET_PARAMETER" just as a 'keep alive',
  // and send back an empty response:
  _snprintf((char*)fResponseBuffer, sizeof fResponseBuffer,
	   "RTSP/1.0 200 OK\r\nCSeq: %s\r\n%sSession: %d\r\n\r\n",
	   cseq, dateHeader(), "1234567890");
}

void RtspServer::handle_bad_cmd(char const* /*cseq*/) {
  // Don't do anything with "cseq", because it might be nonsense
  _snprintf((char*)fResponseBuffer, sizeof fResponseBuffer,
	   "RTSP/1.0 400 Bad Request\r\n%sAllow: %s\r\n\r\n",
	   dateHeader(), allowedCommandNames);
  printf("Bad COMMAND !!! sending answer: \n%s\n",fResponseBuffer);
  exit(1);
}

void RtspServer::handle_notSupported_cmd(char const* cseq) {
  _snprintf((char*)fResponseBuffer, sizeof fResponseBuffer,
	   "RTSP/1.0 405 Method Not Allowed\r\nCSeq: %s\r\n%sAllow: %s\r\n\r\n",
	   cseq, dateHeader(), allowedCommandNames);
 }

bool RtspServer::initSocket() {
// falls windows erstmal den winsock initialisieren
	long rc;
	#if defined(__WIN32__) || defined(_WIN32)
	rc = WSAStartup(MAKEWORD(2,0),&wsa);
	
	if(rc!=0){
		printf("[-] Error starting Winsock ! (code: %d)\n",rc);
	} else {
		printf("[+] Successfully started winsock !\n");
	}
	#endif

	// socket grabben
	sock = socket(AF_INET, SOCK_STREAM, 0);

  
	if(sock == -1) {
		#if defined(__WIN32__) || defined(_WIN32)
		printf("[-] Error ! Couldn't create socket! (code: %d)\n",WSAGetLastError());
		#else
		printf("[-] Error ! Couldn't create socket! (code: %d)\n",sock);
		#endif
	
	} else {
		printf("[+]TCP Socket erstellt!\n");
	}
	memset((char *) &tcpserver, 0, sizeof(tcpserver)); // speicher f�r uns reservieren :)
	
	tcpserver.sin_family=AF_INET;
	tcpserver.sin_port = htons(rtspPort); 
	tcpserver.sin_addr.s_addr = INADDR_ANY;

    if (bind(sock, (struct sockaddr *)&tcpserver, sizeof(tcpserver))==-1) {
      fprintf(stderr, "[-] Error: bind\n");
      return 0;
    }

}

void RtspServer::tcplisten() {
	
	if (listen(sock, 1)==-1) {
      fprintf(stderr, "[-] Error: listen\n");
    }

    size=sizeof(remote_addr);
	
	remote_s = accept(sock, (struct sockaddr *)&remote_addr, &size); // accept incomming connections...

	printf("\nincoming connection from %s\n",inet_ntoa(remote_addr.sin_addr));

	while (1) {  
	
		size=recv(remote_s, buffer,1024,0);
		
		if (size==-1) {
			fprintf(stderr, "error while receiving\n");
		} else {
			//printf("Received %d bytes\nIncomming MSG: \n%s\n", size,buffer);
			handle_INCOMING_request((unsigned char*)buffer,size);
		}
	}
}
/*
int _tmain(int argc, _TCHAR* argv[])
{
	RtspServer* test = new RtspServer();
	test->tcplisten();
}
*/
