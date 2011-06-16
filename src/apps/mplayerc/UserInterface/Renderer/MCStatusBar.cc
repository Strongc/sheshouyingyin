#include "stdafx.h"
#include "MCStatusBar.h"

////////////////////////////////////////////////////////////////////////////////
// Normal part
MCStatusBar::MCStatusBar()
: m_rc(0, 0, 0, 0)
, m_crBKColor(0)    // default color is black
, m_pfnText(0)
, m_pbrText(0)
, m_pstrfm(0)
, m_str(L"射手影音 生活相伴")
, m_bVisible(false)
{
  // Start GDI+ and load the background image
  CImage igForInit;    // Used only for start up GDI+
  igForInit.Load(L"");

  // settings for font
  m_pfnText = new Gdiplus::Font(L"Tahoma", 9);
  m_pbrText = new Gdiplus::SolidBrush(Gdiplus::Color(125, 125, 125));
  m_pstrfm = new Gdiplus::StringFormat(Gdiplus::StringFormatFlags::StringFormatFlagsNoWrap);
  m_pstrfm->SetAlignment(Gdiplus::StringAlignment::StringAlignmentNear);
  m_pstrfm->SetLineAlignment(Gdiplus::StringAlignment::StringAlignmentCenter);
}

MCStatusBar::~MCStatusBar()
{
}

////////////////////////////////////////////////////////////////////////////////
// Properties
void MCStatusBar::SetRect(const CRect &rc)
{
  m_rc = rc;
}

void MCStatusBar::SetText(const std::wstring &str)
{
  if (str.empty())
    m_str = L"射手影音 生活相伴";
  else
    m_str = str;
}

void MCStatusBar::SetVisible(bool bVisible)
{
  m_bVisible = bVisible;
}

void MCStatusBar::SetBKColor(COLORREF cr)
{
  m_crBKColor = cr;
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

std::wstring MCStatusBar::GetText() const
{
  return m_str;
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

  // Check the internal variable
  if (m_rc.IsRectEmpty() || !m_bVisible)
    return;

  // Determine the update rect
  CRect rcParentUpdateArea(m_rc);
  CRect rcThisUpdateArea(rcParentUpdateArea);

  // Using double buffer and GDI+
  Bitmap bmMem(m_rc.Width(), m_rc.Height());
  Graphics gpMem(&bmMem);

  // Background
  SolidBrush brBK(Color(GetRValue(m_crBKColor), GetGValue(m_crBKColor), GetBValue(m_crBKColor)));
  gpMem.FillRectangle(&brBK, 0, 0, m_rc.Width(), m_rc.Height());

  // Text
  Gdiplus::RectF rc(0, 0, m_rc.Width(), m_rc.Height());
  gpMem.DrawString(m_str.c_str(), -1, m_pfnText, rc, m_pstrfm, m_pbrText);

  // Bitblt the mem dc to real dc
  CPoint piStart;
  piStart.x = rcThisUpdateArea.left - m_rc.left;
  piStart.y = rcThisUpdateArea.top - m_rc.top;
  Graphics gpReal(dc.m_hDC);
  gpReal.DrawImage(&bmMem, rcThisUpdateArea.left, rcThisUpdateArea.top, piStart.x, piStart.y,
                   rcThisUpdateArea.Width(), rcThisUpdateArea.Height(), Gdiplus::UnitPixel);
}
