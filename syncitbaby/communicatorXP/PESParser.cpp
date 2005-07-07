 /******************************************************
**
** Video at Home; SEP SS 2005
**
**
** Description:
** Own implementation of a MPEGII-Parser ! 
** Copyright (C) 2005 by Michael Golebski
** 
**
** 
** Modifications:
** Date:           Name:      Changes:           
** 2005-07-04      Golebski   Initial Version (C)
**
**
**
******************************************************/

#include <../server/basics.c>
#include "../server/CommunicatorXP.cpp"
#include "../server/RTCP.cpp"

typedef unsigned char uint8_t;
typedef unsigned int uint32_t;
typedef unsigned short uint16_t;

/* the maximum rtp packet size (UDP_SIZE(1490) MINUS HEADERS(42)...*/
#define PACKET_MAX_SIZE 1448
#define myDEBUG 0
#define VIDEODEBUG 0
/* Attention: if RAW_UDP_SEND is set to 1, RTP_SEND cannot be set to 1 ! */
#define RAW_UDP_SEND 0
#define RTP_SEND 1
#define FRAMEDEBUG 0


/* some var definitions here */
bool haveComputedVideoSeqNumber = 0;
bool haveComputedAudioSeqNumber = 0;
uint16_t vidSeqNumber;
uint16_t audSeqNumber;
bool haveComputedSSID = 0;
uint32_t sourceIdentifier;


/* Audio buffer definitions here */
uint32_t lastOffset;
uint8_t audioOffsetBuffer[10000]; // 10kB
uint8_t audioSendBuffer[10000]; // 10kB


/* FUNCTIONS COME HERE */


uint16_t videoSequenceNumber() {
  /* our random function to calculate the sequence number (packet number) increments every packet ... */
  if(!haveComputedVideoSeqNumber)
    vidSeqNumber = (u_int16_t)our_random();
  
  vidSeqNumber++;
  haveComputedVideoSeqNumber = 1;
  return vidSeqNumber;
}

uint16_t audioSequenceNumber() {
  /* our random function to calculate the sequence number (packet number) increments every packet ... */
  if(!haveComputedAudioSeqNumber)
    audSeqNumber = (u_int16_t)our_random();
  
  audSeqNumber++;
  return audSeqNumber;
}


uint32_t SSID() {
  /* our random function to calculate the synchronisation source identifier (SSID) ... always the same for one medium (VIDEO) */
  if(!haveComputedSSID)
    sourceIdentifier = our_random32();

  haveComputedSSID = 1;
  return sourceIdentifier;
}



