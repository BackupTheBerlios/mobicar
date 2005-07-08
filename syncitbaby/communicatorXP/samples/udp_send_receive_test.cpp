#include "stdafx.h"
#include "CommunicatorXP.h"

int _tmain(int argc, _TCHAR* argv[])
{

// Sample for udp-send....
	
	/*CommunicatorXP* test = new CommunicatorXP();

	test->setDestination("192.168.0.2"); 
	test->setPort(3000);  // set before initing (but not neccessary - just before ya call sendData()
	test->initUDPSender();
	test->sendData((unsigned char*)"Manu",4);
	delete test;*/

// Sample for udp-receive....
/*
	CommunicatorXP* testr = new CommunicatorXP();

	testr->setLocalPort(3333); // do BEFORE u init the receiver!!!!
	testr->initUDPReceiver();  // initialize..
	testr->receiveData();	  // finally start listening for data,,,
	delete testr;
  */
	return 0;
}