#include "stdafx.h"
#include "SPScrollBar.h"
#include "../../Controller/MediaCenterController.h"

SPScrollBar::SPScrollBar(std::wstring respath, BOOL display /* = TRUE */, UINT nums):
UILayer(respath, display, nums),
m_startdrag(FALSE),
m_lasty(0),
m_offset(0)
{
  int w, h;
  GetTextureWH(w, h);
  m_fixx = w + 5;
  m_fixy = h;
}

SPScrollBar::~SPScrollBar()
{

}

int SPScrollBar::GetBarWidth()
{
  return GetDisplay()? m_fixx: 0;
}

BOOL SPScrollBar::IsDragBar()
{
  return m_startdrag;
}

void SPScrollBar::SetClientRect(RECT& rc)
{
  POINT pt;

  int offset = m_direction?-1:1;
  offset *= m_offset;

  pt.x = rc.right - m_fixx;
  pt.y = (rc.bottom - m_fixy) / 2 + offset;

  if (pt.y < rc.top)
    pt.y = rc.top;
  else if (pt.y + m_fixy > rc.bottom)
    pt.y = rc.bottom - m_fixy;

  SetTexturePos(pt);
}

BOOL SPScrollBar::GetDirection()
{
  return m_direction;
}

int SPScrollBar::GetSBarOffset()
{
  return m_offset;
}

BOOL SPScrollBar::ActMouseMove(const POINT& pt)
{
  BOOL ret = FALSE;

  GetTextureRect(m_rcsbar);
  if (::PtInRect(&m_rcsbar, pt))
  {
    if (GetState() != 1)
    {
      SetState(1);
      MediaCenterController::GetInstance()->Render();
    }

    if (m_startdrag)
    {
      m_direction = (m_lasty > pt.y) ? TRUE/*up*/ : FALSE/*down*/;
      m_offset = abs(pt.y - m_lasty);
      ret = TRUE;
    }

    // set hand cursor
    MediaCenterController::GetInstance()->SetCursor(IDC_HAND);
  }
  else
  {
    if (GetState() != 0)
    {
      SetState(0);
      MediaCenterController::GetInstance()->Render();
    }

    // set arrow cursor
    MediaCenterController::GetInstance()->SetCursor(IDC_ARROW);
  }
  return ret;
}

BOOL SPScrollBar::ActMouseLBDown(const POINT& pt)
{
  BOOL ret = FALSE;

  if (::PtInRect(&m_rcsbar, pt))
  {
    m_lasty = pt.y;
    ret = TRUE;
    m_startdrag = TRUE;
    MediaCenterController::GetInstance()->Update();
  }

  return ret;
}

BOOL SPScrollBar::ActMouseLBUp(const POINT& pt)
{
  m_lasty = 0;
  m_offset = 0;

  if (m_startdrag)
  {
    MediaCenterController::GetInstance()->StopUpdate();
    MediaCenterController::GetInstance()->Render();
  }
  m_startdrag = FALSE;

  return FALSE;
}