void doVideoRTPFrame(uint8_t *inSlice, uint32_t inSliceLength, bool isLastPicture, uint16_t temp_ref, uint8_t picture_type, uint32_t inRTPTimestamp, CommunicatorXP *test) {
 if(FRAMEDEBUG)
  printf("Length: %u - isLastPicture: %u - temp_ref: %u - picture-type: %u - timestamp: %u\n",inSliceLength, isLastPicture, temp_ref, picture_type, inRTPTimestamp);

  bool fitsInOnePacket = 0;
  uint32_t numOfPackets; // when frames has to be fragmentated into X rtppackets we have to know how many ...
  uint32_t sameFramePacketCounter = 1; // we do not begin with ZERO ... THINK OF IT !!!
  uint8_t *startCode;

  if(inSliceLength <= PACKET_MAX_SIZE) {
    fitsInOnePacket = 1; // we know it fits into one rtp packet ...
    numOfPackets = 1;
  }
  else {
    /* we have to compute the amout of needed rtp-frames */
    uint32_t roundedAmount = inSliceLength / PACKET_MAX_SIZE;
    double   exactAmount   = (double)inSliceLength / (double)PACKET_MAX_SIZE;
    
    if(exactAmount>(double)roundedAmount)
      numOfPackets = roundedAmount + 1; // we know there have to be roundedAmount+1 Packets ...
    else if(exactAmount==(double)roundedAmount)
      numOfPackets = roundedAmount; // crazy, it exactly fits into X Frames 
  }
  
  printf("SIZE IS: %u - Number of Packets is: %u\n",inSliceLength, numOfPackets);

   /* BEGIN TO STUFF THE RTPHEADER */
  uint8_t rtpHeader[16];
  rtpHeader[0] = 0x80; // RTP Version, Padding, Extension, Contrib. Count
  
  if(isLastPicture)
    rtpHeader[1] = 0xA0; // Set the "last picture of I/B/P-Frame" marker to TRUE && set Payload Type MPEG2 (32)
  else 
    rtpHeader[1] = 0x20; // Set it FALSE && set Payload Type MPEG2 (32)
    
  // the sequence numbers will be computed and set later on ...
  
  enqueue32(rtpHeader,inRTPTimestamp,4);
  //memcpy(rtpHeader+4,&inRTPTimestamp,4); // Let's copy the RTPTimestamp into the RTP-Packet
  
  uint32_t fsourceIdentifier = SSID();
  enqueue32(rtpHeader,fsourceIdentifier,8);
  //memcpy(rtpHeader+8,&fsourceIdentifier,4);
  /* END STUFFING RTPHEADER */
  
  
  /* BEGIN TO STUFF THE MPEGHEADER */
  uint8_t mpegHeader[4];
  enqueue16(mpegHeader,temp_ref,0);
  //memcpy(mpegHeader,&temp_ref,2); // set MBZ and Temporal Picture Reference ...
  
 // we are continuing to calculate the infoByte later on ...
  
  uint8_t infoByte2;
  if(picture_type==0x02)
    infoByte2 = 0x07; // is P-Frame
  else if(picture_type==0x03)
    infoByte2 = 0x77; // is B-Frame
  else if(picture_type==0x01)
    infoByte2 = 0x00; // is I-Frame ... runtime optimized :D
    
  mpegHeader[3] = infoByte2;
  
  /* MPEG HEADER COMPUTED SUCCESSFULLY */
  
  
  
  //printf("HEADER COMPUTED SUCCESSFULLY \n");
    
  /* lets split the rtp-packet now ... */
    uint8_t *videoRTPFrame;
    uint32_t bufferSize;
    uint32_t offset;
    
    for(uint32_t i=0; i<numOfPackets; i++) {

    //printf("Bin in Schleife\n");
    
      if(fitsInOnePacket) {
        bufferSize = inSliceLength+16; 
	offset=0;
      }
      else if(sameFramePacketCounter<numOfPackets) { // not the last packet so we fill it to the maximum
        offset = (sameFramePacketCounter-1) * PACKET_MAX_SIZE;
	bufferSize = PACKET_MAX_SIZE;
      }
      else if((sameFramePacketCounter)==numOfPackets) { // last packet
        offset = (sameFramePacketCounter-1) * PACKET_MAX_SIZE;
        bufferSize = inSliceLength - ((sameFramePacketCounter-1) * PACKET_MAX_SIZE);
      }
      
      printf("Allocated Buffersize is: %u\n",bufferSize);
	
      videoRTPFrame = (uint8_t*)malloc(bufferSize);
      
      uint16_t fsequenceNumber = videoSequenceNumber(); // Let's get the Sequence Number
      enqueue16(rtpHeader,fsequenceNumber,2);
      //memcpy(rtpHeader+2,&fsequenceNumber,2);
      
      memcpy(videoRTPFrame,rtpHeader,12);
      
      
      /* infoByte Information:
       *      7        |        6           |           5             |     4        |     3      |    2 1 0
       * ALWAYS 0 (AN) | New Picture Header | Sequence-Header-Present | Slice begins | Slice ends | Picture-Type 
       */
    
      uint8_t infoByte = picture_type; // first we set infoByte to picture_type cause pic_type is least significant bit(s) ...
  
      if(picture_type==0x01 && sameFramePacketCounter==0)
        infoByte = infoByte + 0x20; // this is only true if frame is I-Frame and first Packet of I-Frame sent
    
      if(fitsInOnePacket)
        infoByte = infoByte + 0x18; // begin of slice and end of slice .. "else" we have to compute later ...
      else if(sameFramePacketCounter==1)
        infoByte = infoByte + 0x10; // its fragmentated, but its the first packet ...
      else if(sameFramePacketCounter==numOfPackets)
        infoByte = infoByte + 0x08; // its fragmentated, but its the last packet ...
	
      mpegHeader[2] = infoByte;
      
    if(FRAMEDEBUG) {
      for(int j=0;j<16;j++)
        printf("%02X", rtpHeader[j]);
      printf("\n\n");
      
      for(int j=0;j<4;j++)
        printf("%02X", mpegHeader[j]);
	      printf("\n\n");
    }
      
      memcpy(videoRTPFrame+12, mpegHeader, 4);
      
      memcpy(videoRTPFrame+16,inSlice+offset,bufferSize);
      
      test->senddata(videoRTPFrame,bufferSize);
      //videoUDPSend(videoRTPFrame,bufferSize);
      
      sameFramePacketCounter++;
      
  
  
  }
  
}


