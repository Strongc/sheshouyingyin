

#include <map>
#include <string>

#define ADD_REMOTECMD(cmd, msgid) \
  remotecmd[cmd] = msgid;

typedef std::map<std::string, int> REMOTECMD;

void GetRemoteCmdMap(REMOTECMD& remotecmd)
{
  ADD_REMOTECMD("play", 1);
  ADD_REMOTECMD("stop", 2);
}