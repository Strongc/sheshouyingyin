#ifndef PHASHCONTROLLER_H
#define PHASHCONTROLLER_H

#include <threadhelper.h>
#include "NetworkControlerImpl.h"
#include "LazyInstance.h"
#include "..\..\..\..\Thirdparty\pHash\phashapi.h"
#include "pHashController.h"
#include <fstream>
//////////////////////////////////////////////////////////////////////////
//
//  pHashController is a global instance controller that calculates
//  phash for the current playback video file.
//
//  Soleo
//  
//  Status: This is a demo with phash support. In this demo, we calculated 2 file's phash, and compared with each other. 
//          Each file deliverd four times decoded data which begin at 10 secs, 30 secs, 50secs, 110secs. The duration is 10secs.
//          We use audio phash following to the steps:  
//
//   FILE 1: Samples to float --> Normalization and channels merged --> Reduce samplerate to 8kHz --> Calc pHash --
//                                                                                                                 |---> Comparing two phashes
//   FILE 2: Samples to float --> Normalization and channels merged --> Reduce samplerate to 8kHz --> Calc pHash --
//  
//          Now we are able to get 2 channels vs 2 channels(same channel amount) video and audio phash, the results are not bad.
//          But 6 channels vs 2 channels (different channel amount) situation is still a problem.
//          

#define NORMALIZE_DB_MIN -145
#define NORMALIZE_DB_MAX 60


class pHashController:
  public LazyInstanceImpl<pHashController>,
  public ThreadHelperImpl<pHashController>,
  public NetworkControlerImpl
{
public:
  pHashController(void);
  ~pHashController(void);
  enum {CALCHASH = 1, NOCALCHASH};
  void _thread_GetAudiopHash();
  BOOL VerifyAudiopHash(uint32_t* pHash);
  HRESULT _thread_DigestpHashData();                        // down samplerate and mix
  
  HRESULT _thread_MonopHash();                              // test only. Get each channel data and calc phash
  
  int GetSwitchStatus();
  void SetSwitchStatus(int status);
  HRESULT SetpHashData(struct phashblock* pbPtr);
 
  void _Thread(); 
  
private:
  
  uint32_t** m_hashes;                                     // Storage pHashes
  int *m_lens;
  struct phashblock* m_pbPtr;
  int m_phashlen;
  float* m_buffer;
  int m_bufferlen;
  int m_sr;                                               // sample rate to convert the stream
  int m_phashswitcher;
  int hashcount;
  
  // Sample to float and normalized
  BOOL SampleToFloat(const unsigned char* const indata, float* outdata, int samples, int type);
  
  // DownSample
  BOOL DownSample(float* inbuf, int nsample, int des_sr, int org_sr, float** outbuf, int& outlen);
  
  // Mix
  BOOL MixChannels(float* buf, int samples, int channels, int nsample, float* MonoChannelBuf);
  void SixchannelsToStereo(float *output, float *input, int n);

  // TODO: Upload phash 
  void _thread_UppHashDownSub();

};


#endif //PHASHCONTROLLER_H