void parseVideoPacket(uint8_t *inBuffer, uint32_t length, uint32_t timeStamp, struct timeval timeOftimeStamp, CommunicatorXP *test) {

  /* First we have to define the vars for start_sequence_code checking routine */
  uint8_t firstByte;
  uint8_t secondByte;
  uint8_t thirdByte;
  uint8_t fourthByte;
  
  uint32_t next4Bytes;
  uint32_t lastSeen = 0;
  uint32_t i;
  uint32_t sliceLength;
  uint32_t sliceLengthSum = 0;
  
  uint16_t temporal_reference;
  uint8_t picture_coding_type;
  
  bool sawSomeStartCode = 0;
  
  /* the start codes are the following ... 
   * VIDEO_SEQUENCE_HEADER_START_CODE 0x000001B3
   * GROUP_START_CODE                 0x000001B8
   * PICTURE_START_CODE               0x00000100
   * SEQUENCE_END_CODE                0x000001B7
   * EXTENSION_START_CODE             0x000001B5
   */
   
   
  for(i=0;i<length;i++) {
  
    if(i>=3) {
      //read of course 4 bytes ...
      firstByte  = inBuffer[i-3];
      secondByte = inBuffer[i-2];
      thirdByte  = inBuffer[i-1];
      fourthByte = inBuffer[i];
    }
    
    if(firstByte==0x00 && secondByte==0x00 &&thirdByte==0x01 && fourthByte==0xB3) {
      sawSomeStartCode = 1;
      //struct timeval lala;
      //gettimeofday(&lala, NULL);
      //buildSRPacket(0x1234567, lala, timeStamp, timeOftimeStamp, 0x11998822, 0x55111188, test);
      if(VIDEODEBUG) 
        printf("Video Sequence Header Start Code\n");
      /* noting really important to get here ... */
    }

      
    else if(firstByte==0x00 && secondByte==0x00 &&thirdByte==0x01 && fourthByte==0xB8) {
      if(!sawSomeStartCode)
        sawSomeStartCode = 1;
      if(VIDEODEBUG) 
        printf("Group Start Code\n");
      /* noting really important to get here, too ... */
    }

      
    else if(firstByte==0x00 && secondByte==0x00 &&thirdByte==0x01 && fourthByte==0x00) {
      if(!sawSomeStartCode)
        sawSomeStartCode = 1;
      /* lets extract the temporal reference and the picture-coding type out of the picture header ... */
      if(VIDEODEBUG) 
        printf("Picture Start Code\n");
      
      next4Bytes = (inBuffer[i+1]<<24) + (inBuffer[i+2]<<16) + (inBuffer[i+3]<<8) + (inBuffer[i+4]);
      temporal_reference = (next4Bytes&0xFFC00000)>>(32-10);
      picture_coding_type = (next4Bytes&0x00380000)>>19;
      if(VIDEODEBUG) 
        printf("Temporal reference: %d, picture_coding_type: %d\n",temporal_reference, picture_coding_type);
    }
      
    else if(firstByte==0x00 && secondByte==0x00 &&thirdByte==0x01 && fourthByte==0xB7) {
      /* this never gonna happen .. dunno */
      if(VIDEODEBUG) 
        printf("Sequence End Code\n");
    }

          
    else if(firstByte==0x00 && secondByte==0x00 &&thirdByte==0x01 && fourthByte==0xB5) {
      if(!sawSomeStartCode)
        sawSomeStartCode = 1;
      if(VIDEODEBUG) 
        printf("Extension Start Code\n");
      /* we have to do it to correcly parse the code ... you can call it HACK as you want */
    }

      
    else if(firstByte==0x00 && secondByte==0x00 &&thirdByte==0x01) {
      /* found a slice ... we have to encapsulate it, so first we have to get its length */
      sliceLength = i - lastSeen;
      sliceLengthSum = sliceLengthSum + sliceLength;
      
      if(VIDEODEBUG) {
        printf("We think the slice length is: %u - lastseen is: %u\n", sliceLength, lastSeen);
	/* here we encapsulate the start code .. and nothing but ONLY the start code !!! */
        for(unsigned j=0;j<sliceLength;j++) 
          printf("%02X ", inBuffer[i+j-3-sliceLength]);
        printf("\n\n");
      }
      
      if(sawSomeStartCode) {
       /* dirty HACK ! We think the startcode begins always at "inBuffer[0]" but for sure it has to be checked ! */
       if(RAW_UDP_SEND)
         test->senddata(inBuffer,sliceLength-3);
       if(RTP_SEND)
         doVideoRTPFrame(inBuffer, sliceLength-3, 0, temporal_reference,  picture_coding_type, timeStamp, test);
	 printf("\n\n");
	 printf("sawsomecode and send the following: \n");
	 for(int u=0;u<(sliceLength-3);u++)
	   printf("%02X ",inBuffer[u]);
	   //startCode = (uint8_t *)malloc(sliceLength-3);
	   //memcpy(startCode,
	   	 
       sawSomeStartCode = 0;
      }
      else {
      printf("fourth byte is %02X\n",inBuffer[i]);
        if(RAW_UDP_SEND)
          test->senddata(inBuffer+i-3-sliceLength,sliceLength);
	  
	if(RTP_SEND)
	  doVideoRTPFrame(inBuffer+i-3-sliceLength, sliceLength, 0, temporal_reference,  picture_coding_type, timeStamp, test);
	
	printf("\n\n");
	printf("else send the following: \n");
	 for(int u=0;u<sliceLength;u++)
	   printf("%02X ",inBuffer[u+i-3-sliceLength]);
      }

        

      //doVideoRTPFrame(inBuffer[i-3-sliceLength], sliceLength, temporalreference, picture_coding_type, timestamp);
      lastSeen = i;
    }
      
  }

  /* now we get the last frame in this packet ;) */
  //for(unsigned j=0;j<(length - sliceLengthSum);j++)
  //  printf("%02X ",inBuffer[j+sliceLengthSum-3]);
  //printf("\n\n");
  if(RAW_UDP_SEND)
    test->senddata(inBuffer+sliceLengthSum-3,length - sliceLengthSum);
  
  if(RTP_SEND)
    doVideoRTPFrame(inBuffer+sliceLengthSum-3, length - sliceLengthSum, 1, temporal_reference,  picture_coding_type, timeStamp, test);
  /* isLastPicture = 1 ;) */
  
}


