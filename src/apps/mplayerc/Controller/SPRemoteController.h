
#pragma once

#include "threadhelper.h"
#include "boost/interprocess/ipc/message_queue.hpp"

class SPRemoteController:
  public ThreadHelperImpl<SPRemoteController>
{
public:
  SPRemoteController();
  ~SPRemoteController();

  void _Thread();
  void SetFrame(HWND hwnd);
  void Clean();
  void InitQ();
  BOOL IsRun();

private:
  HWND m_wndframe;
  boost::interprocess::message_queue* m_q;
  std::string m_queuename;
  BOOL m_running;
};