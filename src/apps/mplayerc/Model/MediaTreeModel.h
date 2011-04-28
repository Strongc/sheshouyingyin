#pragma once

#include "MediaComm.h"
#include "MediaModel.h"

namespace media_tree {

class model
{
public:
  typedef MediaTreeFolders::iterator TreeIterator;
  typedef MediaTreeFiles::iterator FileIterator;

public:
  FileIterator findFile(const std::wstring &sPath, const std::wstring &sFilename);
  TreeIterator findFolder(const std::wstring &sPath, bool bCreateIfNotExist = false);
  MediaTreeFolders& mediaTree();

public:
  void initMerit(const std::wstring &sFolder, int nMerit);
  void initHide(const std::wstring &sFolder, const std::wstring &sFilename, bool bHide);
  void addFolder(const std::wstring &sFolder, bool bIncreaseMerit = false);
  void addFile(const std::wstring &sFolder, const std::wstring &sFilename);
  void save2DB();
  void splitPath(const std::wstring &sPath, std::stack<std::wstring> &skResult);

private:
  static MediaTreeFolders m_lsFolderTree;
  MediaModel       m_model;
};

} // namespace