#include "stdafx.h"
#include "MCDBSource.h"

MCDBSource::MCDBSource(void):
  m_readnums(0),
  m_fixnums(0),
  m_pos(0),
  m_dbcount(80),
  m_frontcount(0),
  m_dbprereadpoint(25),
  m_isdbrun(FALSE),
  m_direction(FALSE),
  m_dbstatus(MCDB_UNKNOW),
  m_readerstatus(MCDB_TOSTART),
  m_cleanbuffer(FALSE)
{
  m_buffer.clear();
  m_frontbuff.clear();
  m_tmpdatas.clear();

  // load data for first time
  _Start();
}

MCDBSource::~MCDBSource(void)
{

}

BOOL MCDBSource::IsMoreData()
{
  return (m_sp == m_frontbuff.begin() && m_frontcount <= m_readnums) ? FALSE : TRUE;
}

void MCDBSource::SetReadNums(UINT nums)
{
  m_readnums = nums;
}

BOOL MCDBSource::PreLoad(UINT nums)
{
  if (m_dbstatus == MCDB_TOEND)
    return FALSE;

  SetReadNums(nums);

  m_frontbuff.swap(m_tmpdatas);
  m_frontcount = m_frontbuff.size();

  if (nums > m_frontcount)
    nums = m_frontcount;

  m_sp = m_frontbuff.begin();
  m_ep = m_sp + nums;
  m_pos += nums;

  return TRUE;
}

void MCDBSource::AdjustRange(UINT nums)
{
  UINT x = 0;
  UINT len = 0;

  if (!nums)
    return;
  else if (nums < m_readnums)
  {
    log.Format(L"[AdjustRange_0_0] m_pos:%d\n", m_pos);MCDEBUG(log);
    int len = m_ep - m_sp;
    x = m_frontbuff.end() - m_sp;
    if (nums > x)
      nums = x;
    m_ep = m_sp + nums;
    m_pos -= (len - nums);
    log.Format(L"[AdjustRange_0_1] m_pos:%d\n", m_pos);MCDEBUG(log);
  }
  else if (nums > m_readnums)
  {
    x = m_frontbuff.end() - m_sp;
    if (nums > x)
      nums = x;
    log.Format(L"[AdjustRange_1_0] m_pos:%d\n", m_pos);MCDEBUG(log);
    int len = m_ep - m_sp;
    m_ep = m_sp + nums;
    m_pos += (m_ep - m_sp) - len;
    log.Format(L"[AdjustRange_1_1] m_pos:%d\n", m_pos);MCDEBUG(log);
  }
}

int MCDBSource::RemoveRowDatas(BOOL direction, UINT movenums)
{
  if (direction)
  {
    m_ep -= movenums - m_fixnums;
    m_pos -= movenums - m_fixnums;
    m_fixnums = 0;
  }
  else
  {
    m_sp += movenums;
  }
  return 0;
}

int MCDBSource::LoadRowDatas(BOOL direction, UINT movenums)
{
  UINT x = 0;
  BOOL preread = TRUE;
  
  if (direction != m_direction)
  {
    m_cleanbuffer = TRUE;
    _Start();
  }

  m_direction = direction;
  m_readerstatus = MCDB_MORE;

  if (direction)
  {
    x = m_sp - m_frontbuff.begin();

    // read ahead
    if (m_dbstatus != MCDB_TOSTART)
      PreDataFromDB(x);

    if (!m_tmpdatas.empty() && !m_cleanbuffer)
    {
      m_cleanbuffer = TRUE;
      int len = m_ep - m_sp;
      int toend = len>>1;
      int toep = (m_frontbuff.end()-toend) - m_ep;
      if (toep < 0)
      {
        log.Format(L"[!!!!!!] toep:%d, toend:%d \n", toep, toend);MCDEBUG(log);
        m_readerstatus = MCDB_TOSTART;
        return m_readerstatus;
      }
      else
      {
        log.Format(L"[exchanges] toep:%d, len:%d, toend:%d \n", toep, len, toend);
        MCDEBUG(log);
        log.Format(L"[exchanges__0] start: %s\n end: %s, begin: %s\n\n",
          (*m_sp)->m_mediadata.filename.c_str(),
          (*m_ep)->m_mediadata.filename.c_str(),
          (m_frontbuff.front())->m_mediadata.filename.c_str());
        MCDEBUG(log);
        m_tmpdatas.insert(m_tmpdatas.end(), m_frontbuff.begin(), m_frontbuff.end()-toend);
        m_frontbuff.swap(m_tmpdatas);
        m_ep = m_frontbuff.end() - toep;
        m_sp = m_ep - len;
        log.Format(L"[exchanges__1] start: %s\n end: %s, begin: %s\n\n",
          (*m_sp)->m_mediadata.filename.c_str(),
          (*m_ep)->m_mediadata.filename.c_str(),
          (m_frontbuff.front())->m_mediadata.filename.c_str());
        MCDEBUG(log);
        _Start();
      }
    }

    if (m_dbstatus == MCDB_TOSTART)
    {
      log.Format(L"[MCDB_TOSTART]: x:%d, n: %d, s:%d\n", x, movenums, m_dbstatus);MCDEBUG(log);
      if (x == 0)
      {
        m_readerstatus = MCDB_TOSTART;
        return m_readerstatus;
      }
      else if (x < movenums)
      {
        m_fixnums = movenums - x;
        movenums = x;
      }
    }

    m_sp -= movenums;

    log.Format(L"[move up dbpos to]: pos:%d, status: %d, x: %d, n: %d\n", m_pos, m_dbstatus, x, movenums);MCDEBUG(log);

  } // move up
  else
  {
    x = m_frontbuff.end() - m_ep;

    // read ahead
    if (m_dbstatus != MCDB_TOEND)
      PreDataFromDB(x);

    if (m_dbstatus == MCDB_TOEND)
    {
      log.Format(L"[MCDB_TOEND]: x:%d, n: %d, s:%d\n", x, movenums, m_dbstatus);MCDEBUG(log);
      if (x == 0)
      {
        m_readerstatus = MCDB_TOEND;
        return m_readerstatus;
      }
      else if (x < movenums)
      {
        m_fixnums = movenums - x;
        movenums = x;
      }
    }
    
    else if (!m_tmpdatas.empty() && !m_cleanbuffer)
    {
      m_cleanbuffer = TRUE;
      int len = m_ep - m_sp;
      int tostart = len>>1;
      int tosp = m_sp - (m_frontbuff.begin()+tostart);
      if (tosp < 0)
      {
        log.Format(L"[!!!!!!] tosp:%d, tostart:%d \n", tosp, tostart);
        MCDEBUG(log);
        m_readerstatus = MCDB_TOEND;
        return m_readerstatus;
      }
      else
      {
        log.Format(L"[exchanges] tosp:%d, len:%d, tostart:%d \n", tosp, len, tostart);
        MCDEBUG(log);
        log.Format(L"[exchanges__0] start: %s\n end: %s\n\n",
          (*m_sp)->m_mediadata.filename.c_str(),
          (*m_ep)->m_mediadata.filename.c_str());
        MCDEBUG(log);
        m_tmpdatas.insert(m_tmpdatas.begin(), m_frontbuff.begin() + tostart, m_frontbuff.end());
        m_frontbuff.swap(m_tmpdatas);
        m_sp = m_frontbuff.begin() + tosp;
        m_ep = m_sp + len;
        log.Format(L"[exchanges__1] start: %s\n end: %s\n\n",
          (*m_sp)->m_mediadata.filename.c_str(),
          (*m_ep)->m_mediadata.filename.c_str());
        MCDEBUG(log);
        _Start();
      }
    }

    m_ep += movenums;
    m_pos += movenums;

    log.Format(L"[move down dbpos to]: pos:%d, status: %d, x: %d, n: %d\n", m_pos, m_dbstatus, x, movenums);MCDEBUG(log);
  } // move down

  return m_readerstatus;
}

