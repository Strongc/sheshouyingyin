#pragma once

#include <threadhelper.h>
#include "NetworkControlerImpl.h"
#include "../Model/MediaComm.h"
#include "../UserInterface/Renderer/BlockList.h"

class CoverDownloadController :
  public ThreadHelperImpl<CoverDownloadController>,
  public NetworkControlerImpl
{
public:
  CoverDownloadController();
  ~CoverDownloadController();

  void SetBlockUnit(BlockUnit* unit);
  void _Thread();
  void _Thread_old();

private:
  std::list<BlockUnit*> m_list;
};