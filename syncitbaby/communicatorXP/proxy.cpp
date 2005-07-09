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
** This file represents the proxy application for the video@home-project.
**
** TODO:
**  - add some useful commandline parameters
**  - dynamic thread handling
**  - cleanup on SIGINT
**
** NOTICE:
** @SkyRaVeR: do not forget to read from the filled videobuffer (vbuffy instead of vbuffy2 ;)) when you shut down the transcoder ...
**
**
** Author: W. Pantke, M. Arz, M. Klama (all Communication)
** Date:   n/a
**
** 
** Modifications:
** Date:           Name:        Changes:   
** 12-06-05        pantke       initial code
** 13-06-05        pantke       some tweaks
** 14-06-05        pantke       changed to use CommunicatorV2 incl new configuration
** 20-06-05        klama/pantke added multithreading for CommunicatorV2, added transcoder, did cleanup and included wlan
** 21-06-05        rose/arz     added smremote
** 21-06-05        pantke       added destination ip-change detection for wlan component
** 23-06-05        pantke       added provisional SIGHandler, fixed memory leak
** 23-06-05        klama        wlan -> transcoder handshake
** 24-06-05        pantke       fixed destip-changer (don't forget todo list(OnChange-Event)), added SIGINT-workaround (common *nix threading bug), changed transcoder call
** 25-06-05        arz          changed sm registration
** Jun. 26, 2005   pantke       added global changeDestination() method (removed "destip-hack"), added commandlineparameter and usage information, added preprocessor statements
** 27-06-05        rose/arz     service-address and service-port can be changed at runtime
** Jun. 27, 2005   pantke       changed to use new derived transcoder quality manager, -> recovered overwritten parts... next time, first UPDATE and THEN commit!! <-
** Jun. 28, 2005   pantke       added GUI integration ;o)
** Jul. 03, 2005   klama        changed buffer to new mutexbuffer
** Jul. 03, 2005   pantke       corrected buffer integration, cleaned directory structure, now including of h-files only.. at last!!
** Jul. 06, 2005   rose/arz     changed service manager remote control and corrected gui-implementation, fixed 15-char ip-bug
**
******************************************************/

#if (defined(__WIN32__) || defined(_WIN32)) && !defined(_WIN32_WCE)
#include "stdafx.h"
#endif

//threading
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>


//#define USE_GUI 1
#define USE_TRANSCODER 1
//#define USE_WLAN 1
//#define USE_RTSPSERVER 1
#define USE_COMMRECEIVE 1
#define USE_COMMSTREAM 1



#include <iostream>

//include the communication classes
#include "BufferClass.h"
#include "CommunicatorXP.cpp"
#include "rtspserver.cpp"

// transcoder
#include "CMPEG2_iomanager_buffer.h"
#include "CMPEG2_quality_manager_wlan.h"
#include "mpeg2.hpp"
#include "mpeg2_buffer.hpp"
#include "mpeg2_bitstream.hpp"
#include "mpeg2_element.hpp"
#include "mpeg2_header.hpp"
#include "mpeg2_parser.hpp"
#include "mpeg2_log.hpp"
#include "mpeg2_manager.hpp"

// Wlanclient
#include "wlan_sender.h"

// SIGINT, exit
#include <csignal>

// gui
//#include "proxy_gui.h"

using namespace std;


typedef unsigned char uint8_t;
typedef unsigned int uint32_t;
typedef unsigned short uint16_t;


char const* progName = NULL;

BufferClass* mutexBufferA   = new BufferClass();
BufferClass* mutexBufferV   = new BufferClass();
BufferClass* mutexBufferV2  = new BufferClass();

Wlan* wlan = new Wlan;

char* destip = "127.0.0.1";
bool sigint = false;
void sighandler(int signal); // forward
void usage(); //forward


void packetSender(uint8_t *inPacket, uint32_t length, CommunicatorXP *test) {

  uint8_t outBuf[100000];
  
  //memcpy(outBuf, inPacket+5, length -5);

  test->sendData(inPacket+5, length-6);
  

}


#ifdef USE_RTSPSERVER
void *startRTSPserver(void *threadid) {
    printf("RTSP-Server is running!\n\n");
  
    RtspServer* rtspserver = new RtspServer();
    
    rtspserver->setLocalIP("0.0.0.0");
    rtspserver->setRTPAudioPort(5010);
    rtspserver->setRTPVideoPort(5020);
    rtspserver->setRtspPort(554);
    //rtspserver->setSessionDescription("");
    //rtspserver->setStreamOrigin("testit");
    rtspserver->tcplisten();
    
  pthread_exit(NULL);
}
#endif