void MCDBSource::PreDataFromDB(UINT remainnum)
{ 
  if (!m_isdbrun && remainnum <= m_dbprereadpoint && m_tmpdatas.empty())
  {
    log.Format(L"[PreDataFromDB]: r:%d  <= %d\n", remainnum, m_dbprereadpoint);
    MCDEBUG(log);
    _Start();
  }
}

void MCDBSource::MediaDataToBlocks()
{
  if (m_buffer.empty())
    return;

  MediaDatas::iterator it = m_buffer.begin();
  for (; it != m_buffer.end(); it++)
  {
    BlockUnit* one = new BlockUnit;
    one->DefLayer();
    one->m_mediadata = (*it);
    m_tmpdatas.push_back(one);
  }

  m_buffer.clear();
}

void MCDBSource::_Thread()
{
  if (m_cleanbuffer)
  {
    m_tmpdatas.clear();
    m_cleanbuffer = FALSE;
    return;
  }

  m_isdbrun = TRUE;

  MediaFindCondition sqlwhere;
  sqlwhere.filename = L"";
  sqlwhere.uniqueid = 0;

  // move up
  if (m_direction)
  {
    if (m_dbstatus == MCDB_TOSTART)
    {
      m_isdbrun = FALSE;
      return;
    }

    m_dbstatus = MCDB_MORE;

    int count = m_pos - (m_ep - m_frontbuff.begin());
    int pos = count - m_dbcount;
    if (pos <= 0)
    {
      pos = 0;
      m_dbstatus = MCDB_TOSTART;
      if (!count)
      {
        m_isdbrun = FALSE;
        return;
      }
    }

    log.Format(L"[_Thread] pos:%d, dbcount:%d, s:%d, mpos:%d,count:%d \n", pos, m_dbcount,m_dbstatus,m_pos,count);MCDEBUG(log);
    m_db.Find(m_buffer, sqlwhere, pos, count);
  }
  // move down
  else
  {
    if (m_dbstatus == MCDB_TOEND)
    {
      m_isdbrun = FALSE;
      return;
    }

    m_dbstatus = MCDB_MORE;

    int pos = m_pos;
    if (m_pos <= 0)
    {
      m_dbstatus = MCDB_TOSTART;
      pos = m_pos = 0;
    }
    else
      pos += m_frontbuff.end() - m_ep;

    log.Format(L"[_Thread] dbpos:%d, dbcount:%d, s:%d\n", pos, m_dbcount,m_dbstatus);MCDEBUG(log);
    m_db.Find(m_buffer, sqlwhere, pos, m_dbcount);
    if (m_buffer.empty())
    {
      m_isdbrun = FALSE;
      m_dbstatus = MCDB_TOEND;
      return;
    }
  }

  MediaDataToBlocks();
  m_isdbrun = FALSE;

  log.Format(L"[_Thread] start: %s\n end: %s\n\n",
    (m_tmpdatas.front())->m_mediadata.filename.c_str(),
    (m_tmpdatas.back())->m_mediadata.filename.c_str());
  MCDEBUG(log);
}