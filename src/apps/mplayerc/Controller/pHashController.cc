#include "stdafx.h"
#include <Strings.h>
#include <io.h> 
#include <fcntl.h>
#include <sys/stat.h>
#include <string>
#include <fstream>
#include "samplerate.h"
#include "sndfile.h"
#include "pHashController.h"
#include "zmqhelper.h"
#include "../apps/mplayerc/Model/pHashModel.h"
#include "../FGManager.h"

#define FREE_PHASHMEM() \
  m_pbPtr->phashdata.clear();\
  m_pbPtr->phashdata.resize(0);

pHashController::pHashController(void) :
  m_buffer(NULL),
  m_pbPtr(NULL),
  m_sr(8000),
  m_phashswitcher(0),
  m_bufferlen(0),
  m_phashlen(0),
  m_hashcount(0),
  m_seekflag(false)
{
  m_hashes = (uint32_t**)malloc(8*sizeof(uint32_t*));
  m_lens = (int*)malloc(8*sizeof(int));
}

pHashController::~pHashController(void)
{
  
}

int pHashController::GetSwitchStatus()
{
  return m_phashswitcher;
}

void pHashController::SetSwitchStatus(int status)
{
  m_phashswitcher = status;
}

HRESULT pHashController::SetpHashData(struct phashblock* pbPtr)
{
  m_pbPtr = pbPtr;

  if (!m_pbPtr)
  {
    Logging(L"m_pbPtr is invalid\n");
    return S_FALSE;
  }
  return S_OK;
}

BOOL pHashController::IsSeek()
{
  if(m_pASF)
    return  m_pASF->IsSeek(); 
}

void pHashController::SetSeek(int seekflag)
{
  if(m_pASF)
  {
    if (m_pASF->SetSeek(seekflag) == S_OK)
      Logging(L"pASF->SetSeek OK");
    else
      Logging(L"pASF->SetSeek Failed");
  }
}

