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
** This class stores PES packets and extracts the payload.  
**
**
**
**
** Author: Wolf-Bastian Poettner
** Date:   Jun. 30, 2005
**
** 
** Modifications:
** Date:           Name:      Changes:           
** Jun. 30, 2005   Poettner   Initial Version
**
**
**
******************************************************/

#include "pes.h"

/**
 * default contructor
 * does nothing
 */
PESPacket::PESPacket() {
}

/**
 * use this contructor to set the payload right away
 */
PESPacket::PESPacket(unsigned char *pesPacket, int pesBytes) {
  this->setPacket(pesPacket, pesBytes);
}

/**
 * checks if there is a packet_start_code_prefix at the beginning of the current
 * packet
 * the packet_start_code_prefix is 0x000001
 */
bool PESPacket::has_packet_start_code_prefix () {
  if( this->m_currentPacket[0] == 0x00 
      && this->m_currentPacket[1] == 0x00
      && this->m_currentPacket[2] == 0x01 ) {
    return true;
  } else {
    return false;
  }
}

/**
 * this function returns the pes_stream_id
 */
short PESPacket::get_stream_id () {
  return (this->m_currentPacket[3]);
}

/**
 * returns the pes_packet_length header field
 * but please, don't trust this one
 */
int PESPacket::get_pes_packet_length () {
  return ((this->m_currentPacket[4] << 8) + this->m_currentPacket[5]);
}

/**
 * sets the pes_packet_length header to the current packet length
 * this is for compatibility reasons
 */
bool PESPacket::set_pes_packet_length() {
  int setLength = this->m_currentPacketLength - 6;
 
  if( setLength > 65535 ) {
    setLength = 65535;
    fprintf(stderr, 
        "[dvb] non-standard PES packet detected (%d > 65535, ~%d pkts)\n", 
        this->m_currentPacketLength, (this->m_currentPacketLength - 65535)/184);
  }
  
  this->m_currentPacket[4] = setLength >> 8;
  this->m_currentPacket[5] = setLength;
  
  if( this->get_pes_packet_length() == this->m_currentPacketLength )
    return true;
  else
    return false;
}

/**
 * returns the audio_stream_number if we do have an audio stream
 * -1 otherwise!
 */
short PESPacket::get_audio_stream_number () {
  if( this->get_stream_id() >= 0xC0 && this->get_stream_id() <= 0xDF ) {
    return (this->m_currentPacket[3] & 0x3F);
  } else {
    return (-1);
  }
  
}

/**
 * returns the video_stream_number if we do have a video stream
 * -1 otherwise
 */
short PESPacket::get_video_stream_number () {
  if( this->get_stream_id() >= 0xE0 && this->get_stream_id() <= 0xEF ) {
    return (this->m_currentPacket[3] & 0xD);
  } else {
    return (-1);
  }
}

/**
 * returns if the current should have additional headers (according to the
 * stream_id)
 */
bool PESPacket::has_additional_header () {
  if( this->get_stream_id() != 0xBC
      && this->get_stream_id() != 0xBE
      && this->get_stream_id() != 0xBF
      && this->get_stream_id() != 0xF0
      && this->get_stream_id() != 0xF1
      && this->get_stream_id() != 0xFF
      && this->get_stream_id() != 0xF2
      && this->get_stream_id() != 0xF8) {
    return true;
  } else {
    return false;
  }
}

/**
 * returns the pes_crc_flag
 */
bool PESPacket::has_pes_crc_flag () {
  if( (this->m_currentPacket[7] & 0x2) >> 1 ) {
    return true;
  } else {
    return false;
  }
}

/**
 * returns the pes_header_data_length which is part of the additional header
 * this one seems to be trustworthy
 */
short PESPacket::get_pes_header_data_length () {
  if( this->has_additional_header() ) {
    return (this->m_currentPacket[8]);
  } else {
    return 0;
  }
}

/**
 * set the current packet
 *
 * @param   pesPacket  the Packet
 * @param   pesBytes   the length of the Packet
 *
 * @return             the length of the Packet
 */
int PESPacket::setPacket (unsigned char *pesPacket, int pesBytes) {
  memcpy(this->m_currentPacket, pesPacket, pesBytes);
  m_currentPacketLength = pesBytes;
  return pesBytes;
}

/**
 * returns the PTS if there are additional headers
 *
 * @return: -1        no additional header fields or no PTS field
 *          0         broken PTS field
 *          >0        PTS value
 */
int PESPacket::getPTS () {

    typedef signed long long mtime_t;
    mtime_t i_pts;

	     
  if( this->has_additional_header() && 
      (this->get_pts_dts_flags() == 2 ||
       this->get_pts_dts_flags() == 3)) {
   
     /* normally you do not have to check this ... cause it's not MPEG 1 !!! */
    if( ((m_currentPacket[9] & 0xF0) >> 4) != 2 && ((m_currentPacket[9] & 0xF0) >> 4) != 3) {
      return 0;
    }
      
          (int)i_pts = ((mtime_t)(m_currentPacket[9]&0x06 ) << 29)|
             (mtime_t)(m_currentPacket[10] << 22)|
            ((mtime_t)(m_currentPacket[11]&0xfe) << 14)|
             (mtime_t)(m_currentPacket[12] << 7)|
             (mtime_t)(m_currentPacket[13] >> 1);
	     
    //printf("PES: %u\n",i_pts);
   
    // Get the First 3 Bits of the PTS Field
    //int first = (m_currentPacket[9] & 0xE) >> 1;

    // Well, the comm guys only want 0x3
    int first = (m_currentPacket[9] & 0x3) >> 1;
    
    // Check for the marker bit
    if( (m_currentPacket[9] & 0x1) != 1 )
      return 0;

    // Get the next 15 Bits of the PTS Field
    int second = (m_currentPacket[10] << 7) + ((m_currentPacket[11] & 0xFE) >> 1);

    // Check for the marker bit
    if( (m_currentPacket[11] & 0x1) != 1 )
      return 0;

    // Get the next 15 Bits of the PTS Field
    int third = (m_currentPacket[12] << 7) + ((m_currentPacket[13] & 0xFE) >> 1);

    // Check for the last marker bit
    if( (m_currentPacket[13] & 0x1) != 1 )
      return 0;
    
    // Now combine the three field to one int
    int pts = (first << 30) + (second << 15) + third;
   
    return i_pts;
  } else 
    return -1;
}


short PESPacket::get_pts_dts_flags() {
  if( this->has_additional_header() ) 
    return ((m_currentPacket[7] & 0xC0) >> 6);
  else
    return 0;
}

/**
 * returns the entire current packet
 *
 * @param   data  pointer to the returned data
 */
int PESPacket::getPacket (unsigned char *data) {
  memcpy(data, &this->m_currentPacket, m_currentPacketLength);
  return m_currentPacketLength;
}

/**
 * returns the content of the current packet, if there is some
 *
 * @param   data  pointer to the returned data
 *
 * @return        Length of the data
 */
int PESPacket::getData (unsigned char *data) {
  int cutLength = 6;

  // If there is an additional header, then figure out the length and crop it
  if( this->has_additional_header () ) {
    cutLength = cutLength + 3 + this->get_pes_header_data_length();
  }

  if( (m_currentPacketLength - cutLength) < 1 )
    return 0;
  if( cutLength < 6 )
    return 0;

  memcpy(data, &this->m_currentPacket[cutLength], (m_currentPacketLength - cutLength));
  return (m_currentPacketLength - cutLength);
}

// vim: expandtab:shiftwidth=2:tabstop=2:softtabstop=2:textwidth=80
