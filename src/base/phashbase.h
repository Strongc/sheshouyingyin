#ifndef PHASHBASE_H
#define PHASHBASE_H

#include <vector>

struct phashblock
{
  WAVEFORMATEX format;
  std::vector<unsigned char> phashdata;
  int phashcnt;
  int prevcnt;
  int type;
};

enum{
  PCM8 = 0,
  PCM16,
  PCM24,
  PCM32,
  FPCM32,
  FPCM64
};

enum {
  CFG_PHASHTIMES = 0,
  CFG_PHASHDATASECS,
  CFG_PHASHSTARTTIME
};

const int g_phash_collectcfg[] = {
  // times;
  4,
  // datasecs;
  10,
  // start collect time;
  0,  // this is a placeholder
  10,
  30,
  50,
  110
};


#endif //PHASHBASE_H