void pHashController::_Thread()
{
  Logging( L"pHashController::_thread enter %x", m_phashswitcher);
  switch (m_phashswitcher)
  {
  case CALCHASH:
    //_thread_MonopHash();
    _thread_DigestpHashData();
//     _thread_GetAudiopHash();
//     _thread_UploadpHash();
    _thread_GetpHashAndSend();
    break;
  case NOCALCHASH:
  default:
    break;
  }
  Logging( L"pHashController::_thread exit %x", m_phashswitcher);
}
HRESULT pHashController::_thread_MonopHash()
{
  int buflen = m_pbPtr->phashdata.size();
  float *buf = new float[buflen];

  int samplebyte = (m_pbPtr->format.wBitsPerSample >> 3);                          // the size of one sample in Byte unit
  int nsample = m_pbPtr->phashdata.size()/m_pbPtr->format.nChannels/samplebyte;    // each channel sample amount
  int samples = nsample * m_pbPtr->format.nChannels;                               // all channels sample amount

  // input buffer for signal
  unsigned char* indata = &m_pbPtr->phashdata[0];

  // Making all data into float type 
  if (SampleToFloat(indata, buf, samples, m_pbPtr->type) == FALSE)
  {
    delete [] buf;
    FREE_PHASHMEM();
    return S_FALSE;
  }
  
  int chns = m_pbPtr->format.nChannels;
  float* tmpmem = new float[nsample];
  
  for (int n = 0; n < chns; ++n)
  {
    memset(tmpmem, 0, sizeof(tmpmem));
    for (int j = 0; j < nsample; j++)
      tmpmem[j] = buf[n+chns*j];
    wfstream fs;
    wchar_t filepath[80];
    wsprintf(filepath, L"C:\\Users\\Staff\\Desktop\\splayer_shao\\hash\\txt_chn_%d.txt", n);
    fs.open(filepath, ios_base::out | ios_base::app);
    for (int i=0; i < nsample; ++i)
     fs << tmpmem[i] << L"\r\n";
    fs.close();
    float* outmem = NULL;
    int outnums;
    DownSample(tmpmem, nsample, m_sr, m_pbPtr->format.nSamplesPerSec, &outmem, outnums);
    m_hashes[m_hashcount] = ph_audiohash(outmem, outnums, m_sr, m_phashlen);
    m_lens[m_hashcount] = m_phashlen;

    free(outmem);
    m_hashcount++;
  }
  
  delete [] tmpmem;
  FREE_PHASHMEM();

  if (m_hashcount < 7)
    return S_OK;


  for (int j = 0; j < 2; ++j)
  {
    for (int ii = 0; ii < 6; ++ii)
    {
      int nc;
      double* cs;
      cs = ph_audio_distance_ber(m_hashes[j], m_lens[j], m_hashes[ii+2], m_lens[ii+2],
        0.3, 256, nc);                                                             // threshold 0.3 , block_size: 256 
      if (!cs)
        ;
      else
      {
        double max_cs = 0.0;
        for (int i=0;i<nc;i++)
        {
          if (cs[i] > max_cs)
            max_cs = cs[i];
        }
        sphash_freemem2(cs);
      }   
    }
  }

  return S_OK;
}
void pHashController::_thread_GetAudiopHash()
{
  int count = g_phash_collectcfg[CFG_PHASHTIMES]*2-1;
  int step = g_phash_collectcfg[CFG_PHASHTIMES];
  if (m_pbPtr->phashcnt <= count)
  {
    int i = m_pbPtr->phashcnt;
    m_hashes[i] = ph_audiohash(m_buffer, m_bufferlen, m_sr, m_phashlen);
    m_lens[i] = m_phashlen;
    Logging(L"m_pbPtr->phashcnt:%d, length = %d, hashes[0]:%X, m_phashlen:%d",i, m_lens[i], m_hashes[i][0], m_phashlen);
    free(m_buffer);
    m_buffer = NULL;
    m_bufferlen = 0;
  }

  if (m_pbPtr->phashcnt == count)
  {
      int nc;
      double* cs;
      for (int index = 0; index < step; index++)
      { 

        Logging(L"m_phash1[%d]:%X,%X,%X,%X mphash1len[%d]:%d",index, m_hashes[index][0], m_hashes[index][1],
          m_hashes[index][2],m_hashes[index][3], index, m_lens[index] );
        Logging(L"m_phash2[%d]:%X,%X,%X,%X mphash2len[%d]:%d",index, m_hashes[index+step][0], m_hashes[index+step][1],
          m_hashes[index+step][2],m_hashes[index+step][3], index, m_lens[index+step] );

        cs = ph_audio_distance_ber(m_hashes[index], m_lens[index], m_hashes[index+step], m_lens[index+step],
          0.3, 256, nc);                                                         // threshold 0.3 , block_size: 256 
        if (!cs)
          return;
        else
        {
          double max_cs = 0.0;
          for (int i=0;i<nc;i++)
          {
            if (cs[i] > max_cs)
              max_cs = cs[i];
          }
          Logging(L"Confidence Score: %f",max_cs);
          sphash_freemem2(cs);
          cs = NULL;
        }
        // free space
        sphash_freemem(NULL, m_hashes[index]);
        sphash_freemem(NULL, m_hashes[index+step]);
        m_hashes[index] = NULL;
        m_hashes[index+step] = NULL;
        m_lens[index] = 0;
        m_lens[index+step] = 0;  
      }
      free(m_lens);
      free(m_hashes);
  }
  m_pbPtr->prevcnt = m_pbPtr->phashcnt;
  m_pbPtr->phashcnt++;
  if (m_pbPtr->phashcnt > count)
    m_pbPtr->phashcnt = 0;
}
void pHashController::SetpASF(CComQIPtr<IAudioSwitcherFilter> pASF)
{
  m_pASF = pASF;
}

CComQIPtr<IAudioSwitcherFilter> pHashController::GetpASF()
{
  return m_pASF;
}

