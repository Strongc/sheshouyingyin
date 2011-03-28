

#include <map>
#include <string>

#define ADD_REMOTECMD(cmd, msgid) \
  remotecmd[cmd] = msgid;

typedef struct {
  std::string cmd;
  UINT msgid;
  time_t timestamp;
} RemoteMsg;

#define MAX_REMOTEMSG_10 10
#define REMOTEMSG_CHANNELNAME "SPlayer_remote_msgchannle"

typedef std::map<std::string, int> REMOTECMD;

void GetRemoteCmdMap(REMOTECMD& remotecmd)
{
  ADD_REMOTECMD("play", 887);
  ADD_REMOTECMD("stop", 888);
}