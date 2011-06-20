#include "stdafx.h"
#include "MCDBSource.h"
#include "../Controller/MediaCenterController.h"

MCDBSource::MCDBSource(void):
  m_readnums(0),
  m_pos(0),
  m_direction(FALSE),
  m_stopdb(TRUE),
  m_readerstatus(MCDB_TOSTART)
{
  m_buffer.clear();
  m_frontbuff.clear();

  m_sp = m_ep = m_frontbuff.begin();
  m_total = m_db.GetCount();
}

MCDBSource::~MCDBSource(void)
{

}

BOOL MCDBSource::IsMoreData()
{
  return (m_total > m_readnums) ? TRUE : FALSE;
}

void MCDBSource::SetReadNums(UINT nums)
{
  m_readnums = nums;
}

void MCDBSource::CleanData()
{
  _Stop();

  BUPOINTER it = m_sp;
  for (; it != m_ep; ++it)
    delete *it;
  
  m_frontbuff.clear();
  m_sp = m_ep = m_frontbuff.begin();
}

BOOL MCDBSource::PreLoad(UINT nums)
{
  if (!m_total)
    return FALSE;

  _Start(THREAD_PRIORITY_LOWEST);

  SetReadNums(nums);

  for (UINT i = 0; i < nums; ++i)
  {
    BlockUnit* bu = new BlockUnit;
    bu->DefLayer();
    m_frontbuff.push_back(bu);
  }

  m_sp = m_frontbuff.begin();
  m_ep = m_frontbuff.end();
  m_pos = nums;

  m_stopdb = FALSE;

  return TRUE;
}

void MCDBSource::AdjustRange(UINT nums, UINT columns)
{
  if (nums == m_readnums)
    return;

  int len = nums - m_readnums;
  if (len > 0)
  {
    for (int i = 0; i < len; ++i)
    {
      BlockUnit* bu = new BlockUnit;
      bu->DefLayer();
      m_frontbuff.push_back(bu);
    }
    m_pos += len;
    if (m_pos > m_total)
      m_pos = m_total;
  }
  else
  {
    len = len * -1;
    BUPOINTER it = m_ep - len;
    for (; it != m_ep; ++it)
      delete *it;
    m_frontbuff.erase(m_ep - len, m_ep);
    m_pos -= len;
    m_readerstatus = MCDB_MORE;
    if (m_pos < 0)
      m_pos = 0;
  }
  m_sp = m_frontbuff.begin();
  m_ep = m_frontbuff.end();

  m_stopdb = FALSE;
}

int MCDBSource::LoadRowDatas(BOOL direction, UINT columns)
{
  UINT len = 0;
  UINT nums = columns;

  m_direction = direction;
  m_readerstatus = MCDB_MORE;

  if (direction)  // move up
  {
    if (m_pos <= m_readnums)
    {
      m_readerstatus = MCDB_TOSTART;
      return m_readerstatus;
    }

    m_stopdb = FALSE;
    
    m_frontbuff.insert(m_frontbuff.begin(), m_ep-columns, m_ep);
    m_frontbuff.erase(m_frontbuff.end()-columns, m_frontbuff.end());
    m_sp = m_frontbuff.begin();
    m_ep = m_frontbuff.end();

    BUPOINTER it = m_sp;
    MediaData md;
    for (int i = 0; it != m_sp+columns; ++it, ++i)
    {
      (*it)->m_mediadata = md;
      (*it)->CleanCover();
    }

    m_pos -= nums;
  }
  else  // move down
  {
    len = m_total - m_pos;
    if (len == 0) 
    {
      m_readerstatus = MCDB_TOEND;
      return m_readerstatus;
    }
    else if (len < columns)
      nums = len;

    m_stopdb = FALSE;
    
    m_frontbuff.insert(m_frontbuff.end(), m_sp, m_sp + columns);
    m_frontbuff.erase(m_frontbuff.begin(), m_frontbuff.begin() + columns);

    m_sp = m_frontbuff.begin();
    m_ep = m_frontbuff.end();
    
    BUPOINTER it = m_ep - columns;
    MediaData md;
    for (int i=0;it != m_ep; ++it, ++i)
    {
      (*it)->m_mediadata = md;
      (*it)->CleanCover();
    }

    m_pos += nums;
  }

  return m_readerstatus;
}

void MCDBSource::_Thread()
{
  MediaFindCondition sqlwhere;
  sqlwhere.filename = L"";
  sqlwhere.uniqueid = 0;

  while(!_Exit_state(2000))
  {
    if (m_stopdb)
      continue;

    m_stopdb = TRUE;

    int start = m_pos - m_readnums;

    m_db.Find(m_buffer, sqlwhere, (start < 0 ? 0 : start), m_readnums);
    MediaDatas::iterator val = m_buffer.begin();
    
    for (BUPOINTER it=m_sp;it != m_ep; ++it,++val)
      (*it)->m_mediadata = *val;

    MediaCenterController::GetInstance()->Render();
    m_buffer.clear();
  }
}