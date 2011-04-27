// SPRemote.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "mongoose.h"
#include "Windows.h"
#include <string>
#include "cmdmap.h"
#include "boost/interprocess/ipc/message_queue.hpp"

#define SPREMOTE_PORT "8874"
static wchar_t *service_name = L"SPRemote";
static wchar_t *service_desc = L"SPlayer Service. See more http://splayer.org.";

enum 
{
  REGISTER_REMOTE_SERVICE,
  UNREGISTER_REMOTE_SERVICE
};

void show_error(void);
BOOL RemoteService(int action);
void WINAPI ServiceMain(void);
void WINAPI ControlHandler(DWORD code);

static SERVICE_TABLE_ENTRY service_table[] = {
  {service_name, (LPSERVICE_MAIN_FUNCTION)ServiceMain},
  {NULL, NULL}
};

static SERVICE_STATUS ss;
static SERVICE_STATUS_HANDLE hStatus;
static struct mg_context* ctx;
static REMOTECMD cmds;
static wchar_t path[MAX_PATH];

DWORD GetSPlayerPID()
{
  DWORD pid = 0;
  HWND wnd = FindWindow(L"SVPLayered", NULL);
  if (wnd)
    GetWindowThreadProcessId(wnd, &pid);

  return pid;
}

void* SPCall(enum mg_event event,
              struct mg_connection *conn,
              const struct mg_request_info *request_info)
{
  if (event != MG_NEW_REQUEST)
    return NULL;

  char* uri = request_info->uri;
  char* query = request_info->query_string;

  /* Remote command format:
   * http://xxx.xxx.xxx/command.html?id={PID}&wm_command={CMD}&p={PARAM}
   * 
   * {PID} is a splayer process id(if not define then it will do find by GetSplayerPID).
   * {CMD} is a one of follow play/pause/next/...
   * {PARAM} is a append value
   */

  // {SIGN}
  if (!strcmp("/command.html", uri))
  {
    char val[128];
    mg_get_var(query, strlen(query), "id", val, 127);
    
    if (val[0] == 0)
      sprintf_s(val, sizeof(val), "%d", GetSPlayerPID());
    std::string queuename = REMOTEMSG_CHANNELNAME;
    queuename += val;

    mg_get_var(query, strlen(query), "wm_command", val, 127);
    std::string cmd = val;
    
    mg_get_var(query, strlen(query), "p", val, 127);
    std::string param = val;

    if (!cmds[cmd].msgid)
      send_http(conn, 500, "error url");
    else
    {
      RemoteMsg msg;
      msg.timestamp = time(NULL);
      msg.cmdstring = param;
      msg.msgid = cmds[cmd].msgid;
      msg.wParam = cmds[cmd].wParam;
      msg.lParam = cmds[cmd].lParam;

      try
      {
        boost::interprocess::message_queue mq(boost::interprocess::open_only,
          queuename.c_str());

        mq.send(&msg, sizeof(msg), 0);
        send_http(conn, 200, "OK");
      }
      catch (...)
      {
        send_http(conn, 500, "Command send fail");
      }
    }

    // if return NULL, this request do other thing.
    return &SPCall;
  }
  return NULL;
}

/*
 * Execute Arguments help
 * -i Register to windows service
 * -u Unregister service
 * -d for service start (for service)
 */

int _tmain(int argc, _TCHAR* argv[])
{
  GetModuleFileName(NULL, path, sizeof(path));
  wchar_t* cmd = argv[1];

  if (cmd == NULL)
  {
    //return 0;
    GetRemoteCmdMap(cmds);
    char* tmp = new char[wcslen(path)+1];
    size_t n;
    wcstombs_s(&n, tmp, wcslen(path)+1, path, wcslen(path));
    std::string root;
    root.assign(tmp);
    delete [] tmp;
    root = root.substr(0, root.find_last_of('\\'));
    const char *options[] = {
      "document_root",  root.c_str(),
      "listening_ports", SPREMOTE_PORT,
      NULL
    };
    ctx = mg_start(&SPCall, options);
    Sleep(1000*3600);
    mg_stop(ctx);
  }
  else if (cmd[0] == '-' && cmd[1] == 'h')
    printf("\nSPRemote Command:\n \
    -h command list\n \
    -i Register to windows service.\n \
    -u Unregister service.\n");

  else if (cmd[0] == '-' && cmd[1] == 'i')
    RemoteService(REGISTER_REMOTE_SERVICE);

  else if (cmd[0] == '-' && cmd[1] == 'u')
    RemoteService(UNREGISTER_REMOTE_SERVICE);

  else if (cmd[0] == '-' && cmd[1] == 'd')
  {
    SC_HANDLE hservice;
    SC_HANDLE hscm;
    SERVICE_STATUS status;
    hscm = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
    if (hscm)
    {
      hservice = OpenService(hscm, service_name, GENERIC_READ);
      if (hservice && QueryServiceStatus(hservice, &status)
          && status.dwCurrentState == SERVICE_START_PENDING)
      {
        CloseServiceHandle(hservice);
        CloseServiceHandle(hscm);

        if (!StartServiceCtrlDispatcher(service_table))
          printf("StartServiceCtrlDispatcher error = %ld\n", GetLastError());
      }
    }
  }
	return 0;
}

