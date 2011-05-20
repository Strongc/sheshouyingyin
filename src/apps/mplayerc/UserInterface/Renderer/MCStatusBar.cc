#include "stdafx.h"
#include "MCStatusBar.h"
#include <atlimage.h>

////////////////////////////////////////////////////////////////////////////////
// Normal part
MCStatusBar::MCStatusBar()
: 
  m_rc(0, 0, 0, 0)
, m_crBKColor(0)    // default color is black
{
  // Start GDI+ and load the background image
  CImage igForInit;    // Used only for start up GDI+
  igForInit.Load(L"");
}

MCStatusBar::~MCStatusBar()
{
}

////////////////////////////////////////////////////////////////////////////////
// Properties
// void MCStatusBar::SetFrame(HWND hwnd)
// {
//   m_hwnd = hwnd;
//   //Update();
// }

void MCStatusBar::SetRect(const CRect &rc)
{
  m_rc = rc;
  //Update();
}

void MCStatusBar::SetText(const std::wstring &str)
{
  m_str = str;
  //Update();
}

void MCStatusBar::SetVisible(bool bVisible)
{
  m_bVisible = bVisible;
  //Update();
}

void MCStatusBar::SetBKColor(COLORREF cr)
{
  m_crBKColor = cr;
  //Update();
}

CRect MCStatusBar::GetRect() const
{
  return m_rc;
}

bool MCStatusBar::GetVisible() const
{
  return m_bVisible;
}

COLORREF MCStatusBar::GetBKColor() const
{
  return m_crBKColor;
}

////////////////////////////////////////////////////////////////////////////////
// Operations
void MCStatusBar::Update(WTL::CDC& dc)
{
  OnPaint(dc);
}

////////////////////////////////////////////////////////////////////////////////
// Event handler
void MCStatusBar::OnPaint(WTL::CDC& dc)
{
  using Gdiplus::Graphics;
  using Gdiplus::Bitmap;
  using Gdiplus::SolidBrush;
  using Gdiplus::Pen;
  using Gdiplus::Color;
  using Gdiplus::Font;
  using Gdiplus::PointF;
  using Gdiplus::StringFormat;
  using Gdiplus::StringAlignment;
  using Gdiplus::StringFormatFlags;

  // Check the internal variable
  if (m_rc.IsRectEmpty() || !m_bVisible)
    return;

  // Determine the update rect
  CRect rcParentUpdateArea(m_rc);
  //::GetUpdateRect(m_hwnd, &rcParentUpdateArea, FALSE);

  CRect rcThisUpdateArea(rcParentUpdateArea);
  //BOOL bRet = rcThisUpdateArea.IntersectRect(&rcParentUpdateArea, &m_rc);
  //if (!bRet)
  //  return;  // this step means no area need to be updated

  // Using double buffer and GDI+
  Bitmap bmMem(m_rc.Width(), m_rc.Height());
  Graphics gpMem(&bmMem);

  // Background
  SolidBrush brBK(Color(GetRValue(m_crBKColor), GetGValue(m_crBKColor), GetBValue(m_crBKColor)));
  gpMem.FillRectangle(&brBK, 0, 0, m_rc.Width(), m_rc.Height());

  // Text
  Font fnText(L"Tahoma", 9);
  SolidBrush brText(Color(125, 125, 125));
  StringFormat strfm(StringFormatFlags::StringFormatFlagsNoWrap);
  strfm.SetAlignment(StringAlignment::StringAlignmentNear);
  strfm.SetLineAlignment(StringAlignment::StringAlignmentCenter);
  Gdiplus::RectF rc(0, 0, m_rc.Width(), m_rc.Height());
  gpMem.DrawString(m_str.c_str(), -1, &fnText, rc, &strfm, &brText);
  //gpMem.DrawString(m_str.c_str(), -1, &fnText, PointF(0, 0), &brText);

  // Bitblt the mem dc to real dc
  CPoint piStart;
  piStart.x = rcThisUpdateArea.left - m_rc.left;
  piStart.y = rcThisUpdateArea.top - m_rc.top;
  Graphics gpReal(dc.m_hDC);
  gpReal.DrawImage(&bmMem, rcThisUpdateArea.left, rcThisUpdateArea.top, piStart.x, piStart.y,
                   rcThisUpdateArea.Width(), rcThisUpdateArea.Height(), Gdiplus::UnitPixel);

  // Validate the window
  //::ValidateRect(m_hwnd, &rcThisUpdateArea);
}
