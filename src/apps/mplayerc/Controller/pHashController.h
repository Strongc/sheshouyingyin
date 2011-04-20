#ifndef PHASHCONTROLLER_H
#define PHASHCONTROLLER_H

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

#include <threadhelper.h>
#include <fstream>
#include <Windows.h>
#include "LazyInstance.h"
#include "phashapi.h"
#include "pHashController.h"
#include "NetworkControlerImpl.h" 
#include "../MainFrm.h"
#include "HashController.h"
#include "Strings.h"
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
  enum {
    ONLYPHASH      = 0x01 << 0,   // 0000 0001
    PHASHANDSPHASH = 0x01 << 1,   // 0000 0010
    LOOKUP         = 0x01 << 2,   // 0000 0100
    INSERT         = 0x01 << 3,   // 0000 1000
    NOCALCHASH     = 0x01 << 4    // 0001 0000
  };
  void _thread_GetAudiopHash();
  BOOL VerifyAudiopHash(uint32_t* pHash);
  HRESULT _thread_DigestpHashData();                        // down samplerate and mix
  HRESULT _thread_MonopHash();                              // test only. Get each channel data and calc phash
  
  int GetSwitchStatus();
  void SetSwitchStatus(int status);
  HRESULT SetpHashData(struct phashblock* pbPtr);
  BOOL IsSeek();
  void SetSeek(int seekflag);
  void _Thread();
  void SetpASF(CComQIPtr<IAudioSwitcherFilter> pASF);
  CComQIPtr<IAudioSwitcherFilter> GetpASF(); 
  void IspHashInNeed(const wchar_t* filepath, int& result);

private:
  
  std::wstring m_sphash;
  uint32_t** m_hashes;                                     // Storage pHashes
  int *m_lens;
  struct phashblock* m_pbPtr;
  int m_phashlen;
  float* m_buffer;
  int m_bufferlen;
  int m_sr;                                               // sample rate to convert the stream
  int m_phashswitcher;
  int m_hashcount;
  int m_seekflag;
  CComQIPtr<IAudioSwitcherFilter> m_pASF;
  // Sample to float and normalized
  BOOL SampleToFloat(const unsigned char* const indata, float* outdata, int samples, int type);
  
  // DownSample
  BOOL DownSample(float* inbuf, int nsample, int des_sr, int org_sr, float** outbuf, int& outlen);
  
  // Mix
  BOOL MixChannels(float* buf, int samples, int channels, int nsample, float* MonoChannelBuf);
  void SixchannelsToStereo(float *output, float *input, int n);

  // TODO: Upload phash 
  typedef struct phashbox_t{
    uint8_t cmd;           // 1 for query, 2 for submission
    uint8_t earlyendflag;  // if end earlier , set the flag to 1; 
    uint8_t amount;        // amount of phash times
    uint8_t id;            // order of phash
    uint32_t nbframes;     // length of phash 
    uint32_t* phash;
  } phashbox;
  phashbox m_phashframe;
  void _thread_UploadpHash();
  void _thread_GetpHashAndSend(int cmd);
  int SendOnepHashFrame(phashbox phashframe);
  void DisconnectUrl();
};


#endif //PHASHCONTROLLER_H