#include "stdafx.h"
#include "CommunicatorXP.h"

int _tmain(int argc, _TCHAR* argv[])
{
	CommunicatorXP* test = new CommunicatorXP();
	long bla;
	unsigned char* blupp;

	test->setDestination("192.168.0.2");
	test->setPort(3333);
	blupp = (unsigned char*)"Manu";
	bla = test->senddata(blupp,4);
	printf("Sent: %i bytes",bla);

	

	return 0;
}