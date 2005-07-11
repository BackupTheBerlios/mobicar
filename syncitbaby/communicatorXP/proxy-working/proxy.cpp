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
#define USE_RTSPSERVER 1
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

  //uint8_t outBuf[100000];
  
  //memcpy(outBuf, inPacket+5, length -5);
  //for(unsigned j=0;j<length-16;j++)
  //  printf("%c", inPacket[j+16]);
    //printf("Length was : %u\n\n", length -5);

  //test->sendData(inPacket+5, length-6);
  

}


#ifdef USE_RTSPSERVER
void *startRTSPserver(void *threadid) {
    //printf("RTSP-Server is running!\n\n");
  
    RtspServer* rtspserver = new RtspServer();
    
    rtspserver->setLocalIP("127.0.0.1");
    rtspserver->setRTPAudioPort(6010);
    rtspserver->setRTPVideoPort(6020);
    rtspserver->setRtspPort(6000);
    //rtspserver->setSessionDescription("");
    //rtspserver->setStreamOrigin("testit");
    rtspserver->tcplisten();
    
  pthread_exit(NULL);
}
#endif

#ifdef USE_COMMRECEIVE
void *startCOMMreceive(void *threadid) {
    
    //printf("COMM-Receive is running!\n\n"); 
      
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

    //printf("[+] COMM-Stream is running!\n\n"); 
    
    	CommunicatorXP* test = new CommunicatorXP();
	test->setDestination("192.168.123.126");
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
    
    uint8_t infoByte;
    bool sliceBegins;
    bool sliceEnds;
    
    //bool sawFirstVideoSequence = 0;
    
    //uint32_t i;
    uint32_t currOffset = 0;
    
    uint8_t oneByte[1];
    uint32_t readByte = 0;
    
    
    while(1) {
    
    	int offset = 0;
	while (true) {
	
		//offset = mutexBufferV2->read(cachingBuffer, 10000, offset);
		offset = mutexBufferV2->read(oneByte, 1, offset);
		//printf("Offset is: %i\n", offset);
		if (offset != -1) {
		        //printf("Sleeping\n");
		        ; //usleep(1);
		}

		else {
		        readByte++;
			//printf("Breaking and readByte is %u, oneByte is %02X\n", readByte, oneByte[0]);
		        break;
		}
	}
    
    

    
    
    //printf("Read Bytes: %u\n", readByte);
    cachingBuffer[readByte-1] = oneByte[0];
    if(readByte>=4) {
      //printf("readByte is %u\n", readByte);
      firstByte  = cachingBuffer[readByte - 4];
      secondByte = cachingBuffer[readByte - 3];
      thirdByte  = cachingBuffer[readByte - 2];
      fourthByte = cachingBuffer[readByte - 1];
      
    
     if(firstByte==0x00 && secondByte==0x00 &&thirdByte==0x01 && fourthByte==0xB4) {
       //printf("SAW FRAME and read %u Bytes\n", readByte);
       if(readByte > 0) {
         if(cachingBuffer[18]==0x00 && cachingBuffer[19]==0x01 && cachingBuffer[20]==0xB3) {
	   //printf("Saw Video Sequence!\n");
	   sawFirstVideoSequence = 1;
	 }
	 
         if(cachingBuffer[18]==0x00 && cachingBuffer[19]==0x01 && cachingBuffer[20]==0x00) {
	   ; //printf("Saw Picture!\n");
         }

	 if(cachingBuffer[106]==0x00 && cachingBuffer[107]==0x01 && cachingBuffer[108]==0xB8) {
	   //printf("Saw GOP!\n");
	   //is GOP
	   /*
	   unsigned next4Bytes = (cachingBuffer[109]<<24) + (cachingBuffer[110]<<16) + (cachingBuffer[111]<<8) + cachingBuffer[112];
	   unsigned time_code = (next4Bytes&0xFFFFFF80)>>(32-25);

           bool drop_frame_flag     = (time_code&0x01000000) != 0;

           unsigned time_code_hours    = (time_code&0x00F80000)>>19;
           unsigned time_code_minutes  = (time_code&0x0007E000)>>13;
           unsigned time_code_seconds  = (time_code&0x00000FC0)>>6;
           unsigned time_code_pictures = (time_code&0x0000003F);

           printf("time_code: 0x%07x, drop_frame %d, hours %d, minutes %d, seconds %d, pictures %d\n", time_code, drop_frame_flag, time_code_hours, time_code_minutes, time_code_seconds, time_code_pictures);
	   */
	   }
	   
	   //printf("Found Vid SEq HEader\n");
       	//printf("%02X %02X %02X\n\n",cachingBuffer[18], cachingBuffer[19], cachingBuffer[20]);
	//for(unsigned j=0;j<10;j++)
	 // printf("%02X ", cachingBuffer[21+j]);
	  //printf("\n\n");
       //for(unsigned i=0;i<readByte-21;i++)
       //  printf("%02X ", cachingBuffer[i+17]);
       //printf("\n\n\n");
         if(sawFirstVideoSequence) {
	   infoByte = cachingBuffer[15] & 0xF7;
	   infoByte = infoByte | 0x8;
	   cachingBuffer[15] = infoByte;
	   //sliceBegins = (infoByte>>4) & 0xF;
           //sliceEnds   = (infoByte>>3) & 0XF;
	   //if(!sliceEnds) {
	     
	   //printf("Sequence Number: %02X %02X - frame length is %u - slice begins: %d - slice ends: %d - hex: %02X\n",cachingBuffer[3], cachingBuffer[4], readByte-4, sliceBegins, sliceEnds, infoByte);
	//}
	   
           //test->sendData(cachingBuffer+17, readByte-21); //sending raw es
           test->sendData(cachingBuffer+1, readByte-4); // sending rtp with old header
           //for(unsigned j=0;j<readByte-21;j++)
           //  printf("%c", cachingBuffer[j+17]);
         }
       }
       
       
       readByte = 0;
     }
      
      
    }
}
    
}   
#endif
    
