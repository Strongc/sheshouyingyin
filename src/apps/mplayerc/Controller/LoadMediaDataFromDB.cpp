#include "stdafx.h"
#include "LoadMediaDataFromDB.h"
#include "MediaCenterController.h"

LoadMediaDataFromDB::LoadMediaDataFromDB(void):
m_limitstart(0)
,m_limitend(0)
{
}

LoadMediaDataFromDB::~LoadMediaDataFromDB(void)
{
}

void LoadMediaDataFromDB::SetList(std::list<BlockUnit*>* list)
{
  m_listbuff = list;
}

void LoadMediaDataFromDB::SetDirection(int dirction)
{
  m_direction = dirction;
}

void LoadMediaDataFromDB::SetExecuteTime(int tm)
{
  m_executetime = tm;
}

void LoadMediaDataFromDB::_Thread()
{
  if (!m_listbuff->empty())
    ClearList();

  int executetime = m_executetime;
  while (executetime--)
  {
    LoadMediadatasFromDB();
  }
  if (m_executetime > 1 && m_limitstart == 0)
    m_mediadatas.clear();
  if (m_executetime)
    AddDataToList();
}

void LoadMediaDataFromDB::LoadMediadatasFromDB()
{
  if (CalculateStartAndEnd())
    m_model.Find(m_limitstart, m_amount, m_mediadatas);
}

BOOL LoadMediaDataFromDB::CalculateStartAndEnd()
{
  if (m_direction == 0)
    return FALSE;

  if (m_direction > 0)
  {
    if (m_limitend >= m_model.GetCount())
      return FALSE;
    if (m_limitend != 0)
      m_limitstart = m_limitend - m_remain - m_windowcapacity;
  }

  if (m_direction < 0)
  {
    if (m_limitstart == 0)
      return FALSE;
    m_limitstart = m_limitstart + m_windowcapacity + m_remain - m_amount;


    if (m_limitstart < 0)
    {
      m_amount += m_limitstart;
      m_limitstart = 0;
    }
  }


  if (m_limitstart + m_amount > m_model.GetCount())
    m_amount = m_model.GetCount() - m_limitstart;

  m_limitend = m_limitstart + m_amount;
  
  return TRUE;
}

void LoadMediaDataFromDB::AddDataToList()
{
//   if (!m_listbuff->empty())
//     ClearList();

  MediaDatas::iterator it = m_mediadatas.begin();

  int count = 1;
  BlockListView* list = &MediaCenterController::GetInstance()->GetBlockListView();

  while (it != m_mediadatas.end())
  {
    BlockUnit* one = new BlockUnit;
    one->m_mediadata = *it;

    if (one->m_mediadata.thumbnailpath.empty())
    {
      std::wstring sFilePath = one->m_mediadata.path + one->m_mediadata.filename;
      one->m_mediadata.thumbnailpath = MediaCenterController::GetCoverPath(sFilePath);
    }
    one->DefLayer();
    m_listbuff->push_back(one);

    if (count == m_mediadatas.size() - m_windowcapacity - m_remain + 1)
      list->SetListBuffIterator(--m_listbuff->end());


    ++it;
    ++count;
  }

  m_mediadatas.clear();
  if (!m_listbuff->empty())
  {
    list->SetClearStat();
    list->ResetListLogicalEnd(FALSE);
  }
  else
  {
    list->ResetListLogicalEnd(TRUE);
  }
      
}

void LoadMediaDataFromDB::SetWindownCapacity(int count)
{
  m_windowcapacity = count;
}

void LoadMediaDataFromDB::SetAmount(int amount)
{
  m_amount = amount;
}

void LoadMediaDataFromDB::ClearList()
{
  
  std::list<BlockUnit*>::iterator it = m_listbuff->begin();
  while (it != m_listbuff->end())
  {
    (*it)->DeleteLayer();
    delete *it;
    ++it;
  }

  m_listbuff->clear();
}

void LoadMediaDataFromDB::SetListRemainItem(int remain)
{
  m_remain = remain;
}

std::list<BlockUnit*>* LoadMediaDataFromDB::GetListBuff()
{
  return m_listbuff;
}