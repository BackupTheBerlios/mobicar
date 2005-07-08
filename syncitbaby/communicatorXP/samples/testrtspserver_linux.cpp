#include <stdio.h>
#include "rtspserver.h"

int main() {

	RtspServer* test = new RtspServer();
	
	test->setLocalIP("192.168.0.2");
	test->setRTPAudioPort(5010);
	test->setRTPVideoPort(5020);
	test->setRtspPort(554);
	test->setSessionDescription("testit");
	test->setStreamOrigin("testit");
	test->tcplisten();
 	
}