void parseAudioPacket(uint8_t *inBuffer, uint32_t length, uint32_t timeStamp, CommunicatorXP *test) {

  /* INFO AN MICH SELBST: Audio Frames haben immer die gleiche Größe (sollten sie haben).
   * Ein Paket beginnt nie mit einem Audio-Frame! Deshalb muss man das Offset des letzten Paketes
   * speichern und den RTP-Timestamp auch vom alten Paket nehmen ... 
   */

  /* First we have to define the vars for start_sequence_code checking routine */
  uint8_t firstByte;
  uint8_t secondByte;
  uint8_t thirdByte;
  uint8_t fourthByte;
  
  uint32_t i;
  uint32_t next4Bytes;
  
  /* Berechnung der Offsets */
  uint32_t currOffset = 0;
  
    
  /* PRINT THE WHOLE INCOMING PACKET .. DEBUG ONLY
  for(unsigned u=0;u<length;u++)
    printf("%02X ",inBuffer[u]);
  printf("\n\n");
  */
  
  for(i=0;i<length;i++) {
  
    if(i>=3) {
      //read of course 4 bytes ...
      firstByte  = inBuffer[i-3];
      secondByte = inBuffer[i-2];
      thirdByte  = inBuffer[i-1];
      fourthByte = inBuffer[i];
    }
    
    next4Bytes = (firstByte<<24) + (secondByte<<16) + (thirdByte<<8) + (fourthByte);
    //if((next4Bytes&0xFFE00000)==0xFFE00000) { // THIS SHIT AIN'T GONNA WORK .. DUNNO WHY .. MPEG BITLOOSE PROBLEMS ?!
    if(next4Bytes==0XFFFCA400) {
      
      currOffset = i-3;
      
      if(myDEBUG) {
        printf("Found mpeg audio header .. Length: %u\n",length);   
        printf("First %02X, Second %02X, Third %02X, Fourth %02X\n",firstByte, secondByte, thirdByte, fourthByte);    
        /* well we know the difference (=framesize) 'cause this doesn't vary .. if it does we have a corrupt packet (maybe ?!) */
        printf("last offset was %u - offset now is %u - difference is %u\n",lastOffset,currOffset, currOffset+lastOffset);
      }
      
      uint32_t audioLength = currOffset + lastOffset;
      
      memcpy(audioSendBuffer,audioOffsetBuffer, lastOffset);  // lets copy the last offset into the audioBuffer
      memcpy(audioSendBuffer+lastOffset,inBuffer,currOffset); // lets copy the offset from the actual frame into the audioBuffer 
      /* NOW WE SHOULD HAVE A COMPLETE AUDIO FRAME IN THE BUFFER */
      //printf("TRYING TO SEND ..\n");
      //test->senddata(audioSendBuffer,audioLength);
      //for(unsigned a=0;a<audioLength;a++) 
      //  printf("%02X ",audioSendBuffer[a]);
      //printf("\n\n");
      
      /* ATTENTION ... THIS HACK IS DIRTY 'CAUSE WE EXPECT ONLY ONE COMPLETE FRAME INTO THIS PACKET ... */
      memcpy(audioOffsetBuffer,inBuffer+currOffset,length-currOffset); // copy the actual "beginning" of frame into buffer
      
      
      lastOffset =  length-(i-3); // there is a new audio header at this position so the last offset was from zero up to here 

    }
    
    if(myDEBUG) {
      if((i+1)==length)
        printf("LENGTH WAS: %u \n\n", length);
    }
    
  }

}








  

  

