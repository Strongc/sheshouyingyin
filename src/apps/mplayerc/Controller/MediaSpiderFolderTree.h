#pragma once

#include "MediaSpiderImpl.h"
#include "..\Model\MediaTreeModel.h"

class MediaSpiderFolderTree : public MediaSpiderImpl<MediaSpiderFolderTree>
{
public:
  MediaSpiderFolderTree();
  ~MediaSpiderFolderTree();

public:
  void _Thread();
  BOOL Search(const std::wstring &sFolder);

private:
  media_tree::model m_treeModel;

  int m_tSleep;  // loop time, unit is second
  int m_spiderinterval;
};