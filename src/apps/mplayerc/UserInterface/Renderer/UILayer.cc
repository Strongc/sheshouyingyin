#include "stdafx.h"
#include "UILayer.h"
#include <ResLoader.h>
#include <sstream>
#include "logging.h"

UILayer::UILayer(std::wstring respath, BOOL display /* = TRUE */, UINT nums /*= 1*/):
  m_stat(0),
  m_displaywidth(0),
  m_displayheight(0),
  m_bltx(0),
  m_blty(0),
  m_bltw(0),
  m_blth(0)
{
  ResLoader rs;
  HBITMAP hBitmap = rs.LoadBitmap(respath);
  SetTexture(hBitmap, nums);
  SetDisplay(display);
}

UILayer::~UILayer()
{

}

BOOL UILayer::SetTexture(HBITMAP texture, UINT nums)
{
  if (texture == NULL)
    return FALSE;

  m_displaywidth = 0;
  m_displayheight = 0;
  m_bltx = 0;
  m_blty = 0;
  m_bltw = 0;
  m_blth = 0;

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

void UILayer::SetBltPos(int x, int y)
{
  m_bltx = x;
  m_blty = y;
}

void UILayer::SetBltWH(int w, int h)
{
  m_bltw = w;
  m_blth = h;
}

void UILayer::TiledTexture(int w, int h)
{
  int bltw = w;
  int blth = h;
  int x = 0, y = 0;

  if (m_bm.bmWidth <= w && m_bm.bmHeight <= h)
  {
    bltw = m_bm.bmWidth;
    blth = m_bm.bmHeight;
  }
  else
  {
    float ratiow = (float)m_bm.bmWidth / (float)w;
    float ratioh = (float)m_bm.bmHeight / (float)h;

    if (ratiow > ratioh)
    {
      bltw = (float)(m_bm.bmHeight * w) / (float)h;
      blth = m_bm.bmHeight;
      x = (float)(m_bm.bmWidth - bltw) / 2.f;
    }
    else if (ratiow < ratioh)
    {
      blth = (float)(m_bm.bmWidth * h) / (float)w;
      bltw = m_bm.bmWidth;
      y = (float)(m_bm.bmHeight - blth) / 2.f;
    }
  }

  SetBltPos(x, y);
  SetBltWH(bltw, blth);
}

BOOL UILayer::DoPaint(WTL::CDC& dc)
{
  if (!m_display)
    return FALSE;

  WTL::CDC texturedc;
  HBITMAP hold_texture;
  
  texturedc.CreateCompatibleDC(dc);
  hold_texture = texturedc.SelectBitmap(m_texture);

  int displayw = m_displaywidth ? m_displaywidth : m_bm.bmWidth;
  int displayh = m_displayheight ? m_displayheight : m_bm.bmHeight;

  int bltx = m_bltx ? m_bltx : 0;
  int blty = m_blty ? m_blty : m_bm.bmHeight * m_stat;

  int bltw = m_bltw ? m_bltw : m_bm.bmWidth;
  int blth = m_blth ? m_blth : m_bm.bmHeight;

  if (m_bm.bmBitsPixel == 32)
  {
    BLENDFUNCTION bf = {AC_SRC_OVER, 0, 255, AC_SRC_ALPHA};
    dc.AlphaBlend(m_texturepos.x, m_texturepos.y, displayw, displayh, 
      texturedc, bltx, blty, bltw, blth, bf);
  }
  else
  {
    dc.SetStretchBltMode(COLORONCOLOR);
    dc.StretchBlt(m_texturepos.x, m_texturepos.y, displayw, displayh,
      texturedc, bltx, blty, bltw, blth, SRCCOPY);
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