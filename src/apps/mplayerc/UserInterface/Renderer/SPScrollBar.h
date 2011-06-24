#pragma once

#include "UILayer.h"

class SPScrollBar: public UILayer
{
public:
  SPScrollBar(std::wstring respath, BOOL display = TRUE, UINT nums = 1);
  ~SPScrollBar();

  void SetClientRect(RECT& rc);

  int GetSBarOffset();
  int GetBarWidth();
  BOOL GetDirection();
  BOOL IsDragBar();

  BOOL ActMouseMove(const POINT& pt);
  BOOL ActMouseLBDown(const POINT& pt);
  BOOL ActMouseLBUp(const POINT& pt);
  BOOL ActRButtonUp(const POINT& pt);
  BOOL ActLButtonDblClick(const POINT& pt);

private:
  int m_fixx;
  int m_fixy;

  int m_offset;

  RECT m_rcparent;
  RECT m_rcsbar;
  BOOL m_startdrag;
  int m_lasty;
  BOOL m_direction;
};

