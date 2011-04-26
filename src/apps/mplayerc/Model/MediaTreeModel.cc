#include "stdafx.h"
#include "MediaTreeModel.h"
#include <stack>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>
#include <boost/foreach.hpp>
//#include "SVPToolBox.h"
//#include "../Utils/SPlayerGUID.h"

////////////////////////////////////////////////////////////////////////////////
// normal part
MediaTreeFolders media_tree::model::m_lsFolderTree;

////////////////////////////////////////////////////////////////////////////////
// properties
media_tree::model::TreeIterator media_tree::model::findFolder(const std::wstring &sPath, bool bCreateIfNotExist /* = false */)
{
  using namespace boost::lambda;
  using std::wstring;
  using std::vector;
  using std::stack;

  wstring sPreferredPath = makePathPreferred(sPath);
  stack<wstring> skPathParts;
  splitPath(sPreferredPath, skPathParts);

  MediaTreeFolders::tree_type *pCurTree = &m_lsFolderTree;
  TreeIterator itCurTree = pCurTree->begin();
  TreeIterator itTreeEnd;
  TreeIterator itResult;
  while (!skPathParts.empty())
  {
    wstring sCurPart = skPathParts.top();
    TreeIterator itFind = std::find_if(pCurTree->begin(), pCurTree->end(),
                          bind(&media_tree::folder::sFolderPath, _1) == sCurPart);
    if (itFind == pCurTree->end())
    {
      // insert new node if allowed, else return an invalid iterator
      if (bCreateIfNotExist)
      {
        media_tree::folder fd;
        fd.sFolderPath = sCurPart;
        fd.tFolderCreateTime = ::time(0);
        itFind = pCurTree->insert(fd);

        pCurTree = itFind.node();
        itCurTree = pCurTree->begin();
        itResult = itFind;
      }
      else
      {
        itResult = itTreeEnd;
        break;
      }
    }
    else
    {
      pCurTree = itFind.node();
      itCurTree = pCurTree->begin();
      itResult = itFind;
    }

    skPathParts.pop();
  }

  return itResult;
}

MediaTreeFolders& media_tree::model::mediaTree()
{
  return m_lsFolderTree;
}

////////////////////////////////////////////////////////////////////////////////
// add media info to the tree and save the info to the database
void media_tree::model::addFolder(const std::wstring &sFolder, bool bIncreaseMerit /* = false */)
{
  TreeIterator itFolder = findFolder(sFolder, true);
  TreeIterator itEnd;
  if (itFolder != itEnd)
  {
    // modify something about this folder
    if (bIncreaseMerit)
      ++(itFolder->nMerit);
  }
  else
  {
    // should never go here
  }
}

void media_tree::model::addFile(const std::wstring &sFolder, const std::wstring &sFilename)
{
  using namespace boost::lambda;

  TreeIterator itFolder = findFolder(sFolder, true);
  TreeIterator itEnd;
  if (itFolder != itEnd)
  {
    // modify something about this folder's file list
    // insert unique file
    MediaTreeFiles &files = itFolder->lsFiles;
    MediaTreeFiles::iterator itFiles = std::find_if(files.begin(), files.end(),
                                       bind(&media_tree::file::sFilename, _1) == sFilename);
    if (itFiles == files.end())
    {
      media_tree::file fe;
      fe.sFilename = sFilename;
      //      CSVPToolBox toolbox;
      //      std::wstring sThumbnailPath;
      //      toolbox.GetAppDataPath(sThumbnailPath);
      //      sThumbnailPath += L"\\" + SPlayerGUID::RandMakeGUID() + L".jpg";
      //fe.sFileThumbnail = sThumbnailPath;
      //fe.sFileHash = ;
      //fe.sFileUID = ;
      fe.tFileCreateTime = ::time(0);
      files.push_back(fe);
    }
  }
  else
  {
    // should never go here
  }
}

void media_tree::model::save2DB()
{
  MediaTreeFolders::tree_type::pre_order_iterator it = m_lsFolderTree.pre_order_begin();
  while (it != m_lsFolderTree.pre_order_end())
  {
    // store path info
    std::wstring sFolderPath = fullFolderPath(it.node());

    MediaPath mp;
    mp.path = sFolderPath;
    mp.merit = it->nMerit;
    m_model.Add(mp);

    // store file info
    MediaTreeFiles &files = it->lsFiles;
    MediaTreeFiles::iterator itFile = files.begin();
    while (itFile != files.end())
    {
      MediaData md;
      md.path = sFolderPath;
      md.filename = itFile->sFilename;
      md.thumbnailpath = itFile->sFileThumbnail;
      // md.videotime = ;

      m_model.Add(md);

      ++itFile;
    }

    ++it;
  }
}

void media_tree::model::splitPath(const std::wstring &sPath, std::stack<std::wstring> &skResult)
{
  using namespace boost;
  using namespace boost::regex_constants;
  using std::wstring;
  using std::vector;
  using std::stack;

  // split the path into various parts
  vector<wstring> vtResult;
  std::wstring sTemp(sPath);

  wsmatch what;
  wregex rx(L"(^[^\\\\]+)\\\\?|(^\\\\\\\\[^\\\\]+)\\\\?");  // match both normal path and UNC path
  wstring::const_iterator start = sTemp.begin();
  wstring::const_iterator end   = sTemp.end();
  while (regex_search(start, end, what, rx, match_default))
  {
    if (!what[1].str().empty())
      vtResult.push_back(what[1]);  // Normal path's each part
    else
      vtResult.push_back(what[2]);  // UNC prefix

    start = what[0].second;
  }

  std::for_each(vtResult.rbegin(), vtResult.rend(), lambda::bind(&std::stack<std::wstring>::push, &skResult, lambda::_1));
}

void media_tree::model::initMerit(const std::wstring &sFolder, int nMerit)
{
  TreeIterator itFolder = findFolder(sFolder, true);
  TreeIterator itEnd;
  if (itFolder != itEnd)
  {
    itFolder->nMerit = nMerit;
  }
}
