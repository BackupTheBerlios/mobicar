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
** server application of the Video@Home project
**
** TODO:
** add commandline parameters and usage information!
** SM-handling: add flag for TYPE: SERVER, CHANNEL
**
**
** Author: W. Pantke, M. Arz, M. Klama (all Communication)
** Date:   n/a
**
** 
** Modifications:
** Date:           Name:      Changes:   
** 10-06-05        arz        projectheader and comments added
**                 pantke     cleaned up, added BufferMode
** 11-06-05        pantke     included Communicator class with Buffer support
** 12-06-05        pantke     cleaned directory structure, added AudioPort-Setting
** 13-06-05        pantke     some tweaks
** 14-06-05        pantke     changed to use CommunicatorV2 incl new configuration
** 20-06-05        arz        added servicemanager remote
** 21-06-05        rose       some changes and bugfixes, change some servicemanager remote fncts
** 21-06-05        klama      changed some commclass-management, fixxed small bug & had fun :D
** 26-06-05        pantke     added global changeDestination() method (removed "destip-hack"), beautified output ;o)
** 27-06-05        rose/arz   service-address and service-port can be changed at runtime
** Jul. 01, 2005   W. Pantke  first try to port for MacOSX
**
**
******************************************************/


#if (defined(__WIN32__) || defined(_WIN32)) && !defined(_WIN32_WCE)
#include "stdafx.h"
#endif

#define USE_PARSER 1 
//#define USE_GUI 1
//#define USE_COMMUNICATOR 1
//#define USE_REMOTESM 1

#include "server.h"

//threading
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

//include the communication classes
#include "smremote.h"

//and the dvb-parser-classes
#include "record_to_buffer.h"


// gui
#ifdef USE_GUI
#include "server_gui.h"
#endif

using namespace std;


smremote* smremote = NULL;


void *startParser(void *threadid) {
  
  dvb * test = new dvb();
  test->startParser(mBuffy);

  printf("Parser is running!\n\n");
  pthread_exit(NULL);  
}


void *startCommunicator(void *threadid)
{

  CommunicatorV2 comm;
  p_comm = &comm;

  comm.init();
  comm.setDestination("192.168.123.3");  //to be changed thru sm-remote
  //comm.setAudioBuffer(aBuffy);
  //comm.setVideoBuffer(vBuffy);
  comm.setMuxedBuffer(mBuffy);
  comm.setStreamMode(1); 	//1 = streaming from Buffer, 0 = streaming from File

  comm.setRtpAudioPort(5010);
  comm.setRtcpAudioPort(5011);
  comm.setRtpVideoPort(5020);
  comm.setRtcpVideoPort(5021);
  comm.setRtspPort(554);

  comm.setEstimatedAudioBW(160);
  comm.setEstimatedVideoBW(14000);
  comm.setRTSPMode(1);

  comm.startStream();

  pthread_exit(NULL);
}

void *startSMRemote(void *threadid) {

  while (p_comm == NULL)
    sleep(1);

  printf("[*] Initiating Service Manager Remote...\n");

  smremote = smremote::createInstance(false, p_comm);
  smremote->setupConnection(true, servicePort, serviceAddress); //to be changed thru gui


  smremote->doJobs();

  pthread_exit(NULL);

}

#ifdef USE_GUI
void *startGUI(void *threadid) {
    printf("GUI is running!\n\n");

    server_gui();
  
    pthread_exit(NULL);
}
#endif


void changeDestination(char* ip){
  strcpy(destip,ip);

  if(p_comm != NULL){
    p_comm->changeDestination(ip);
  }
}





int main (int argc, char *argv[]) {
  strcpy(destip,"127.0.0.1");

  argcDreck = argc;
  argDreck = argv;

  /* by uska */
  mBuffy = bufferClass::init();
  //vBuffy = bufferClass::init();
  //aBuffy = bufferClass::init();
	
  pthread_t thread_streamer;
  pthread_t thread_parser;
  pthread_t thread_gui;
  pthread_t thread_smremote;
  
  int err,t=0;
  
  printf("\nThreading:\n\n");
  
  #ifdef USE_PARSER 
  printf("[**] starting parser...\n");
  err = pthread_create(&thread_parser,NULL,startParser,(void *)t);
  sleep(5);
  #else
  printf("[-] parser disabled\n\n");
  #endif

  #ifdef USE_GUI
  printf("[**] starting GUI...\n");
  err = pthread_create(&thread_gui,NULL,startGUI,(void *)t);
  #else
  printf("[-] gui disabled\n\n");
  #endif
  
  #ifdef USE_COMMUNICATOR
  printf("[**] starting Communicator...\n");
  //this delay should be set to prevent an empty buffer
  sleep(1);
  err = pthread_create(&thread_streamer,NULL,startCommunicator,(void *)t);
  #else
  printf("[-] Communicator disabled...\n\n");
  #endif
  

  #ifdef USE_REMOTESM
  printf("[**] Servicemanager Remote starting up...\n");
  err = pthread_create(&thread_smremote,NULL,startSMRemote,(void *)t);
  #else
  printf("[-] Remote Servicemanager disabled...\n\n");
  #endif
      
  
  if(err) {
    printf("ERROR;return code from pthread_create() is %d\n",err);
    exit(-1);
  }

  while (true) {
 	sleep(5);

  }

  pthread_exit(NULL);
  return 0;  
}

