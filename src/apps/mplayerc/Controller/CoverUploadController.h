#pragma once

#include <threadhelper.h>
#include "NetworkControlerImpl.h"
#include "../Model/MediaComm.h"
#include "../UserInterface/Renderer/BlockList.h"

class CoverUploadController:
  public ThreadHelperImpl<CoverUploadController>,
  public NetworkControlerImpl
{
public:
  CoverUploadController(void);
  ~CoverUploadController(void);

  void SetFrame(HWND hwnd);

  //Upload cover
  void _Thread();

  void SetCover(BlockUnit* unit, std::wstring orgpath);
  void UploadCover(BlockUnit* unit, std::wstring url);

  std::wstring GetSystemTimeString(std::string szFileHash);

private:
  std::list<BlockUnit*> m_list;
  HWND m_hwnd;
};
