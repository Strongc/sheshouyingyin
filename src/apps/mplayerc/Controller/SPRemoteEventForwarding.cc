#include "stdafx.h"
#include "SPRemoteEventForwarding.h"

void SPRemoteEventForwarding::Connect(typDispatch pfnDispatch)
{
  m_pfnDispatch = pfnDispatch;
}

void SPRemoteEventForwarding::dispatch(const RemoteMsg &msg)
{
  m_pfnDispatch(WM_COMMAND, msg.wParam, msg.lParam);
}