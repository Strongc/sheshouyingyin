#include "stdafx.h"
#include "UILayer.h"
#include <ResLoader.h>
#include <sstream>
#include "logging.h"

UILayer::UILayer(std::wstring respath, BOOL display /* = TRUE */, UINT nums /*= 1*/):
  m_stat(0),
  m_displaywidth(0),
  m_displayheight(0)
{
  ResLoader rs;
  HBITMAP hBitmap = rs.LoadBitmap(respath);
  SetTexture(hBitmap, nums);
  SetDisplay(display);

  // set the original rect
  m_orginalsize.cx = 0;
  m_orginalsize.cy = 0;

  BITMAP bmInfo = {0};
  ::GetObject(hBitmap, sizeof(BITMAP), &bmInfo);

  m_orginalsize.cx = bmInfo.bmWidth;
  m_orginalsize.cy = bmInfo.bmHeight;
  if (nums)
    m_orginalsize.cy /= nums;
}

UILayer::~UILayer()
{

}

BOOL UILayer::SetTexture(HBITMAP texture, UINT nums)
{
  if (texture == NULL)
    return FALSE;

  m_texture.Attach(texture);

  m_texture.GetBitmap(&m_bm);
  
  if(m_bm.bmBitsPixel == 32)
  {
    for (int y=0; y<m_bm.bmHeight; y++)
    {
      BYTE * pPixel = (BYTE *) m_bm.bmBits + m_bm.bmWidth * 4 * y;
      for (int x=0; x<m_bm.bmWidth; x++)
      {
        pPixel[0] = pPixel[0] * pPixel[3] / 255; 
        pPixel[1] = pPixel[1] * pPixel[3] / 255; 
        pPixel[2] = pPixel[2] * pPixel[3] / 255; 
        pPixel += 4;
      }
    }
  }

  if (nums)
    m_bm.bmHeight /= nums;

  return TRUE;
}

BOOL UILayer::GetTextureRect(RECT& rc)
{
  rc.left = m_texturepos.x;
  rc.top = m_texturepos.y;
  rc.right = rc.left + m_bm.bmWidth;
  rc.bottom = rc.top + m_bm.bmHeight;

  return TRUE;
}

BOOL UILayer::GetTextureWH(int& w, int& h)
{
  BOOL ret = FALSE;
  if (m_bm.bmWidth)
  {
    w = m_bm.bmWidth;
    h = m_bm.bmHeight;
    ret = TRUE;
  }
  else
  {
    w = 0;
    h = 0;
  }
  return ret;
}

BOOL UILayer::GetTexturePos(POINT& pt)
{
  pt = m_texturepos;
  return TRUE;
}

BOOL UILayer::SetTexturePos(const POINT& pt)
{
  m_texturepos = pt;
  return TRUE;
}

BOOL UILayer::SetDisplay(BOOL display)
{
  m_display = display;
  return TRUE;
}

BOOL UILayer::GetDisplay()
{
  return m_display;
}

void UILayer::SetDisplayWH(int w, int h)
{
  m_displaywidth = w;
  m_displayheight = h;
}

BOOL UILayer::DoPaint(WTL::CDC& dc)
{
  if (!m_display)
    return FALSE;

  WTL::CDC texturedc;
  HBITMAP hold_texture;
  
  texturedc.CreateCompatibleDC(dc);
  hold_texture = texturedc.SelectBitmap(m_texture);

  BLENDFUNCTION bf = {AC_SRC_OVER, 0, 255, AC_SRC_ALPHA};
  if (m_bm.bmBitsPixel == 32)
  {
    int w = (m_displaywidth) ? m_displaywidth : m_bm.bmWidth;
    int h = (m_displayheight) ? m_displayheight: m_bm.bmHeight;

    dc.AlphaBlend(m_texturepos.x, m_texturepos.y, w, h, 
      texturedc, 0, m_bm.bmHeight * m_stat, m_bm.bmWidth, m_bm.bmHeight, bf);
  }
  else
  {
    dc.SetStretchBltMode(COLORONCOLOR);

    // adjust the ratio
    if ((m_bm.bmHeight != 0) && (m_bm.bmWidth != 0))
    {
      float fWHRatio = (float)(m_bm.bmWidth) / m_bm.bmHeight;
      float fHWRatio = (float)(m_bm.bmHeight) / m_bm.bmWidth;

      if (m_bm.bmWidth > m_orginalsize.cx)
      {
        // width is the same value as layer's width
        // so we adjust the vertical position
        int nYOffset = (m_orginalsize.cy - m_orginalsize.cx * fHWRatio) / 2.0;
        dc.StretchBlt(m_texturepos.x, m_texturepos.y + nYOffset, m_orginalsize.cx, m_orginalsize.cx * fHWRatio
          , texturedc, 0, 0, m_bm.bmWidth, m_bm.bmHeight, SRCCOPY);
      } 
      else
      {
        // height is the same value as layer's height
        // so we adjust the horizontal position
        int nXOffset = (m_orginalsize.cx - m_orginalsize.cy * fWHRatio) / 2.0;
        dc.StretchBlt(m_texturepos.x + nXOffset, m_texturepos.y, m_orginalsize.cy * fWHRatio, m_orginalsize.cy
          , texturedc, 0, 0, m_bm.bmWidth, m_bm.bmHeight, SRCCOPY);
      }
    }
    else
    {
      Logging(L"layer height or width is invalid");
    }
  }

  texturedc.SelectBitmap(hold_texture);
  texturedc.DeleteDC();

  return TRUE;
}

BOOL UILayer::DeleteTexture()
{
  return m_texture.DeleteObject();
}

void UILayer::SetState(int stat)
{
  m_stat = stat;
}

int UILayer::GetState()
{
  return m_stat;
}