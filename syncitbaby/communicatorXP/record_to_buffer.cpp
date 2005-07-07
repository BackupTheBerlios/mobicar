#include "record_to_buffer.h"
#include "../server/PESParser.cpp"
//#include "../server/CommunicatorXP.h"

  uint32_t    rtpBaseTime = 0;
  struct timeval rtpBaseTimeRealTime;	      
	      
dvb::dvb() {
  lastAudioPCR = 0;
  lastVideoPCR = 0;
}

uint64_t dvb::getLastVideoPCR() {
  return lastVideoPCR;
}

uint64_t dvb::getLastAudioPCR() {
  return lastAudioPCR;
}

void dvb::startParser(struct circleBuffer* mBuffer) {

  int         video_started=  0, 
              audio_started=  0, 
              inlength=       0, 
              length=         0, 
              video_pos=      0, 
              audio_pos=      0,  
              error =         0,
	      counter =       0;
	      
      
 
  uint8_t     buf[TS_PACKET_LENGTH], 
              video[PES_PACKET_LENGTH], 
              audio[PES_PACKET_LENGTH];

  bool        havePacket = false;
  bool        wassendbefore = false;
  struct timeval timeOfRTPTimestamp;
  
  PESPacket   *PESPacketParser = new PESPacket();
  DvbStation  *station;
  
   
	CommunicatorXP* test = new CommunicatorXP();
	test->setDestination("127.0.0.1");
	test->setPort(5020);
	
	CommunicatorXP* test3 = new CommunicatorXP();
	test3->setDestination("127.0.0.1");
	test3->setPort(5021);
	
	CommunicatorXP* test2 = new CommunicatorXP();
	test2->setDestination("127.0.0.1");
	test2->setPort(5010);
	
	CommunicatorXP* test4 = new CommunicatorXP();
	test4->setDestination("127.0.0.1");
	test4->setPort(5011);
  

  try {
    //station = new DvbStation("Das Erste:198500:I0B7C34D12M16T8G4Y0:T:27500:4369:4370:4372:0:128:0:0:0");
    station = new DvbStation("SAT.1:658000:I0B8C23D23M16T8G4Y0:T:27500:385:386;392:391:0:16408:0:0:0");
  } catch(DvbStationException e) {
    e.Report();
    exit(1337);
  }

  cout << "[dvb] opening dvb device..." << endl;
#ifdef __MACOSX__
  MacDvbDevice::scanDevices();
  MacDvbDevice *device = new MacDvbDevice(0);
#else
  LinuxDvbDevice::scanDevices();
  LinuxDvbDevice *device = new LinuxDvbDevice(0);
#endif

  cout << "[dvb] tuning default station..." << endl;
  device->tune(station);
 
  cout << "[dvb] waiting for lock..." << endl;
  while(!device->hasLock()) {
    sleep(1);
  }

  cout << "[dvb] beginning to buffer data..." << endl;

  TSPacket * TSPacketParser;
  
  while( !this->m_die ) {
    counter++;
    //if(counter<100)
    //  printf("counting ... %u\n",counter);
    #ifdef DEBUG
      cout << "Get data..." << endl;
    #endif
     
    while( !havePacket ) {
      try {
        TSPacketParser = device->readTSPacket();
        havePacket = true;
      } catch(DvbDeviceException e) {
        e.Report();
      }
    }

    havePacket = false;

    #ifdef DEBUG
      cout << "[dvb] Got Data!" << endl;
      cout << "[dvb] TS Parser..." << endl;
    #endif

    if( TSPacketParser->get_pid() == station->getVideoPid()) {

      if( TSPacketParser->has_transport_error_indicator() )
        cout << "[dvb] broken TS video packet" << endl;
    
      if( !TSPacketParser->has_sync_byte() )
        cout << "[dvb] TS video packet without sync_byte" << endl;
      
      lastVideoPCR = TSPacketParser->get_pcr();
      
      #ifdef DEBUG  
        cout << "TS Parser -> getData..." << endl;
      #endif

      inlength = TSPacketParser->getData(&buf[0]);
      
      #ifdef DEBUG  
        cout << "[dvb] TS Parser -> getData: " << inlength << endl;
      #endif
        
      if( TSPacketParser->has_payload_unit_start_indicator() || TSPacketParser->has_pusi() ) {
       
        #ifdef DEBUG
          cout << "[dvb] Start of new PES packet" << endl;
        #endif

        if( video_pos > 0 ) {
          #ifdef DEBUG
            cout << "\t[dvb] inserting " << video_pos 
              << " Bytes into the Parser" << endl;
          #endif
     
          PESPacketParser->setPacket(video, video_pos);

          video_pos = PESPacketParser->getPacket(video);
          
          if( !PESPacketParser->has_packet_start_code_prefix() )
            ;//cout << "[dvb] PES packet without packet_start_code_prefix" << endl;
         
          if( PESPacketParser->get_pes_packet_length() < 1 ) {
            PESPacketParser->set_pes_packet_length();
          }

          #ifdef DEBUG
            cout << "\t[dvb] Bytes inserted and Length set" << endl;
            cout << "\t[dvb] has_packet_start_code_prefix? " 
              << PESPacketParser->has_packet_start_code_prefix() << endl;
            cout << "\t[dvb] pes_packet_length? " 
              << PESPacketParser->get_pes_packet_length() << endl;
            cout << "\t[dvb] stream_id? " 
              << PESPacketParser->get_stream_id() << endl;
            cout << "\t[dvb] audio_stream_number? " 
              << PESPacketParser->get_audio_stream_number() << endl;
            cout << "\t[dvb] video_stream_number? " 
              << PESPacketParser->get_video_stream_number() << endl;
            cout << "\t[dvb] additional_header? " 
              << PESPacketParser->has_additional_header() << endl;
            cout << "\t[dvb] pes_header_data_length " 
              << PESPacketParser->get_pes_header_data_length() << endl;
          #endif
	        gettimeofday(&timeOfRTPTimestamp,NULL);
                unsigned actualPTS = PESPacketParser->getPTS();
		
		if (!wassendbefore) {
		  rtpBaseTime = actualPTS;
		  gettimeofday(&rtpBaseTimeRealTime, NULL);
		  wassendbefore = 1;
		}
		
	        /* hack by uska */
	        length = PESPacketParser->getData(&video[0]); // Gets the PES data
		parseVideoPacket(video, length, actualPTS, timeOfRTPTimestamp, test, test3);
		usleep(1);
		//printf("PES HEADER LENGTH: %u\t",PESPacketParser->get_pes_header_data_length());
		//for(int i=0;i<length;i++)
		//  printf("%02X ",video[i]);
		//printf("\n\n");
          //length = PESPacketParser->getData(&video[0]); // Gets the ES data
          
          #ifdef DEBUG
            cout << "\t[dvb] PES Packet has " << length 
              << " Bytes of data" << endl;
            cout << "\t[dvb] writing the data to the output buffer..." << endl;
          #endif
         
          if( !this->m_die ) {
	  
	                      //while(bufferhasLock) {
			        //printf("Buffer has lock in RECORD_TO_BUFFER in video\n");
			     //   usleep(1);
		              //}
			     //if(counter<1000)
			     //bufferClass::write(mBuffer, video, length, 0);
            //if( error != -1 ) {
            //  cout << "[dvb] output buffer is full, discarding packets" << endl;
            //}
          }

          #ifdef DEBUG  
            cout << "\t[dvb] " << length << " Bytes written to buffer" << endl;
          #endif

          video_pos = 0;
        } else {
          video_started = 1;
        }
      } else if ( buf[0] == 0x00 &&
          buf[1] == 0x00 &&
          buf[2] == 0x01 &&
          buf[3] >= 188) {
        cout << "[dvb] new PES packet without PUSI" << endl;
      }

      if( inlength > 0 && video_started ) {
        if( (video_pos + inlength) >= PES_PACKET_LENGTH ) {
          cout << "[dvb] PES video packet buffer overflow" << endl;
        } else {
          memcpy(&video[video_pos], &buf[0], inlength);
          video_pos = video_pos + inlength;
        }
      }
    }

    vector<unsigned short> apids = station->getAudioPids();
    if( TSPacketParser->get_pid() == apids.at(0)) {

      if( TSPacketParser->has_transport_error_indicator() )
        cout << "[dvb] broken TS audio packet" << endl;

      if( !TSPacketParser->has_sync_byte() )
        cout << "[dvb] audio TS packet without sync_byte" << endl;
      
      // store the last PCR for the sync problem
      lastAudioPCR = TSPacketParser->get_pcr();
      
      inlength = TSPacketParser->getData(&buf[0]);

      if( TSPacketParser->has_payload_unit_start_indicator() || TSPacketParser->has_pusi() ) {
        
        if( audio_pos > 0 ) {
          PESPacketParser->setPacket(audio, audio_pos);

          if( PESPacketParser->get_pes_packet_length() < 1) {
            PESPacketParser->set_pes_packet_length();
          }
          unsigned actualPTS2 = PESPacketParser->getPTS();
	        /* hack by uska */
          length = PESPacketParser->getData(&audio[0]); // Gets the PES data
	  parseAudioPacket(audio, length, actualPTS2, test2, test4);
	        //length = PESPacketParser->getData(&audio[0]); // Gets the ES data
          if( !this->m_die ) {
	  
	                     //while(bufferhasLock) {
			      //printf("Buffer has lock in RECORD_TO_BUFFER in audio!\n");
			     // usleep(1);
			     //}
			     //if(counter<1000)
			     //bufferClass::write(mBuffer, audio, length, 0);
            //if( error != -1 ) {
            //  cout << "[dvb] output buffer is full, discarding packets..." << endl;
            //}
          }

          audio_pos = 0;
        } else {
          audio_started = 1;
        }
      }

      if( inlength > 0 && audio_started) {
        if( (audio_pos + inlength) >= PES_PACKET_LENGTH ) {
          cout << "[dvb] PES audio packet buffer overflow" << endl;
        } else {
          memcpy(&audio[audio_pos], &buf[0], inlength);
          audio_pos = audio_pos + inlength;
        }
      }
    }
    delete(TSPacketParser);
  }

  cout << "Closing all filehandles..." << endl;
  delete(device);
}

// vim: expandtab:shiftwidth=2:tabstop=2:softtabstop=2:textwidth=80
