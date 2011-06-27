#pragma once

#include <threadhelper.h>
#include "MediaModel.h"
#include "MCBlockUnit.h"
#include <vector>

#define MCDEBUG(str) \
  if (::GetStdHandle(STD_OUTPUT_HANDLE) == 0) \
  ::AllocConsole(); \
  ::WriteConsole(::GetStdHandle(STD_OUTPUT_HANDLE), str, str.GetLength(), 0, 0);Logging(log);

class MCDBSource:
  public ThreadHelperImpl<MCDBSource>
{
public:
  typedef std::vector<BlockUnit*>::iterator BUPOINTER;
  
  // db & reader status
  enum
  {
    MCDB_MORE,
    MCDB_TOEND,
    MCDB_TOSTART
  };


  MCDBSource(void);
  ~MCDBSource(void);

  #define MCLoopList(P)           \
    {                               \
      MCDBSource::BUPOINTER sp;     \
      MCDBSource::BUPOINTER ep;     \
      P->GetPointer(sp, ep);      \
      for (;sp != ep; sp++)  \
      {

  #define MCLoopOne() (*sp)
  #define MCEndLoop()               \
      }                             \
    }

  #define MCLoopInit(P) \
      MCDBSource::BUPOINTER sp;     \
      MCDBSource::BUPOINTER ep;     \
                        P->GetPointer(sp, ep);
  #define MCLoopCurData()  ((*sp))
  #define MCLoopNextData()  if(sp!=ep)sp++;

  BOOL PreLoad(UINT nums);
  void HideData(const MediaData &md);
  void Reload();   // refresh current datas
  void CleanData();

  void AdjustRange(UINT nums, UINT columns);
  BOOL IsMoreData();

  int LoadRowDatas(BOOL direction, UINT movenums);

  void SetReadNums(UINT nums);

  void _Thread();

  inline void GetPointer(BUPOINTER& sp, BUPOINTER& ep) {sp = m_sp;ep = m_ep;}
  inline void GetData(BUPOINTER& sp) {sp = m_sp;}
  inline int GetReaderStatus() {return m_readerstatus;}
  
private:
  // reader and it's position
  MediaModel m_db;
  UINT m_pos;
  UINT m_total;

  // m_frontbuff pointer & readable numbers
  UINT m_readnums;
  BUPOINTER m_sp;
  BUPOINTER m_ep;

  // temp buffer for save db data
  MediaDatas m_buffer;
  std::vector<BlockUnit*> m_frontbuff;

  // determine the db read direction
  BOOL m_direction;
  BOOL m_stopdb;

  int m_readerstatus;
  BOOL m_isadjust;
  CString log;
};
