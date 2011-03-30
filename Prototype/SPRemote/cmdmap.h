

#include <map>
#include <string>

#define ADD_REMOTECMD(cmd, msgid) \
  remotecmd[cmd] = msgid;

typedef struct {
  std::string cmdstring;
  UINT msgid;
  time_t timestamp;
} RemoteMsg;

#define MAX_REMOTEMSG_10 10
#define REMOTEMSG_CHANNELNAME "SPlayerMsgChannle"

typedef std::map<std::string, int> REMOTECMD;

void GetRemoteCmdMap(REMOTECMD& remotecmd)
{
  ADD_REMOTECMD("play", 887);
  ADD_REMOTECMD("stop", 888);
  ADD_REMOTECMD("next", 919);
  ADD_REMOTECMD("prev", 918);
}