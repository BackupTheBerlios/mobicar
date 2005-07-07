 /******************************************************
**
** Video at Home; SEP SS 2005
**
**
** Description:
** Own (very simple but efficient) implementation of the RTCP-Protocol ! 
** Copyright (C) 2005 by Michael Golebski
** 
**
** 
** Modifications:
** Date:           Name:      Changes:           
** 2005-07-06      Golebski   Initial Version (C)
**
**
**
******************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <sys/time.h>
#include <unistd.h>

#define RTCPDEBUG 0

typedef unsigned char uint8_t;
typedef unsigned int uint32_t;
typedef unsigned short uint16_t;


/* enqueueX usage:
 * - buffer is buffer where data should get filled ...
 * - data is data the buffer gets filled with ...
 * - offset is the position of the buffer where first byte should be written 
 */


void enqueue8(uint8_t *buffer, uint8_t data, uint32_t offset) {
  buffer[offset] = data;
}

void enqueue16(uint8_t *buffer, uint16_t data, uint32_t offset) {
  uint8_t firstByte  = (data>>8) & 0xFF;
  uint8_t secondByte = data & 0xFF;
  buffer[offset] = firstByte;
  buffer[offset+1] = secondByte; 
}

void enqueue32(uint8_t *buffer, uint32_t data, uint32_t offset) {
  uint8_t firstByte  = (data>>24) & 0xFF;
  uint8_t secondByte = (data>>16) & 0xFF;
  uint8_t thirdByte  = (data>>8) & 0xFF;
  uint8_t fourthByte = data & 0xFF;
  //printf("First %02X - Second %02X - Third %02X - Fourth %02X\n",firstByte, secondByte, thirdByte, fourthByte);
  buffer[offset] = firstByte;
  buffer[offset+1] = secondByte; 
  buffer[offset+2] = thirdByte;
  buffer[offset+3] = fourthByte;
}


void buildSRPacket(uint32_t SSRC, struct timeval lastRTPTime, uint32_t lastRTPStamp, struct timeval lasttimeRTPStamp, uint32_t packetCounter, uint32_t octetCounter, CommunicatorXP *test) {

  uint32_t i;
  uint8_t firstPart[28]; // the RTCP Sender Report consists of two parts .. this is the first .. always static length
  
  char hostName[255]; // maximum length that hostname can be ... so array has to be so large !
  uint32_t returnhostName; // just to check if it was successfull
  uint32_t hostNameLength;
  
  uint32_t bytesToStuff;
  uint32_t secondPartLength;
  uint8_t *secondPart;
  
  uint8_t *completeSRPacket;
  
  struct timeval timeNow;
  gettimeofday(&timeNow, NULL);


  enqueue8(firstPart, 0x80, 0); // Version, Padding, Reception report count
  enqueue8(firstPart, 0xC8, 1); // =>(200)DEC .. is Sender Report
  enqueue8(firstPart, 0x00, 2);
  enqueue8(firstPart, 0x06, 3); // octet length of this part ... its fixed so we can set it here ...
  
  enqueue32(firstPart, SSRC, 4); // copy the Sender Source Identifier into the packet
  
  /* lets compute the NTP ... */
  enqueue32(firstPart, timeNow.tv_sec + 0x83AA7E80, 8);
  // NTP timestamp most-significant word (1970 epoch -> 1900 epoch)
  double fractionalPart = (timeNow.tv_usec/15625.0)*0x04000000; // 2^32/10^6
  enqueue32(firstPart, ((unsigned)(fractionalPart+0.5)), 12);
  
  /* TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO */
  printf("LAST TIME WAS: %u,%u\n",lasttimeRTPStamp.tv_sec, lasttimeRTPStamp.tv_usec);
  printf("NOW TIME IS  : %u,%u\n",timeNow.tv_sec, timeNow.tv_usec);
  // have to compute the real timestamp !!!
  unsigned fRTPTimestamp = lastRTPStamp;
  
  enqueue32(firstPart, fRTPTimestamp, 16);
  
  enqueue32(firstPart, packetCounter, 20);
  enqueue32(firstPart, octetCounter, 24);


  /* well, we've computed the first part of the RTCP-Sender-Packet, so let's go to the next ... */
  
  /* first we have to get the hostname (CNAME) of this computer */
  
  returnhostName = gethostname(hostName, sizeof(hostName));
  if(returnhostName==-1)
    printf("FAILURE: COULD NOT GET HOSTNAME OF THIS COMPUTER !!! UNHANDLED EXCEPTION ! CONTACT foo@lish.com \n");
  hostName[sizeof hostName-1] = '\0'; // just in case
  hostNameLength = strlen(hostName);
  
  /* fixed header size is 8 byte .. */
  
  bytesToStuff = 8 - ((hostNameLength +1) % 8); // hostnamelength plus 1 (END OF LINE 0x00)
  //printf("BYTES TO STUFF: %u",bytesToStuff);
  
  secondPartLength = 8 + hostNameLength + 1 + bytesToStuff;
    
  secondPart = (uint8_t *)malloc(secondPartLength); // dynamically allocate the secondPacket length !
  
  enqueue8(secondPart, 0x81, 0);
  enqueue8(secondPart, 0xCA, 1);
  enqueue16(secondPart, (secondPartLength / 4)-1, 2); // crazy RFC *g*
  enqueue32(secondPart, SSRC, 4);
  enqueue8(secondPart, 0x01, 8);
  enqueue8(secondPart, (uint8_t)hostNameLength, 9);
  
  memcpy(secondPart+10,hostName, hostNameLength); // copy the hostname into secondPart ...
  
  completeSRPacket = (uint8_t *)malloc(28+secondPartLength);
  
  memcpy(completeSRPacket,firstPart,28);
  memcpy(completeSRPacket+28,secondPart,secondPartLength);
  
  if(RTCPDEBUG) {
    for(i=0;i<28+secondPartLength;i++) {
      printf("%02X ", completeSRPacket[i]);
    }
  }
  test->senddata(completeSRPacket,secondPartLength+28);

}