void pHashController::_thread_GetpHashAndSend()
{
  int count = g_phash_collectcfg[CFG_PHASHTIMES] - 1;
  Logging("phashcnt:%d", m_pbPtr->phashcnt);
  if (m_pbPtr->phashcnt <= count)
  { 
    int i = m_pbPtr->phashcnt;
    m_phashframe.cmd = 1;
    m_phashframe.amount = g_phash_collectcfg[CFG_PHASHTIMES];
    m_phashframe.id = i;

    if (IsSeek() == true)
    {
      m_phashframe.earlyendflag = 1;
      m_phashframe.nbframes = 0;
      m_phashframe.phash = NULL;
      m_phashswitcher = NOCALCHASH;    // turn off phash
    }
    else
    {
      m_phashframe.earlyendflag = 0;
      m_hashes[i] = ph_audiohash(m_buffer, m_bufferlen, m_sr, m_phashlen);
      m_lens[i] = m_phashlen;
      Logging(L"m_pbPtr->phashcnt: %d, length: %d, hashes[0]: %X, m_phashlen: %d", i, m_lens[i], m_hashes[i][0], m_phashlen);
      m_phashframe.nbframes = m_lens[i];
      m_phashframe.phash = m_hashes[i];
    }
    

    free(m_buffer);
    m_buffer = NULL;
    m_bufferlen = 0;
    
    //sending to server
    SendOnepHashFrame(m_phashframe);
    sphash_freemem(NULL, m_hashes[i]);
    m_hashes[i] = NULL;
    m_lens[i] = 0;
  }

  m_pbPtr->prevcnt = m_pbPtr->phashcnt;
  m_pbPtr->phashcnt++;
  if (m_pbPtr->phashcnt > count)
  {
    m_pbPtr->phashcnt = 0;
    free(m_hashes);
    free(m_lens);
    m_hashes = NULL;
    m_lens = NULL;
  }
}

BOOL pHashController::SampleToFloat(const unsigned char* const indata, float* outdata, int samples, int type)
{
  int samplecnt = 0;
  int tmp;

  switch (type)
  {
   // PCM8
  case PCM8:
    for (samplecnt = 0; samplecnt < samples; samplecnt++)      
      outdata[samplecnt] = ((float)(((BYTE*)indata)[samplecnt]) -0x7f) / 0x80; //UCHAR_MAX
    break;
   // PCM16
  case PCM16:
    for (samplecnt = 0; samplecnt < samples; samplecnt++)
      outdata[samplecnt] = (float)(((short*)indata)[samplecnt])/(float)SHRT_MAX;
    break;  
   // PCM24
  case PCM24:
    for (samplecnt = 0; samplecnt < samples; samplecnt++)
    {
      memcpy(((BYTE*)&tmp)+1, &indata[3*samplecnt], 3);  
      outdata[samplecnt] += (float)(tmp >> 8) / ((1<<23)-1);
    }
    break;
   // PCM32
  case PCM32:
    for (samplecnt = 0; samplecnt < samples; samplecnt++)
      outdata[samplecnt] = (float)(((int*)indata)[samplecnt]);
    break;
    // FPCM32
  case FPCM32:
    for (samplecnt = 0; samplecnt < samples; samplecnt++)
      outdata[samplecnt] = (float)(((float*)indata)[samplecnt]);
    break;
    // FPCM64: did not have it tested
  case FPCM64: 
    for (samplecnt = 0; samplecnt < samples; samplecnt++)
      outdata[samplecnt] = (float)(((double*)indata)[samplecnt]);
    break;

  default:
    return FALSE;
  }

  return TRUE;
}