#ifdef USE_COMMRECEIVE
void *startCOMMreceive(void *threadid) {
    
    printf("COMM-Receive is running!\n\n"); 
      
    CommunicatorXP* testr = new CommunicatorXP();

    testr->setLocalPort(5020); // do BEFORE u init the receiver!!!!
    testr->initUDPReceiver();  // initialize..
    while(1) {
        testr->receiveData();// finally start listening for data,,,
    }
    
    delete testr;     
}
#endif

#ifdef USE_COMMSTREAM
void *startCOMMStream(void *threadid) {

    printf("[+] COMM-Stream is running!\n\n"); 
    
    	CommunicatorXP* test = new CommunicatorXP();
	test->setDestination("127.0.0.1");
	test->setPort(6020);
    
      /* First we have to define the vars for start_sequence_code checking routine */
    uint8_t firstByte;
    uint8_t secondByte;
    uint8_t thirdByte;
    uint8_t fourthByte;
    
    uint8_t cachingBuffer[10000];
    uint8_t frameOut[10000];
    uint8_t fragmentedPacket[10000];
    uint32_t fragmentedPacketLength;
    bool isFragmented;
    
    bool sawFirstVideoSequence = 0;
    
    uint32_t i;
    uint32_t currOffset = 0;
    
    
    while(1) {
    
    uint32_t readTest = mutexBufferV2->read(cachingBuffer, 10000, 0); 
    
    if(readTest!=-1) {
      sleep(1);
      //printf("Sleeping cause %u\n",readTest);
    } 

//00 01 B3 [22] ..
    
    //well, our buffer is filled ... lets begin to frame it ...
    
    for(i=0;i<10000;i++) {
    
      if(i>=3) {
        //read of course 4 bytes ...
        firstByte  = cachingBuffer[i-3];
        secondByte = cachingBuffer[i-2];
        thirdByte  = cachingBuffer[i-1];
        fourthByte = cachingBuffer[i];
      }
      
      if(firstByte==0x00 && secondByte==0x00 &&thirdByte==0x01 && fourthByte==0xB4) {
      
        //printf("SAW SOME B4 HAXXORED START CODE HERE ... \n");
	/*
	printf("Inhalt at offset %u is :\n", i);
	for(unsigned j=0;j<21;j++)
	  printf("%02X ",cachingBuffer[j+i-3]);
	printf("\n\n\n");
	*/
	//printf("wert 1 %u - i ist %u - wert 2 %u\n",currOffset, i,  i-currOffset);
	memcpy(frameOut,cachingBuffer+currOffset, i-currOffset -3);
		
	//printf("%02X %02X %02X\n\n",cachingBuffer[i-3+22], cachingBuffer[i-3+23], cachingBuffer[i-3+24]);
	//(cachingBuffer[i-3+24]==0xB3) {
	
	if(cachingBuffer[i-3+22]==0x00 && cachingBuffer[i-3+23]==0x01 && cachingBuffer[i-3+24]==0xB3)
	  sawFirstVideoSequence = 1; //printf("START SEQUENCE \n\n");
	
	//sawFirstVideoSequence
	if(sawFirstVideoSequence) {
	if(isFragmented) {
	  memcpy(fragmentedPacket+fragmentedPacketLength-1, frameOut, (i-currOffset-3));
	  packetSender(fragmentedPacket, (fragmentedPacketLength + (i-currOffset -3)), test);
	  for(unsigned z=0;z<(fragmentedPacketLength + (i-currOffset -3));z++)
	    printf("%02X ", fragmentedPacket[z]);
	    
	  isFragmented = 0;
	}
	else {
	  packetSender(frameOut, (i-currOffset -3), test);
	  //test->sendData(frameOut, (i-currOffset -3));
	  //for(uint32_t t=0;t < (i - currOffset -3); t++) 
	  //  printf("%02X ",frameOut[t]);
	  //printf("\n\n\n");
	  }
	}
	
	//
	//printf("Current offset is: %u\n",currOffset);
	currOffset = i-3;
	  
      }
      
      if(i+1==10000 && sawFirstVideoSequence) {
        //printf("Reached end of buffer\n");
	//printf("Offset is: %u - i is %u\n",currOffset, i);
	//Offset is: 9315 - i is 9999
	//uint8_t fragmentedPacket[10000];
        //uint32_t fragmentedPacketLength;
	memcpy(fragmentedPacket,cachingBuffer+currOffset,(i-currOffset-1));
	fragmentedPacketLength = i - currOffset +1; 
	isFragmented = 1;
	
	//for(uint32_t e=0; e< (i - currOffset); e++)
	//  printf("%02X ", cachingBuffer[currOffset+e]);
	//printf("END OF THIS END ...\n\n");
	currOffset = 0;
        //break;
      }
	
    
    
    
    }
    
    }
      
    /*  
    unsigned char data[1];
    unsigned char* esBuffer;
    unsigned char dataPes[65565+6];
    
    esBuffer = (unsigned char*) malloc(100000000);
    
    int i;
        int len = 0;        
        int gettingPes; 
	unsigned readTest;  
         while(1){
            gettingPes=5;
	    //while(readTest==-1) {
              readTest = mutexBufferV2->read(data, 1, 0); 
	    //}
	    if(readTest!=-1) {
	      sleep(1);
	      printf("Sleeping cause %u\n",readTest);
	    }
	    
            printf("%02X ",data[0]);
         }

              
    
    
    
        
    //for (int i=0; i<40;i++) printf(  
    */
         
}   
#endif
    
