#include "stdafx.h"
#include "CommunicatorXP.h"

int _tmain(int argc, _TCHAR* argv[])
{

// Sample for udp-send....
	CommunicatorXP* test = new CommunicatorXP();

	test->setDestination("192.168.0.2"); 
	test->setPort(3000);  // set before initing (but not neccessary - just before ya call sendData()
	test->initUDPSender();
	test->sendData((unsigned char*)"Manu",4);
	delete test;

// Sample for udp-receive....
	CommunicatorXP* testr = new CommunicatorXP();

	testr->setLocalPort(3333); // do BEFORE u init the receiver!!!!
	testr->initUDPReceiver();  // initialize..
	testr->receiveData();	  // finally start listening for data,,,
	delete testr;
  


	return 0;
}



// client fun for tcp
/*
was not commented but fun 2
	long bla;
	unsigned char* blupp;

	test->setDestination("192.168.0.2");
	test->setPort(3333);
	blupp = (unsigned char*)"Manu";
	bla = test->senddata(blupp,4);
	printf("Sent: %i bytes",bla);


  struct sockaddr_in host_addr;
  int size;
  int s;
  struct hostent *host;
  char hostname[20];
  char buffer[1000];

  printf("\nEnter Hostname: ");
  scanf("%s",&hostname);

  host=gethostbyname(hostname);
  if (host==NULL)
    {
      fprintf(stderr, "Unknown Host %s\n",hostname);
      return -1;
    }

  fflush(stdout);
  s = socket(AF_INET, SOCK_STREAM, 0);
  if (s < 0)
    {
      fprintf(stderr, "Error: socket\n");
      return -1;
    }
  host_addr.sin_family = AF_INET;
  host_addr.sin_addr = *((struct in_addr *)host->h_addr);
  host_addr.sin_port = htons(5000);
  if (connect(s, (struct sockaddr *)&host_addr, sizeof(host_addr))==-1)
    {
      fprintf(stderr, "Error: connect\n");
      return -1;
    }
	send(s,"OPTIONS rtsp://192.168.0.2/testStream RTSP/1.0\n",47,0);
	send(s,"CSeq: 1\n",8,0);
	send(s,"User-Agent: SKY\n",16,0);

  size = recv(s, buffer, 1000, 0);
  if (size==-1)
    {
      fprintf(stderr, "reading data failed\n");
      return -1;
    }

  printf("Getting %d Bytes of Data\nData:%s\n",size,buffer);
*/

/*
	int tcpsock=0;
	struct sockaddr_in tcp_addr;
	struct hostent *host;
    
	host=gethostbyname("192.168.0.2");

	tcpsock = socket(AF_INET, SOCK_STREAM, 0);
    
	if (tcpsock < 0)
    {
      fprintf(stderr, "Error: socket\n");
      return -1;
    }
    
  	tcp_addr.sin_family = AF_INET;
  	tcp_addr.sin_addr = *((struct in_addr *)host->h_addr);
  	tcp_addr.sin_port = htons(5000);
   
  	if (connect(tcpsock, (struct sockaddr *)&tcp_addr, sizeof(tcp_addr))==-1)
    
	{
     // close(tcpsock);
      fprintf(stderr, "Error: connect\n");
      return -1;
    }
	sendto(tcpsock,"OPTIONS rtsp://192.168.0.2/testStream RTSP/1.0\r\n",48,0,(SOCKADDR*)&tcp_addr,sizeof(tcp_addr));
	sendto(tcpsock,"CSeq: 1\r\n",9,0,(SOCKADDR*)&tcp_addr,sizeof(tcp_addr));
	sendto(tcpsock,"User-Agent: SKY\r\n",17,0,(SOCKADDR*)&tcp_addr,sizeof(tcp_addr));
*/
