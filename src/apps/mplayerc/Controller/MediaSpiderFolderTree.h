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
  BOOL Search(const std::wstring &sFolder);

private:
  media_tree::model m_treeModel;

  int m_tSleep;  // loop time, unit is second
  int m_nSpideInterval;  // record spide interval
  int m_nThreadStartInterval;  //
};