void WINAPI ControlHandler(DWORD code)
{
  if (code == SERVICE_CONTROL_STOP || code == SERVICE_CONTROL_SHUTDOWN)
  {
    ss.dwWin32ExitCode = 0;
    ss.dwCurrentState = SERVICE_STOPPED;
  }
  SetServiceStatus(hStatus, &ss);
}

void WINAPI ServiceMain(void)
{
  ss.dwServiceType = SERVICE_WIN32;
  ss.dwCurrentState = SERVICE_RUNNING;
  ss.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;
  
  GetRemoteCmdMap(cmds);

  size_t n;
  std::string root;
  char* tmp = new char[wcslen(path)+1];
  wcstombs_s(&n, tmp, wcslen(path)+1, path, wcslen(path));
  root.assign(tmp);
  delete [] tmp;
  root = root.substr(0, root.find_last_of('\\'));
  const char *options[] = {
    "document_root",  root.c_str(),
    "listening_ports", SPREMOTE_PORT,
    NULL
  };
  ctx = mg_start(&SPCall, options);

  hStatus = RegisterServiceCtrlHandler(service_name, ControlHandler);
  SetServiceStatus(hStatus, &ss);

  while (ss.dwCurrentState == SERVICE_RUNNING)
    Sleep(1000);

  mg_stop(ctx);

  ss.dwCurrentState = SERVICE_STOPPED;
  ss.dwWin32ExitCode = (DWORD) -1;
  SetServiceStatus(hStatus, &ss);
}

BOOL RemoteService(int action)
{
  SC_HANDLE hscm = NULL;
  SC_HANDLE hservice = NULL;

  hscm = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
  if (!hscm)
  {
    show_error();
    return FALSE;
  }
  if (action == REGISTER_REMOTE_SERVICE)
  {
    SERVICE_DESCRIPTION descr = {service_desc};
    wcsncat_s(path, MAX_PATH, L" -d", 3);
    hservice = CreateService(hscm, service_name, service_name,
      SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS,
      SERVICE_AUTO_START, SERVICE_ERROR_NORMAL,
      path, NULL, NULL, NULL, NULL, NULL);

    if (hservice)
      ChangeServiceConfig2(hservice, SERVICE_CONFIG_DESCRIPTION, &descr);
    else
      show_error();

    // Can't immediate start after create a new service.
    // That's cause of call Sleep.
    Sleep(2000);
    if (!StartService(hservice, NULL, NULL))
      show_error();
  }
  else if (action == UNREGISTER_REMOTE_SERVICE)
  {
    hservice = OpenService(hscm, service_name, SERVICE_ALL_ACCESS);
    if (hservice)
    {
      SERVICE_STATUS status;
      SERVICE_STATUS errorstatus;
      if (QueryServiceStatus(hservice, &status)
        && status.dwCurrentState == SERVICE_RUNNING)
      {
        if (ControlService(hservice, SERVICE_CONTROL_STOP, &errorstatus))
          // try to wait 2 seconds. it's bad.
          // The best way is to call the QueryServiceStatus
          // in 30 seconds timeout in until the status is SERVICE_STOP.
          Sleep(2000);
      }
      DeleteService(hservice);
    }
    else
      show_error();
  }

  CloseServiceHandle(hservice);
  CloseServiceHandle(hscm);
  return TRUE;
}

void show_error(void)
{
  wchar_t buf[256];
  FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
    NULL, GetLastError(),
    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
    buf, sizeof(buf), NULL);
  MessageBox(NULL, buf, L"Error", MB_OK);
}