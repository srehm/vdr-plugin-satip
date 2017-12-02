/*
 * rtp.c: SAT>IP plugin for the Video Disk Recorder
 *
 * See the README file for copyright information and how to reach the author.
 *
 */

#define __STDC_FORMAT_MACROS // Required for format specifiers
#include <inttypes.h>
#include <cstring>

#include "config.h"
#include "common.h"
#include "log.h"
#include "rtp.h"

cSatipRtpPacket::cSatipRtpPacket()
{
  headerM = dataM = 0;
  hlenM = dlenM = 0;
  seqNumM = 0;
  versionM = 2;
  ptM = 0;
}

cSatipRtpPacket::cSatipRtpPacket(u_char* d, size_t l)
{
  headerM = dataM = 0;
  hlenM = dlenM = 0;
  seqNumM = 0;
  versionM = 2;
  ptM = 0;

  if (l > 0 && (d[0] == TS_SYNC_BYTE)) {
     dlenM = 1;
     dataM = new u_char[1];
     dataM[0] = TS_SYNC_BYTE;
  }
  else if (l > 3) {
     // http://tools.ietf.org/html/rfc3550
     // http://tools.ietf.org/html/rfc2250
     // Version
     versionM = (d[0] >> 6) & 0x03;
     // Extension bit
     unsigned int x = (d[0] >> 4) & 0x01;
     // CSCR count
     unsigned int cc = d[0] & 0x0F;
     // Payload type: MPEG2 TS = 33
     ptM = d[1] & 0x7F;
     if (ptM == 33) {
        // Sequence number
        seqNumM = ((d[2] & 0xFF) << 8) | (d[3] & 0xFF);

        // Header length
        hlenM = (3 + cc) * (unsigned int)sizeof(uint32_t);
        // Check if extension
        if (x) {
           // Extension header length
          unsigned int ehl = (((d[hlenM + 2] & 0xFF) << 8) | (d[hlenM + 3] & 0xFF));
          // Update header length
          hlenM += (ehl + 1) * (unsigned int)sizeof(uint32_t);
        }

        headerM = new u_char[hlenM];
        std::memcpy(headerM, d, hlenM);

        dlenM = l - hlenM;

        if (dlenM > 0) {
           dataM = new u_char[dlenM];
           std::memcpy(dataM, d + hlenM, dlenM);
        }
     }
  }
}

cSatipRtpPacket::~cSatipRtpPacket()
{
  if (headerM) {
     delete headerM;
     headerM = 0;
  }
  if (dataM) {
     delete dataM;
     dataM = 0;
  }
}

bool cSatipRtpPacket::Valid(void) const
{
  if (dlenM == 0 || versionM != 2)
     return false;

  if (dataM[0] == TS_SYNC_BYTE)
     return true;

  if ((ptM != 33) || ((dlenM % TS_SIZE) != 0))
     return false;

  return true;
}

cSatipRtp::cSatipRtp(cSatipTunerIf &tunerP)
: cSatipSocket(SatipConfig.GetRtpRcvBufSize()),
  tunerM(tunerP),
  bufferLenM(eRtpPacketReadCount * eMaxUdpPacketSizeB),
  bufferM(MALLOC(unsigned char, bufferLenM)),
  lastErrorReportM(0),
  packetErrorsM(0),
  sequenceNumberM(-1)
{
  debug1("%s () [device %d]", __PRETTY_FUNCTION__, tunerM.GetId());
  if (!bufferM)
     error("Cannot create RTP buffer! [device %d]", tunerM.GetId());
}

cSatipRtp::~cSatipRtp()
{
  debug1("%s [device %d]", __PRETTY_FUNCTION__, tunerM.GetId());
  FREE_POINTER(bufferM);
}

int cSatipRtp::GetFd(void)
{
  return Fd();
}

void cSatipRtp::Close(void)
{
  debug1("%s [device %d]", __PRETTY_FUNCTION__, tunerM.GetId());

  cSatipSocket::Close();

  sequenceNumberM = -1;
  if (packetErrorsM) {
     info("Detected %d RTP packet error%s [device %d]", packetErrorsM, packetErrorsM == 1 ? "": "s", tunerM.GetId());
     packetErrorsM = 0;
     lastErrorReportM = time(NULL);
     }
}

void cSatipRtp::Process(void)
{
  debug16("%s [device %d]", __PRETTY_FUNCTION__, tunerM.GetId());
  if (bufferM) {
     unsigned int lenMsg[eRtpPacketReadCount];
     int count = 0;

     do {
        count = ReadMulti(bufferM, lenMsg, eRtpPacketReadCount, eMaxUdpPacketSizeB);
        for (int i = 0; i < count; ++i) {
           unsigned char *p = &bufferM[i * eMaxUdpPacketSizeB];
           tunerM.EnqueueRtpPacket(p, lenMsg[i]);
        }
     } while (count >= eRtpPacketReadCount);
  }
}

void cSatipRtp::Process(unsigned char *dataP, int lengthP)
{
  debug16("%s [device %d]", __PRETTY_FUNCTION__, tunerM.GetId());
  if (dataP && lengthP > 0) {
     tunerM.EnqueueRtpPacket(dataP, lengthP);
  }
}

cString cSatipRtp::ToString(void) const
{
  return cString::sprintf("RTP [device %d]", tunerM.GetId());
}
