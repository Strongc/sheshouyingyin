
#pragma once
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// some CString constructors will be explicit

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers
#endif

#include "targetver.h"

#include <afx.h>
#include <afxwin.h>         // MFC core and standard components
#include <atlcoll.h>

#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <share.h>

#include <string>
#include <vector>

#define SVP_MIN(a, b)  (((a) < (b)) ? (a) : (b)) 
#define SVP_MAX(a, b)  (((a) > (b)) ? (a) : (b)) 

//#define SVP_DEBUG_LOGFILEPATH _T(".\\SVPDebug.log")
void SVP_LogMsg(CString logmsg, int level = 15);
BOOL SVP_CanUseCoreAvcCUDA(BOOL useCUDA);
//extern void SVP_RealCheckUpdaterExe(BOOL* bCheckingUpdater);
//extern BOOL SVP_SetCoreAvcCUDA(BOOL useCUDA);
//extern BOOL SVP_ForbidenCoreAVCTrayIcon();
void SVP_LogMsg2(LPCTSTR fmt, ...);
void SVP_LogMsg3(LPCSTR fmt, ...);
void SVP_LogMsg4(BYTE* buff, __int64 iLen);
void SVP_LogMsg5(LPCTSTR fmt, ...);
void SVP_LogMsg6(LPCSTR fmt, ...);
static UINT logTick = 0;