// mix into one channel and reduced samplerate to 8000HZ for further cal
HRESULT pHashController::_thread_DigestpHashData()
{
  int buflen = m_pbPtr->phashdata.size();
  float *buf = new float[buflen];

  int samplebyte = (m_pbPtr->format.wBitsPerSample >> 3);                             // the size of one sample in Byte unit
  int nsample = m_pbPtr->phashdata.size()/m_pbPtr->format.nChannels/samplebyte;       // each channel sample amount
  int samples = nsample * m_pbPtr->format.nChannels;                                  // all channels sample amount
  
  // alloc input buffer for signal
  unsigned char* indata = &m_pbPtr->phashdata[0];

  // Making all data into float type 
  if (SampleToFloat(indata, buf, samples, m_pbPtr->type) == FALSE)
  {
    delete [] buf;
    FREE_PHASHMEM();
    return S_FALSE;
  }
  else
  {
    FREE_PHASHMEM();
  }

  // Mix as Mono channel
  int MonoLen = buflen/m_pbPtr->format.nChannels;
  float* MonoChannelBuf = new float[MonoLen];
  if (MixChannels(buf, samples, m_pbPtr->format.nChannels, nsample, MonoChannelBuf) == TRUE)
  {
    delete[] buf;
    FREE_PHASHMEM();
  }
  else
  {
    delete[] buf;
    delete[] MonoChannelBuf;
    FREE_PHASHMEM();
    return S_FALSE;
  }

  // Samplerate to 8kHz 
  float* outmem = NULL;
  int outnums = 0;
  if (DownSample(MonoChannelBuf, nsample, m_sr, m_pbPtr->format.nSamplesPerSec, &outmem, outnums) == TRUE)
  {
    delete[] MonoChannelBuf;
    m_buffer = outmem;
    m_bufferlen = outnums;
  }
  else
  {
    free(outmem);
    FREE_PHASHMEM();
    delete[] MonoChannelBuf;
    return S_FALSE;
  }

  return S_OK;
}
BOOL pHashController::DownSample(float* inbuf, int nsample, int des_sr, int org_sr, float** outbuf, int& outlen)
{
  // resample float array , set desired samplerate ratio
  double sr_ratio = (double)(des_sr)/(double)org_sr;
  if (src_is_valid_ratio(sr_ratio) == 0)
  {
    Logging(L"sr ratio is invalid,sr_ratio:%f,sr:%d,samplerate:%d",sr_ratio, des_sr, org_sr);
    FREE_PHASHMEM();
    return FALSE;
  }

  // allocate output buffer for conversion
  outlen = sr_ratio * nsample;
  *outbuf = (float*)malloc(outlen * sizeof(float)); 
  if (!outbuf)
  {
    Logging(L"outbuffer create failed!");
    FREE_PHASHMEM();
    return FALSE;
  }
  int error;
  SRC_STATE *src_state = src_new(SRC_LINEAR, 1, &error);
  if (!src_state)
  {
    free(*outbuf);
    FREE_PHASHMEM();
    Logging(L"src_state failed");
    return FALSE;
  }

  SRC_DATA src_data;
  src_data.data_in = inbuf;
  src_data.data_out = *outbuf;
  src_data.input_frames = nsample;
  src_data.output_frames = outlen;
  src_data.end_of_input = SF_TRUE;
  src_data.src_ratio = sr_ratio;

  // Sample rate conversion
  if (src_process(src_state, &src_data))
  {
    free(*outbuf);
    free(inbuf);
    src_delete(src_state);
    FREE_PHASHMEM();
    Logging(L"src_process: failed");
    return FALSE;
  }
  src_delete(src_state);
  return TRUE;
}
BOOL pHashController::MixChannels(float* buf, int samples, int channels, int nsample, float* MonoChannelBuf)
{
  int bufindx = 0;
  if (channels == 2)
  {
    do 
    {
      for (int j = 0; j < samples; j += channels)
      {
        MonoChannelBuf[bufindx] = 0.0f;
        for (int i = 0; i < channels; ++i)
          MonoChannelBuf[bufindx] += buf[j+i];
        MonoChannelBuf[bufindx++] /= channels;
      }
    }while (bufindx < nsample);
  }
  else if (channels == 6)
  { // TODO: now i won't work actually
    float* temp = new float[nsample*2];
    memset(temp, 0, sizeof(temp));
    SixchannelsToStereo(temp, buf, nsample);
    for (int i = 1; i < nsample; i++)
    {
       MonoChannelBuf[i] = 0.0f;
       for (int j = 0; j < 2; j++)
      {
        MonoChannelBuf[i] = temp[i+2]; 
      }
      MonoChannelBuf[i] /= 2;
    }
    delete [] temp;
  }
  return TRUE;
}

