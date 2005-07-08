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
** This Class handles UDP send/receive 
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
** Jul. 05, 2005   M. Klama     added reicever functions
** Jul. 06, 2005   M. Klama     added sender functions
** Jul. 07, 2005   M. Klama     added parse_sequence
** Jul. 08, 2005   M. Klama     ported to linux
**
** TODO:
** - search for fiXX and fiXX ;) 
** - RECV_BUFFER_SIZE
** - debug options
** - cleanup after receive/send
** - handle incoming data else then printf
** - fix codingstandard
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

#define RECV_BUFFER_SIZE 204800 // 200kb enough?


class CommunicatorXP {

public:
	CommunicatorXP();
	~CommunicatorXP();
	
	bool initUDPSender();
	bool initUDPReceiver();

	long sendData(unsigned char* data, int length);
	void receiveData();	// its void because uska gets refernce to obj
	
	void setPort(unsigned short port);
	unsigned short getPort();
	
	void setDestination(char* host);
	char* getDestination();
	
	void setLocalPort(unsigned short port);
	unsigned short getLocalPort();

	unsigned short getRTPSequenceNumber(unsigned char* buf);

private:

	bool initializedReceiver;
	bool initializedSender;

	int sock;	// socket
    struct sockaddr_in server; /* server address info better its called client,,, but still on the "server" */

	unsigned char* buffer;	   /* just another buffer where data shall be stored */
	unsigned short port;	   /* port of remote server to send to */
	unsigned short localport;

	char* destination;		   /* host/ip of destination */
	
	#if defined(__WIN32__) || defined(_WIN32)
	WSADATA wsa; // necessary for win
	int startWinsock(void);
	#endif

// ######### Variables for the udpserver on the proxyside
	int sockudprecv;                 
    struct sockaddr_in udpserver;      
	unsigned char* recvdata;
	unsigned int recvlength;

};
