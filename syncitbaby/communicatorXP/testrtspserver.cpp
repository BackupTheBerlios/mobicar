#include "rtspserver.h"

int _tmain(int argc, _TCHAR* argv[])
{
	printf("starting server...\n");
	RtspServer* test = new RtspServer();
	test->setLocalIP("192.168.0.2");
	test->setRtspPort(554);			// if not set uses 554
	test->setRTPAudioPort(5010);		// if not set uses 5010 rtcp always +1
	test->setRTPVideoPort(5020);		// if not set uses 5020 rtcp always +1
	test->tcplisten();
	delete test;
}
