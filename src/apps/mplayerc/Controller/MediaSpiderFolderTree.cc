#include "stdafx.h"
#include "MediaSpiderFolderTree.h"
#include "..\Controller\PlayerPreference.h"
#include "..\Controller\SPlayerDefs.h"
#include "../Resource.h"
#include "../mplayerc.h"
#include "MediaCenterController.h"
#include <boost/filesystem.hpp>
#include <boost/regex.hpp>

////////////////////////////////////////////////////////////////////////////////
// Normal part
MediaSpiderFolderTree::MediaSpiderFolderTree()
{
  // Init the last search path from the database
  m_sLastSearchPath = PlayerPreference::GetInstance()->GetStringVar(STRVAR_LASTSPIDERPATH);

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
  // Store the last search path, if the path is NULL, then represent the last
  // search is a complete search
  PlayerPreference::GetInstance()->SetStringVar(STRVAR_LASTSPIDERPATH, m_sLastSearchPath);
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

bool _helper_sort_iterator_vector(const MediaTreeFolders::pre_order_iterator &first
                                , const MediaTreeFolders::pre_order_iterator &second)
{
  return first->folder_data.merit > second->folder_data.merit;
}

void MediaSpiderFolderTree::_Thread()
{
  using std::wstring;
  using std::vector;

  while (true)
  {
    // see if need to be stop
    SleepOrExit();

    // 只能先保存节点指针到vector里再排序了
    MediaTreeFolders &mediaTree = m_treeModel.mediaTree();

    typedef vector<MediaTreeFolders::pre_order_iterator> TreeIteratorVector;
    TreeIteratorVector vtIteratorTree;
    MediaTreeFolders::pre_order_iterator itTreeIterator = mediaTree.pre_order_begin();
    while (itTreeIterator != mediaTree.pre_order_end())
    {
      vtIteratorTree.push_back(itTreeIterator);
      ++itTreeIterator;
    }

    // sort the path according the merit by descending order
    std::sort(vtIteratorTree.begin(), vtIteratorTree.end(), _helper_sort_iterator_vector);

    // search the media files
    TreeIteratorVector::iterator it = vtIteratorTree.begin();
    time_t tCur = ::time(0);
    while (it != vtIteratorTree.end())
    {
      // see if need to be stop
      SleepOrExit();

      // search the path for media files
      std::wstring sFullPath = fullFolderPath(it->node());
      if ((*it)->tNextSpiderInterval == 0)
        Search(sFullPath);
      else if (((tCur - (*it)->tFolderCreateTime) % (*it)->tNextSpiderInterval) == 0)
        Search(sFullPath);

      ++it;
    }

    // sleep for a moment
    SleepOrExit(300);
  }
}

void MediaSpiderFolderTree::Search(const std::wstring &sFolder)
{
  using std::wstring;
  using std::vector;
  using boost::wregex;
  using boost::regex_replace;
  using namespace boost::filesystem;

  // ---------------------------------------------------------------------------
  // Note:search cur folder and its parent's sub folders(cur's brother folder)
  // ---------------------------------------------------------------------------

  // First, we analysis all the path we will search and store them into vector
  vector<wpath> vtAllPath;  // all paths need to search
  try
  {
    wpath ptCur(regex_replace(sFolder, wregex(L"\\\\*$"), L""));  // must remove the back slash
    wpath ptParent(ptCur.parent_path());

    directory_iterator it(ptParent);
    directory_iterator itEnd;
    while (it != itEnd)
    {
      SleepOrExit();

      if (is_directory(it->path())
       && !isHiddenPath(it->path().wstring()))
        vtAllPath.push_back(it->path());

      ++it;
    }
  }
  catch (const filesystem_error &err)
  {
  	Logging(err.what());
    return;
  }

  // Second, we search the media in the path we find in above step
  vector<wpath>::iterator itPath = vtAllPath.begin();
  while (itPath != vtAllPath.end())
  {
    SleepOrExit(80);

    try
    {
      directory_iterator it(*itPath);
      directory_iterator itEnd;

      while (it != itEnd)
      {
        SleepOrExit(80);

        if (IsSupportExtension(it->path().wstring())
         && !is_directory(it->path())
         && !isHiddenPath(it->path().wstring())
         && !IsFilteredItem(it->path().wstring()))
        {
          // check to see if the file is already exist in the tree
          media_tree::model::tagFileInfo fileInfo = m_treeModel.findFile(itPath->wstring(), it->path().filename().wstring());
          if (!fileInfo.isValid())
          {
            // add it to the folder tree
            MediaData md;
            md.path = itPath->wstring();
            md.filename = it->path().filename().wstring();
            m_treeModel.addFile(md);

            // add it to the media center for appending
            media_tree::model::tagFileInfo fileInfo = m_treeModel.findFile(itPath->wstring(), it->path().filename().wstring());
            if (fileInfo.isValid())
            {
              MediaCenterController::GetInstance()->AddNewFoundData(fileInfo.itFile);

              // notify this change to main frame window
              CMPlayerCApp *pApp = AfxGetMyApp();
              if (pApp)
              {
                CWnd *pWnd = pApp->GetMainWnd();
                if (::IsWindow(pWnd->m_hWnd))
                  pWnd->PostMessage(WM_COMMAND, ID_SPIDER_NEWFILE_FOUND);
              }
            }
            else
            {
              Logging(L"find file in spider fail(should never go fail)");
            }
          }
        }

        ++it;
      }
    }
    catch (const filesystem_error &err)
    {
      Logging(err.what());
    }

    ++itPath;
  }
}