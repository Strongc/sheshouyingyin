#include "stdafx.h"
#include "UILayer.h"
#include <ResLoader.h>

UILayer::UILayer(std::wstring respath, BOOL display /* = TRUE */, UINT nums /*= 1*/):
  m_stat(0)
{
  ResLoader rs;
  SetTexture(rs.LoadBitmap(respath), nums);
  SetDisplay(display);
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
    dc.AlphaBlend(m_texturepos.x, m_texturepos.y, m_bm.bmWidth, m_bm.bmHeight,
    texturedc, 0, m_bm.bmHeight * m_stat, m_bm.bmWidth, m_bm.bmHeight, bf);
  else
  {
    dc.SetStretchBltMode(COLORONCOLOR);
    dc.StretchBlt(m_texturepos.x, m_texturepos.y, m_bm.bmWidth, m_bm.bmHeight,
      texturedc, 0, m_bm.bmHeight * m_stat, m_bm.bmWidth, m_bm.bmHeight, SRCCOPY);
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
