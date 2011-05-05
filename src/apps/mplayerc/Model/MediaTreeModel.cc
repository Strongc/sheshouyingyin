#include "stdafx.h"
#include "MediaTreeModel.h"
#include <stack>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>
#include <boost/foreach.hpp>
#include "MediaSQLite.h"

////////////////////////////////////////////////////////////////////////////////
// normal part
MediaTreeFolders media_tree::model::m_lsFolderTree;

////////////////////////////////////////////////////////////////////////////////
// properties
media_tree::model::TreeIterator media_tree::model::findFolder(const std::wstring &sPath, bool bCreateIfNotExist /* = false */)
{
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
    TreeIterator itFind = pCurTree->begin();
    while (itFind != pCurTree->end())
    {
      if (itFind->folder_data.path == sCurPart)
        break;

      ++itFind;
    }

    if (itFind == pCurTree->end())
    {
      // insert new node if allowed, else return an invalid iterator
      if (bCreateIfNotExist)
      {
        media_tree::folder fd;
        fd.folder_data.path = sCurPart;
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

media_tree::model::tagFileInfo media_tree::model::findFile(const std::wstring &sPath, const std::wstring &sFilename)
{
  tagFileInfo fileResult;

  TreeIterator itFolder = findFolder(sPath);
  TreeIterator itEnd;
  if (itFolder != itEnd)
  {
    // find this folder
    FileIterator itFile = itFolder->lsFiles.begin();
    while (itFile != itFolder->lsFiles.end())
    {
      if (itFile->file_data.filename == sFilename)
      {
        fileResult.itFile = itFile;
        fileResult.pFileList = &(itFolder->lsFiles);
        break;
      }

      ++itFile;
    }
  }

  return fileResult;
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
      ++(itFolder->folder_data.merit);
  }
  else
  {
    // should never go here
  }
}

void media_tree::model::addFile(const MediaData &md)
{
  using namespace boost::lambda;

  TreeIterator itFolder = findFolder(md.path, true);
  TreeIterator itEnd;
  if (itFolder != itEnd)
  {
    // modify something about this folder's file list
    // insert unique file
    MediaTreeFiles &files = itFolder->lsFiles;
    MediaTreeFiles::iterator itFiles = files.begin();
    while (itFiles != files.end())
    {
      if (itFiles->file_data.filename == md.filename)
        break;

      ++itFiles;
    }

    if (itFiles == files.end())
    {
      media_tree::file fe;
      fe.file_data.filename = md.filename;
      fe.file_data.path = fullFolderPath(itFolder.node());
      fe.file_data.thumbnailpath = md.thumbnailpath;
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
  MediaSQLite<>::exec(L"begin transaction");
  MediaTreeFolders::tree_type::pre_order_iterator it = m_lsFolderTree.pre_order_begin();
  while (it != m_lsFolderTree.pre_order_end())
  {
    // store path info
    std::wstring sFolderPath = fullFolderPath(it.node());

    MediaPath mp;
    mp.path = sFolderPath;
    mp.merit = it->folder_data.merit;
    m_model.Add(mp);

    // store file info
    MediaTreeFiles &files = it->lsFiles;
    MediaTreeFiles::iterator itFile = files.begin();
    while (itFile != files.end())
    {
      MediaData md;
      md.path = sFolderPath;
      md.filename = itFile->file_data.filename;
      md.thumbnailpath = itFile->file_data.thumbnailpath;
      md.bHide = itFile->file_data.bHide;
      // md.videotime = ;

      m_model.Add(md);

      ++itFile;
    }

    ++it;
  }
  MediaSQLite<>::exec(L"end transaction");
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
    itFolder->folder_data.merit = nMerit;
}

void media_tree::model::initHide(const std::wstring &sFolder, const std::wstring &sFilename, bool bHide)
{
  tagFileInfo fileInfo = findFile(sFolder, sFilename);
  if (fileInfo.isValid())
    fileInfo.itFile->file_data.bHide = bHide;
}