#ifdef USE_GUI
void *startGUI(void *threadid) {
  //printf("GUI is running!\n\n");
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
  //dbg.set_state(LOG_COUT);
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
  //printf("[+] Client detected @ %s, trying to get WLAN-Data..\n",destip);

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
  
  //printf("\n[**] Threading:\n\n");
 
  
  #ifdef USE_RTSPSERVER
  //printf("[**] starting RTSP-Server...\n");
  err = pthread_create(&thread_rtspserver,NULL,startRTSPserver,(void *)t);
  #else
  //printf("[-] RTSP-Server disabled\n\n");
  #endif
  
  #ifdef USE_COMMRECEIVE
  //printf("[**] starting COMM-RECEIVE...\n");
  err = pthread_create(&thread_commreceive,NULL,startCOMMreceive,(void *)t);
  #else
  //printf("[-] RTSP-Server disabled\n\n");
  #endif
  
  #ifdef USE_COMMSTREAM
  //printf("[**] starting COMM-RECEIVE...\n");
  err = pthread_create(&thread_commstream,NULL,startCOMMStream,(void *)t);
  #else
  //printf("[-] RTSP-Server disabled\n\n");
  #endif
  
  #ifdef USE_GUI
  //printf("[**] starting GUI...\n");
  err = pthread_create(&thread_gui,NULL,startGUI,(void *)t);
  #else
  //printf("[-] gui disabled\n\n");
  #endif
  
  #ifdef USE_TRANSCODER
  //printf("[**] starting transcoder...\n");
  err = pthread_create(&thread_transcoder,NULL,startTranscoder,(void *)t);
  sleep(1);
  #else
  //printf("[-] transcoder disabled\n\n");
  #endif

  #ifdef USE_WLAN
  //printf("[**] starting Wlan client...\n");
  err = pthread_create(&thread_wlan,NULL,startWlan,(void *)t);
  #else
 // printf("[-] Wlan client disabled ...\n\n");
  #endif

  if(err) {
    //printf("ERROR;return code from pthread_create() is %d\n",err);
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
