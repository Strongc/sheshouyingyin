#include "StdAfx.h"

#include "SPRemoteController.h"
#include "../../../../Prototype/SPRemote/cmdmap.h"

SPRemoteController::SPRemoteController() :
  m_wndframe(NULL),
  m_running(FALSE),
  m_q(NULL)
{
  char pid[16];
  sprintf_s(pid, 16, "%u", GetCurrentProcessId());
  m_queuename = REMOTEMSG_CHANNELNAME;
  m_queuename += pid;
  Logging("Remote URL: http://127.0.0.1:8080/splayer/%s/play|stop|next|prev", pid);
  InitQ();
}

SPRemoteController::~SPRemoteController()
{
  _Stop();
  Clean();
}

void SPRemoteController::SetFrame(HWND hwnd)
{
  m_wndframe = hwnd;
}

BOOL SPRemoteController::IsRun()
{
  return m_running;
}

void SPRemoteController::InitQ()
{
  Clean();

  try
  {
    m_q = new boost::interprocess::message_queue(boost::interprocess::create_only,
      m_queuename.c_str(), MAX_REMOTEMSG_10, sizeof(RemoteMsg));
  }
  catch (boost::interprocess::interprocess_exception& e)
  {
    Logging("Can't create message queue. %s", e.what());
  }
}

void SPRemoteController::_Thread()
{
  if (!m_wndframe || !m_q)
    return;

  m_running = TRUE;
  try
  {
    while (true)
    {
      if (_Exit_state(2000))
      {
        Clean();
        return;
      }

      RemoteMsg msg;
      size_t recvsize;
      unsigned int priority;
      time_t timestamp = time(NULL);
      if (m_q->try_receive(&msg, sizeof(RemoteMsg), recvsize, priority)
        && timestamp - msg.timestamp < 4)
      {
        std::string param = msg.cmdstring;
        if (msg.msgid == 919 || msg.msgid == 918)
          ::PostMessage(m_wndframe, WM_COMMAND, msg.msgid, (LPARAM)msg.msgid);
        else
        ::PostMessage(m_wndframe, WM_COMMAND, msg.msgid, NULL);
      }
    }
  }
  catch (boost::interprocess::interprocess_exception& e)
  {
    Clean();
    Logging("Receive message error from queue. %s", e.what());
  }
}

void SPRemoteController::Clean()
{
  m_running = FALSE;
  boost::interprocess::message_queue::remove(m_queuename.c_str());

  if (m_q)
    delete m_q;
}