void pHashController::SixchannelsToStereo(float* output, float* input, int n)
{
  float *p = input;

  while(n-- > 0)
  {
     *output++ = (p[0] + p[1] + p[3] + p[5]) / 3;
     *output++ = (p[1] + p[2] + p[4] + p[5]) / 3;
    p += 6;
  }
}

// Upload pHash
void pHashController::_thread_UploadpHash()
{
  // connection
  void* context = zmq_init(1);
  void* client = socket_connect(context, ZMQ_REQ, "tcp://192.168.10.46:5000");
  
  // sending all phashes
  m_phashframe.cmd = 1;
  m_phashframe.amount = g_phash_collectcfg[CFG_PHASHTIMES];
  for (int times = 0; times < g_phash_collectcfg[CFG_PHASHTIMES]; times++)
  {
    m_phashframe.id = times;
    m_phashframe.nbframes = m_lens[times];
    m_phashframe.phash = new uint32_t[m_lens[times]];
    memcpy_s(m_phashframe.phash, m_lens[times], m_hashes[times], m_lens[times]);
    sendmore_msg_vsm(client, &m_phashframe.cmd, sizeof(uint8_t));
    sendmore_msg_vsm(client, &m_phashframe.amount, sizeof(uint8_t));
    sendmore_msg_vsm(client, &m_phashframe.id, sizeof(uint8_t));
    sendmore_msg_vsm(client, &m_phashframe.nbframes, sizeof(uint32_t));
    sendmore_msg_data(client, m_phashframe.phash, m_phashframe.nbframes*sizeof(uint32_t), free_fn, NULL);
    Logging("send:cmd:%d, amount:%d, id:%d, nbframes:%d, phash[0]:%X", 
      m_phashframe.cmd, m_phashframe.amount, m_phashframe.id , m_phashframe.nbframes, m_phashframe.phash[0]);

  }
  send_empty_msg(client);
  
  Sleep(1);
  
  // get result
  uint8* data = NULL;
  uint8_t result;
  int64_t more;
  size_t msg_size, more_size = sizeof(int64_t);
  recieve_msg(client, &msg_size, &more, &more_size, (void**)&data);
  if (msg_size == sizeof(uint8_t))
  {
    memcpy(&result, data, sizeof(uint8_t));
    Logging("Get result:%d", result);
  }
  zmq_close(client);
  zmq_term(context);
}

int pHashController::SendOnepHashFrame(phashbox_t phashframe)
{
  // connection
  void* context = zmq_init(1);
  void* client = socket_connect(context, ZMQ_REQ, "tcp://192.168.10.46:5000");

  // sending all phashes
  sendmore_msg_vsm(client, &phashframe.cmd, sizeof(uint8_t));
  sendmore_msg_vsm(client, &phashframe.earlyendflag, sizeof(uint8_t));
  sendmore_msg_vsm(client, &phashframe.amount, sizeof(uint8_t));
  sendmore_msg_vsm(client, &phashframe.id, sizeof(uint8_t));
  sendmore_msg_vsm(client, &phashframe.nbframes, sizeof(uint32_t));
  send_msg_data(client, phashframe.phash, phashframe.nbframes*sizeof(uint32_t), NULL, NULL);
  if (phashframe.earlyendflag == 0)
  {
    Logging("send:cmd:%d, earlyendflag:%d, amount:%d, id:%d, nbframes:%d, phash[0]:%X", 
      phashframe.cmd, phashframe.earlyendflag, phashframe.amount, phashframe.id , phashframe.nbframes, phashframe.phash[0]);
  }

  Sleep(1);

  // get result
  uint8* data = NULL;
  uint8_t result;
  int64_t more;
  size_t msg_size, more_size = sizeof(int64_t);
  recieve_msg(client, &msg_size, &more, &more_size, (void**)&data);
  if (msg_size == sizeof(uint8_t))
  {
    memcpy(&result, data, sizeof(uint8_t));
    Logging("Get result:%d", result);
  }

  zmq_close(client);
  zmq_term(context);
  
  return result;
}
