#pragma once

#include "..\..\..\..\Prototype\SPRemote\cmdmap.h"
#include <boost/function.hpp>

typedef boost::function<BOOL (UINT msg, WPARAM wParam, LPARAM lParam)> typDispatch;

// event forwarding class
class SPRemoteEventForwarding
{
public:
  void Connect(typDispatch pfnDispatch);
  void dispatch(const RemoteMsg &msg);

private:
  typDispatch m_pfnDispatch;
};