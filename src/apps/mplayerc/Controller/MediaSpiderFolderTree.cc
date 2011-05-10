#include "stdafx.h"
#include "MediaSpiderFolderTree.h"
#include <boost/filesystem.hpp>
#include <boost/regex.hpp>
#include "../Model/MediaDB.h"

////////////////////////////////////////////////////////////////////////////////
// Normal part
MediaSpiderFolderTree::MediaSpiderFolderTree()
: m_tSleep(1)
, m_nSpideInterval(5)
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

  while (true)
  {
    // fetch a record
    time_t tCur = ::time(0);
    std::wstringstream ssSQL;
    ssSQL << L"SELECT path, lastspidetime, breakpoint FROM detect_path ORDER BY merit DESC";
    vector<wstring> vtPath;
    vector<time_t> vtLastSpideTime;
    vector<bool> vtBreakPoint;
    MediaDB<wstring, time_t, bool>::exec(ssSQL.str(), &vtPath, &vtLastSpideTime, &vtBreakPoint);

    // spider the first record
    wstring sPath;
    time_t tLastSpideTime = 0;
    if (!vtPath.empty())
    {
      // find the last breakpoint
      int i = 0;
      for (i = 0; i < vtBreakPoint.size(); ++i)
      {
        if (vtBreakPoint[i] == true)
          break;
      }

      if ((i == vtBreakPoint.size()) || (i == vtBreakPoint.size() - 1))
      {
        // not find last position, always the first search
        // OR find the position, already the last position
        // next search path is the first item in the vector
        sPath = vtPath[0];
        tLastSpideTime = vtLastSpideTime[0];
      } 
      else
      {
        // find the position
        // next search path is the next item in the vector
        sPath = vtPath[i + 1];
        tLastSpideTime = vtLastSpideTime[i + 1];
      }

      // search the folder when the interval is greater than m_nSpideInterval
      if ((tCur - tLastSpideTime) > m_nSpideInterval)
      {
        Search(sPath);

        ssSQL.str(L"");
        ssSQL << L"UPDATE detect_path SET lastspidetime=" << tCur
          << L" WHERE path='" << MediaModel::EscapeSQL(sPath) << L"'";
        MediaDB<>::exec(ssSQL.str());  // save last spide time
      }

      // update the database
      MediaDB<>::exec(L"begin transaction");

      ssSQL.str(L"");
      ssSQL << L"UPDATE detect_path SET breakpoint=0";
      MediaDB<>::exec(ssSQL.str());  // reset the breakpoint

      ssSQL.str(L"");
      ssSQL << L"UPDATE detect_path SET "
        << L" breakpoint=" << (int)true
        << L" WHERE path='" << MediaModel::EscapeSQL(sPath) << L"'";
      MediaDB<>::exec(ssSQL.str());  // update the search path's info

      MediaDB<>::exec(L"end transaction");
    }

    // see if need to be stop
    if (_Exit_state(0))
      return;

    // sleep for next loop
    ::Sleep(m_tSleep * 1000);
  }
}

void MediaSpiderFolderTree::Search(const std::wstring &sFolder)
{
  using std::wstring;
  using std::vector;
  using boost::wregex;
  using boost::regex_replace;
  using namespace boost::filesystem;

  // search media in the path
  try
  {
    directory_iterator it(sFolder);
    directory_iterator itEnd;

    // search
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