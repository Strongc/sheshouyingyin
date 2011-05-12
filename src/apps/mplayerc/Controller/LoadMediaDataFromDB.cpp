#include "stdafx.h"
#include "LoadMediaDataFromDB.h"

LoadMediaDataFromDB::LoadMediaDataFromDB(void)
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
  while (m_executetime--)
  {
    LoadMediadatasFromDB();
    AddDataToList();
  }
}

void LoadMediaDataFromDB::LoadMediadatasFromDB()
{
  m_model.Find(m_mediadatas, m_windowcapacity, m_amount, m_remain, m_direction);
}

void LoadMediaDataFromDB::AddDataToList()
{
  if (!m_listbuff->empty())
    ClearList();

  MediaDatas::iterator it = m_mediadatas.begin();
  
  while (it != m_mediadatas.end())
  {
    BlockUnit* one = new BlockUnit;
    one->m_mediadata = *it;
    one->DefLayer();
    m_listbuff->push_back(one);
    ++it;
  }

  m_mediadatas.clear();
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