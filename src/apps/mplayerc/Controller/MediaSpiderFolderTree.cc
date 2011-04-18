#include "stdafx.h"
#include "MediaSpiderFolderTree.h"
#include "..\Controller\PlayerPreference.h"
#include "..\Controller\SPlayerDefs.h"
#include "../Resource.h"
#include "../mplayerc.h"
#include "MediaCenterController.h"
#include <boost/filesystem.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>
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

void MediaSpiderFolderTree::_Thread()
{
  using namespace boost::lambda;
  using std::wstring;
  using std::vector;

  while (true)
  {
    // see if need to be stop
    if (_Exit_state(0))
      return;

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
    std::sort(vtIteratorTree.begin(), vtIteratorTree.end(), 
              bind(&media_tree::folder::nMerit, *_1) > bind(&media_tree::folder::nMerit, *_2));

    // search the media files
    TreeIteratorVector::iterator it = vtIteratorTree.begin();
    time_t tCur = ::time(0);
    while (it != vtIteratorTree.end())
    {
      // see if need to be stop
      if (_Exit_state(0))
        return;

      // search the path for media files
      std::wstring sFullPath = fullFolderPath(it->node());
      if ((*it)->tNextSpiderInterval == 0)
        Search(sFullPath);
      else if (((tCur - (*it)->tFolderCreateTime) % (*it)->tNextSpiderInterval) == 0)
        Search(sFullPath);

      ++it;
    }

    // sleep for a moment
    ::Sleep(300);
  }
}

void MediaSpiderFolderTree::Search(const std::wstring &sFolder)
{
  using std::wstring;
  using std::vector;
  using namespace boost::filesystem;
  using boost::wregex;
  using boost::regex_replace;

  // ---------------------------------------------------------------------------
  // Note:search current folder and its parent's sub folders
  // ---------------------------------------------------------------------------

  // if the folder is not exist or the folder is been exclude, then return
  if (!is_directory(sFolder) || IsExcludePath(sFolder))
    return;

  // search the folder
  wpath folder(sFolder);
  directory_iterator itCur(folder);
  directory_iterator itEnd;

  while (itCur != itEnd)
  {
    // see if need to be stop
    if (_Exit_state(0))
      return;

    if (!is_directory(itCur->path()) && IsSupportExtension(itCur->path().wstring()))
    {
      // add it to the folder tree
      m_treeModel.addFile(sFolder, itCur->path().filename().wstring());

      // add it to the media center for appending
      MediaData md;
      md.path = sFolder;
      md.filename = itCur->path().filename().wstring();

      MediaCenterController::GetInstance()->AddNewFoundData(md);

      // notify this change to main frame window
      CMPlayerCApp *pApp = AfxGetMyApp();
      if (pApp)
      {
        CWnd *pWnd = pApp->GetMainWnd();
        if (pWnd)
          pWnd->PostMessage(WM_COMMAND, ID_SPIDER_NEWFILE_FOUND);
      }
    }

    ++itCur;

    // sleep for a moment
    ::Sleep(50);
  }

  // search the current folder's parent's sub folders(current folder's brother)
  wpath ptParent(folder.parent_path());
  directory_iterator itParentCur(ptParent);
  directory_iterator itParentEnd;
  while (itParentCur != itParentEnd)
  {
    // see if need to be stop
    if (_Exit_state(0))
      return;  
    
    if (is_directory(itParentCur->path()) && (itParentCur->path().wstring() != sFolder))
    {
      directory_iterator itSubCur(itParentCur->path());
      directory_iterator itSubEnd;
      while (itSubCur != itSubEnd)
      {
        // see if need to be stop
        if (_Exit_state(0))
          return;
        
        if (!is_directory(itSubCur->path()) && IsSupportExtension(itSubCur->path().wstring()))
        {
          // Note: do not add it to the folder tree
          //// add it to the folder tree
          MediaData md;
          md.path = itSubCur->path().parent_path().wstring();
          md.path = regex_replace(md.path, wregex(L"\\\\*$"), L"\\\\");
          md.filename = itSubCur->path().filename().wstring();

          //m_treeModel.addFile(md);

          // add it to the media center for appending
          MediaCenterController::GetInstance()->AddNewFoundData(md);

          // notify this change to main frame window
          CMPlayerCApp *pApp = AfxGetMyApp();
          if (pApp)
          {
            CWnd *pWnd = pApp->GetMainWnd();
            if (pWnd)
              pWnd->PostMessage(WM_COMMAND, ID_SPIDER_NEWFILE_FOUND);
          }
        }

        ++itSubCur;

        // sleep for a moment
        ::Sleep(50);
      }
    }

    // sleep for a moment
    ::Sleep(50);

    ++itParentCur;
  }
}