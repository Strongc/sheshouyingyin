#include "stdafx.h"
#include "MediaSpiderFolderTree.h"
#include <boost/filesystem.hpp>
#include <boost/regex.hpp>
#include "../Model/MediaDB.h"
#include "../Utils/SPlayerGUID.h"

////////////////////////////////////////////////////////////////////////////////
// Normal part
MediaSpiderFolderTree::MediaSpiderFolderTree()
: m_tSleep(1)
, m_nSpideInterval(5)
, m_nThreadStartInterval(30)
{
  // Init the media type and the exclude folders & files from the database
  // Warning: the case is sensitive !!!
  SetSupportExtension(L".avi");
  SetSupportExtension(L".wmv");
  SetSupportExtension(L".mkv");
  SetSupportExtension(L".rmvb");
  SetSupportExtension(L".rm");
  SetSupportExtension(L".asf");
  SetSupportExtension(L".mov");
  SetSupportExtension(L".mp4");
  SetSupportExtension(L".mpeg");
  SetSupportExtension(L".3gp");
  SetSupportExtension(L".divx");

  SetExcludePath(L"c:\\Windows\\");
  SetExcludePath(L"C:\\Program Files\\");
  SetExcludePath(L"C:\\Program Files (x86)\\");

  SetFilteredItem(L"private");
}

MediaSpiderFolderTree::~MediaSpiderFolderTree()
{
}

//void cDebug(const std::wstring &sDebugInfo, bool bAutoBreak = true)
//{
//  if (::GetStdHandle(STD_OUTPUT_HANDLE) == 0)
//    ::AllocConsole();
//
//  HANDLE h = ::GetStdHandle(STD_OUTPUT_HANDLE);
//
//  if (bAutoBreak)
//  {
//    ::WriteConsole(h, sDebugInfo.c_str(), sDebugInfo.size(), 0, 0);
//    ::WriteConsole(h, L"\n", 1, 0, 0);
//  } 
//  else
//  {
//    ::WriteConsole(h, sDebugInfo.c_str(), sDebugInfo.size(), 0, 0);
//  }
//}

void MediaSpiderFolderTree::_Thread()
{
  using std::wstring;
  using std::vector;

  time_t tCur = ::time(0);
  std::wstringstream ssSQL;

  bool bRun = false;
  time_t tLast = 0;
  ssSQL.str(L"");
  ssSQL << L"SELECT already_run, last_time FROM spider_info";
  MediaDB<bool, time_t>::exec(ssSQL.str(), &bRun, &tLast);

  if (MediaDB<>::last_error() != 0)
    return;

  if (!bRun)
  {
    ssSQL.str(L"");
    ssSQL << L"INSERT INTO spider_info(already_run, last_time) VALUES(1, " << ::time(0) << L")";
    MediaDB<>::exec(ssSQL.str());

    if (MediaDB<>::last_error() != 0)
      return;
  } 
  else
  {
    if (::time(0) - tLast >= m_nThreadStartInterval)
    {
      ssSQL.str(L"");
      ssSQL << L"UPDATE spider_info SET last_time=" << ::time(0);
      MediaDB<>::exec(ssSQL.str());

      if (MediaDB<>::last_error() != 0)
        return;
    }
    else
    {
      return;
    }
  }

  // do something
  bool bBreakType = 1;
  while (true)
  {
    // fetch a record
    wstring sPath;
    time_t tLastSpideTime = 0;
    ssSQL.str(L"");
    ssSQL << L"SELECT path, lastspidetime FROM detect_path WHERE breakpoint=" << (int)bBreakType << L" ORDER BY merit";
    MediaDB<wstring, time_t>::exec(ssSQL.str(), &sPath, &tLastSpideTime);

    // update database
    ssSQL.str(L"");
    ssSQL << L"UPDATE detect_path SET "
      << L" breakpoint=" << (int)(!bBreakType)
      << L" WHERE path='" << MediaModel::EscapeSQL(sPath) << L"'";
    MediaDB<>::exec(ssSQL.str());  // update the search path's info
    
    if (MediaDB<>::last_changes() != 0)
    {
      Search(sPath);
    }
    else
    {
      bBreakType = !bBreakType;
    }

    if (_Exit_state(0))
    {
      ssSQL.str(L"");
      ssSQL << L"DELETE FROM spider_info";
      MediaDB<>::exec(ssSQL.str());
      return;
    }
  }
}

void MediaSpiderFolderTree::Search(const std::wstring &sFolder)
{
  using std::wstring;
  using std::vector;
  using boost::wregex;
  using boost::regex_replace;
  using namespace boost::filesystem;

  try
  {
    // First, fetch all db's data to tree and delete these data
    vector<wstring> vtPath;
    vector<wstring> vtFilename;
    vector<wstring> vtThumbnailPath;
    vector<bool> vtHide;

    std::wstringstream ssSQL;
    ssSQL.str(L"");
    ssSQL << L"SELECT path, filename, thumbnailpath, hide FROM media_data WHERE path='" << sFolder << L"'";
    MediaDB<wstring, wstring, wstring, bool>::exec(ssSQL.str(), &vtPath, &vtFilename, &vtThumbnailPath, &vtHide);
    for (int i = 0; i < vtPath.size(); ++i)
    {
      MediaData md;
      md.path = vtPath[i];
      md.filename = vtFilename[i];
      md.thumbnailpath = vtThumbnailPath[i];
      md.bHide = vtHide[i];
      m_treeModel.addFile(md);
    }

    ssSQL.str(L"");
    ssSQL << L"DELETE FROM media_data WHERE path='" << sFolder << L"'";
    MediaDB<>::exec(ssSQL.str());

    // Second, search media in the path
    directory_iterator it(sFolder);
    directory_iterator itEnd;

    while (it != itEnd)
    {
      if (_Exit_state(0))
        return;

      if (IsSupportExtension(it->path().wstring())
       && !is_directory(it->path())
       && !isHiddenPath(it->path().wstring())
       && !IsFilteredItem(it->path().wstring()))
      {
        MediaData md;
        md.path = sFolder;
        md.filename = it->path().filename().wstring();
        m_treeModel.addFile(md);
              //MediaCenterController::GetInstance()->AddNewFoundData(fileInfo.itFile);
//               CMPlayerCApp *pApp = AfxGetMyApp();
//               if (pApp)
//               {
//                 CWnd *pWnd = pApp->GetMainWnd();
//                 if (::IsWindow(pWnd->m_hWnd))
//                   pWnd->PostMessage(WM_COMMAND, ID_SPIDER_NEWFILE_FOUND);
//               }
      }

      ++it;
    }

    // store info to db
    m_treeModel.save2DB();

    // delete tree
    m_treeModel.delTree();
  }
  catch (const filesystem_error &err)
  {
    Logging(err.what());
  }
}