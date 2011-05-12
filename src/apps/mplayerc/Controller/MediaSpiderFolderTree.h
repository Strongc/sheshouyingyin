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
  media_tree::model m_treeModel;

  time_t m_tSleep;  // loop time, unit is second
  int m_nSpideInterval;  // record spide interval
  int m_nThreadStartInterval;  //
};