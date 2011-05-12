#pragma once

#include <threadhelper.h>
#include "../Model/MediaModel.h"
#include "../Model/MediaComm.h"
#include "..\UserInterface\Renderer\BlockList.h"

class LoadMediaDataFromDB:
  public ThreadHelperImpl<LoadMediaDataFromDB>
{
public:
  LoadMediaDataFromDB(void);
  ~LoadMediaDataFromDB(void);

  void SetList(std::list<BlockUnit*>* list);
  void SetDirection(BOOL bl);
  void SetWindownCapacity(int count);
  void SetListRemainItem(int remain);
  void SetAmount(int amount);
  void SetExecuteTime(int tm);

  void _Thread();
  void LoadMediadatasFromDB();
  void AddDataToList();
  void ClearList();

private:
  MediaModel m_model;
  std::list<BlockUnit*>* m_listbuff;
  int m_direction;
  MediaDatas m_mediadatas;
  int m_windowcapacity;
  int m_amount;
  int m_remain;
  int m_executetime;
  HWND m_hwnd;
};
