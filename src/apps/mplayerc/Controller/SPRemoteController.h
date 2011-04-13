
#pragma once

#include "threadhelper.h"
#include "boost/interprocess/ipc/message_queue.hpp"
#include "SPRemoteEventForwarding.h"

class SPRemoteController:
  public ThreadHelperImpl<SPRemoteController>
{
public:
  SPRemoteController();
  ~SPRemoteController();

  void _Thread();
  void Clean();
  void InitQ();
  BOOL IsRun();

private:
  SPRemoteEventForwarding m_event;
  boost::interprocess::message_queue* m_q;
  std::string m_queuename;
  BOOL m_running;
};