/*
 * tunerif.h: SAT>IP plugin for the Video Disk Recorder
 *
 * See the README file for copyright information and how to reach the author.
 *
 */

#ifndef __SATIP_TUNERIF_H
#define __SATIP_TUNERIF_H

#include <string>

class cSatipRtpPacket;

class cSatipTunerIf {
public:
  cSatipTunerIf() {}
  virtual ~cSatipTunerIf() {}
  virtual void ProcessVideoData(u_char *bufferP, int lengthP) = 0;
  virtual void ProcessApplicationData(u_char *bufferP, int lengthP) = 0;
  virtual void ProcessRtpData(u_char *bufferP, int lengthP) = 0;
  virtual void ProcessRtcpData(u_char *bufferP, int lengthP) = 0;
  virtual void SetStreamId(int streamIdP) = 0;
  virtual void SetSessionTimeout(const char *sessionP, int timeoutP) = 0;
  virtual void SetupTransport(int rtpPortP, int rtcpPortP, const char *streamAddrP, const char *sourceAddrP) = 0;
  virtual int GetId(void) = 0;
  virtual void EnqueueRtpPacket(u_char* d, size_t l) = 0;
  virtual cSatipRtpPacket* DequeueRtpPacket(ushort cseq) = 0;
  virtual cSatipRtpPacket* DequeueOldestRtpPacket() = 0;
  virtual void ClearQueue(void) = 0;
  virtual size_t QueueSize(void) = 0;

private:
  explicit cSatipTunerIf(const cSatipTunerIf&);
  cSatipTunerIf& operator=(const cSatipTunerIf&);
};

#endif // __SATIP_TUNERIF_H
