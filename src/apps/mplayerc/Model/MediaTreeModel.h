#pragma once

#include "MediaComm.h"
#include "MediaModel.h"
#include "CriticalSection.h"

namespace media_tree {

class model
{
public:
  typedef MediaTreeFolders::iterator TreeIterator;
  typedef MediaTreeFiles::iterator FileIterator;

public:
  struct tagFileInfo
  {
    tagFileInfo() : pFileList(0) {}
    bool isValid() { return pFileList && (itFile != pFileList->end()); }
    FileIterator itFile;
    MediaTreeFiles *pFileList;
  };

public:
  tagFileInfo findFile(const std::wstring &sPath, const std::wstring &sFilename);
  TreeIterator findFolder(const std::wstring &sPath, bool bCreateIfNotExist = false);
  MediaTreeFolders& mediaTree();

public:
  void initMerit(const std::wstring &sFolder, int nMerit);
  void initHide(const std::wstring &sFolder, const std::wstring &sFilename, bool bHide);
  void addFolder(const std::wstring &sFolder, bool bIncreaseMerit = false);
  void addFile(const MediaData &md);
  void save2DB();
  void delTree();
  void splitPath(const std::wstring &sPath, std::stack<std::wstring> &skResult);

private:
  static MediaTreeFolders m_lsFolderTree;
  static CriticalSection m_cs;
  MediaModel       m_model;
};

} // namespace