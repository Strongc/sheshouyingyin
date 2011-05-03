#pragma once

#include "MediaSpiderAbstract.h"
#include "..\Model\MediaTreeModel.h"

class MediaSpiderFolderTree : public MediaSpiderAbstract<MediaSpiderFolderTree>
{
public:
  MediaSpiderFolderTree();
  ~MediaSpiderFolderTree();

public:
  void _Thread();
  void Search(const std::wstring &sFolder);

private:
  std::wstring m_sLastSearchPath;
  media_tree::model m_treeModel;
};