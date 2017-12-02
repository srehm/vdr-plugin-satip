/*
 * rtp.h: SAT>IP plugin for the Video Disk Recorder
 *
 * See the README file for copyright information and how to reach the author.
 *
 */

#ifndef __SATIP_RTP_H_
#define __SATIP_RTP_H_

#include "socket.h"
#include "tunerif.h"
#include "pollerif.h"

class cSatipRtpPacket
{
private:
  u_char* headerM;
  u_char* dataM;
  size_t hlenM;
  size_t dlenM;
  ushort seqNumM;
  uchar versionM;
  uchar ptM;
  
  cSatipRtpPacket();

public:
  cSatipRtpPacket(u_char* d, size_t l);
  ~cSatipRtpPacket();

  inline size_t HeaderLength(void) const { return hlenM; }
  inline size_t DataLength(void) const { return dlenM; }
  inline ushort SequenceNumber(void) const { return seqNumM; }
  inline uchar Version(void) const { return versionM; }
  inline uchar PayloadType(void) const { return ptM; }

  inline uchar* Header(void) { return headerM; }
  inline uchar* Data(void) { return dataM; }

  bool Valid(void) const;
};

class cSatipRtp : public cSatipSocket, public cSatipPollerIf {
private:
  enum {
    eRtpPacketReadCount = 50,
    eMaxUdpPacketSizeB  = TS_SIZE * 7 + 12,
    eReportIntervalS    = 300 // in seconds
  };
  cSatipTunerIf &tunerM;
  unsigned int bufferLenM;
  unsigned char *bufferM;
  time_t lastErrorReportM;
  int packetErrorsM;
  int sequenceNumberM;
  int GetHeaderLength(unsigned char *bufferP, unsigned int lengthP);

public:
  explicit cSatipRtp(cSatipTunerIf &tunerP);
  virtual ~cSatipRtp();
  virtual void Close(void);

  // for internal poller interface
public:
  virtual int GetFd(void);
  virtual void Process(void);
  virtual void Process(unsigned char *dataP, int lengthP);
  virtual cString ToString(void) const;
};

#endif /* __SATIP_RTP_H_ */