#ifdef USE_GUI
void *startGUI(void *threadid) {
  printf("GUI is running!\n\n");
  proxy_gui();
  pthread_exit(NULL);
}   
#endif
    
#ifdef USE_TRANSCODER
void *startTranscoder(void *threadid) {
  CMPEG2_transcoder_manager   transcoder;
  CMPEG2_iomanager_buffer     iomngr;
  CMPEG2_quality_manager_wlan qmanager;
    
  dbg.set_state(LOG_OFF);
  iomngr.set_ibuffer(mutexBufferV);             // incoming buffer from receiver
  iomngr.set_obuffer(mutexBufferV2);            // outgoing buffer to streamer
  transcoder.init(&iomngr, &qmanager);
    
  while(transcoder.parse() != STATE_UNDEFINED);
}   
#endif
    
#ifdef USE_WLAN
void *startWlan(void *threadid) {
  
  while(!strcmp(destip,"127.0.0.1")){
    sleep(1);
  }
  WlanReceiver wlanrecv;
  printf("[+] Client detected @ %s, trying to get WLAN-Data..\n",destip);

  while(wlanrecv.receive(destip,wlan)){
    sleep(1);
  }
  pthread_exit(NULL);
}
#endif

int main (int argc, char *argv[]) {
  signal(SIGINT, sighandler);

  progName = argv[0];

//  vBuffy = bufferClass::init(); deprecated.. constructor does work now
  
  wlan->initializeWlan();

  pthread_t thread_gui;
  pthread_t thread_transcoder;
  pthread_t thread_wlan;
  pthread_t thread_rtspserver;
  pthread_t thread_commreceive;    
  pthread_t thread_commstream;    
  
  
  int err,t=0;
  
  printf("\n[**] Threading:\n\n");
 
  
  #ifdef USE_RTSPSERVER
  printf("[**] starting RTSP-Server...\n");
  err = pthread_create(&thread_rtspserver,NULL,startRTSPserver,(void *)t);
  #else
  printf("[-] RTSP-Server disabled\n\n");
  #endif
  
  #ifdef USE_COMMRECEIVE
  printf("[**] starting COMM-RECEIVE...\n");
  err = pthread_create(&thread_commreceive,NULL,startCOMMreceive,(void *)t);
  #else
  printf("[-] RTSP-Server disabled\n\n");
  #endif
  
  #ifdef USE_COMMSTREAM
  printf("[**] starting COMM-RECEIVE...\n");
  err = pthread_create(&thread_commstream,NULL,startCOMMStream,(void *)t);
  #else
  printf("[-] RTSP-Server disabled\n\n");
  #endif
  
  #ifdef USE_GUI
  printf("[**] starting GUI...\n");
  err = pthread_create(&thread_gui,NULL,startGUI,(void *)t);
  #else
  printf("[-] gui disabled\n\n");
  #endif
  
  #ifdef USE_TRANSCODER
  printf("[**] starting transcoder...\n");
  err = pthread_create(&thread_transcoder,NULL,startTranscoder,(void *)t);
  sleep(1);
  #else
  printf("[-] transcoder disabled\n\n");
  #endif

  #ifdef USE_WLAN
  printf("[**] starting Wlan client...\n");
  err = pthread_create(&thread_wlan,NULL,startWlan,(void *)t);
  #else
  printf("[-] Wlan client disabled ...\n\n");
  #endif

  if(err) {
    printf("ERROR;return code from pthread_create() is %d\n",err);
    exit(-1);
  }

  while(true) {
	sleep(100);
  }
  pthread_exit(NULL);
  return 0;  
}





void sighandler(int signal){
  if (signal == SIGINT){
    if(!sigint){
      sigint = true;
      cout << "STRG-C detected! Cleaning up..." << endl;
      delete wlan;
      delete mutexBufferA;
      delete mutexBufferV;
      delete mutexBufferV2;
      exit(0);
    }
    else{
      cout << "More than one STRG+C detected.. the common *nix threading bug? [Ignoring]" << endl;
    }
  }
  else{
    psignal(signal, "Unbekanntes Signal: ");
  }
} 



void usage() {
	cout << "\nUsage: " << progName
	    << " [-sm|--service-manager <IP>]\n\n"
	    << "e.g. " << progName << " --service-manager 127.0.0.1\n";

	exit(2);

}