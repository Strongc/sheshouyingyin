#include "stdafx.h"
#include "MediaSpiderFolderTree.h"
#include <boost/filesystem.hpp>
#include <boost/regex.hpp>
#include "../Model/MediaDB.h"
#include "../Utils/SPlayerGUID.h"
#include "MediaCenterController.h"

////////////////////////////////////////////////////////////////////////////////
// Normal part
MediaSpiderFolderTree::MediaSpiderFolderTree()
: m_tSleep(1)
, m_nSpideInterval(5)
, m_nThreadStartInterval(30)
{
  // Init the media type and the exclude folders & files from the database
  // Note: the case is not sensitive, will change it to lowercase before compare
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

  while (true)
  {
    wstring sPath;
    UINT uniqueid = 0;
    time_t lasttime = ::time(0) - m_nThreadStartInterval;

    ssSQL.str(L"");
    ssSQL << L"SELECT uniqueid, path FROM detect_path WHERE lasttime <="
          << lasttime
          << L" ORDER BY merit limit 0, 1";
    MediaDB<UINT, wstring>::exec(ssSQL.str(), &uniqueid, &sPath);

    if (uniqueid && Search(sPath))
    {
      ssSQL.str(L"");
      ssSQL << L"UPDATE detect_path SET "
            << L" lasttime=" << ::time(0)
            << L" WHERE uniqueid=" << uniqueid;
      MediaDB<>::exec(ssSQL.str());
    }

    if (_Exit_state(m_tSleep * 1000))
    {
      ssSQL.str(L"");
      ssSQL << L"DELETE FROM spider_info";
      MediaDB<>::exec(ssSQL.str());
      return;
    }
  }
}

BOOL MediaSpiderFolderTree::Search(const std::wstring &sFolder)
{
  using std::wstring;
  using std::vector;
  using boost::wregex;
  using boost::regex_replace;
  using namespace boost::filesystem;

  // check if we should spider this folder
  if (0 && !IsValidPath(sFolder))
    return FALSE;

  try
  {
    // First, fetch all db's data to tree
    vector<wstring> vtPath;
    vector<wstring> vtFilename;
    vector<wstring> vtFilmname;
    vector<wstring> vtThumbnailPath;
    vector<bool> vtHide;

    std::wstringstream ssSQL;
    ssSQL.str(L"");
    ssSQL << L"SELECT path, filename, filmname, thumbnailpath, hide FROM media_data WHERE path='" << sFolder << L"'";
    MediaDB<wstring, wstring, wstring, wstring, bool>::exec(ssSQL.str(), &vtPath, &vtFilename, &vtFilmname, &vtThumbnailPath, &vtHide);
    for (size_t i = 0; i < vtPath.size(); ++i)
    {
      MediaData md;
      md.path = vtPath[i];
      md.filename = vtFilename[i];
      md.filmname = vtFilmname[i];
      md.bHide = vtHide[i];

      // dynamic get the cover path first
      // if fail, then use database's thumbnail path
      md.thumbnailpath = MediaCenterController::GetCoverPath(md.path + md.filename);
      if (md.thumbnailpath.empty())
        md.thumbnailpath = vtThumbnailPath[i];

      m_treeModel.addFile(md);

      // add media to cover thread to get the cover
      MediaCenterController::GetInstance()->GetCoverDownload().SetBlockUnit(md);

      if (_Exit_state(0))
        return FALSE;
      ::Sleep(300);
    }

    // Second, search media in the path
    directory_iterator it(sFolder);
    directory_iterator itEnd;
    while (it != itEnd)
    {
      if (_Exit_state(300))
        return FALSE;

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
    return FALSE;
  }
  return TRUE;
}