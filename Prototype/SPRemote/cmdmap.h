#pragma once

#include <map>
#include <string>
#include "../../src/apps/mplayerc/resource.h"

#define ADD_REMOTECMD(cmd, msgid) \
  remotecmd[cmd] = msgid;

typedef struct {
  std::string cmdstring;
  UINT msgid;
  time_t timestamp;
  WPARAM wParam;
  LPARAM lParam;
} RemoteMsg;

#define MAX_REMOTEMSG_10 10
#define REMOTEMSG_CHANNELNAME "SPlayerMsgChannle"

struct tagRemoteCmdInfo
{
  tagRemoteCmdInfo(UINT id = 0, WPARAM w = 0, LPARAM l = 0) : msgid(id), wParam(w), lParam(l) {}
  UINT msgid;
  WPARAM wParam;
  LPARAM lParam;
};
typedef std::map<std::string, tagRemoteCmdInfo> REMOTECMD;

inline void GetRemoteCmdMap(REMOTECMD& remotecmd)
{
  ADD_REMOTECMD("play", tagRemoteCmdInfo(ID_PLAY_PLAY, ID_PLAY_PLAY, 0));
  ADD_REMOTECMD("pause", tagRemoteCmdInfo(ID_PLAY_PAUSE, ID_PLAY_PAUSE, 0));
  ADD_REMOTECMD("stop", tagRemoteCmdInfo(ID_FILE_CLOSEPLAYLIST, ID_FILE_CLOSEPLAYLIST, 0));
  ADD_REMOTECMD("prev", tagRemoteCmdInfo(ID_NAVIGATE_SKIPBACKPLITEM, ID_NAVIGATE_SKIPBACKPLITEM, 0));
  ADD_REMOTECMD("next", tagRemoteCmdInfo(ID_NAVIGATE_SKIPFORWARDPLITEM, ID_NAVIGATE_SKIPFORWARDPLITEM, 0));
  //ADD_REMOTECMD("prev", tagRemoteCmdInfo(ID_NAVIGATE_SKIPBACKPLITEM, ID_NAVIGATE_SKIPBACKPLITEM, ID_NAVIGATE_SKIPBACKPLITEM));
  //ADD_REMOTECMD("next", tagRemoteCmdInfo(ID_NAVIGATE_SKIPFORWARDPLITEM, ID_NAVIGATE_SKIPFORWARDPLITEM, ID_NAVIGATE_SKIPFORWARDPLITEM));
}