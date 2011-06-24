#include "stdafx.h"
#include "MCUILayer.h"


ULBackground::ULBackground(std::wstring respath, BOOL display, UINT nums) :
  UILayer(respath, display, nums)
{

}

ULBackground::~ULBackground()
{

}

BOOL ULBackground::ActMouseOver(const POINT& pt)
{

  BOOL ret = FALSE;
  if (GetState() == 0)
  {
    ret = TRUE;
    SetState(1);
  }

  return ret;
}

BOOL ULBackground::ActMouseOut(const POINT& pt)
{
  BOOL ret = FALSE;

  RECT rc;
  GetTextureRect(rc);

  if (GetState() == 1 && !PtInRect(&rc, pt))
  {
    ret = TRUE;
    SetState(0);
  }

  return ret;
}

BOOL ULBackground::ActMouseLBDown(const POINT& pt)
{
  return FALSE;
}

BOOL ULBackground::ActMouseLBUp(const POINT& pt)
{
  return FALSE;
}

ULDel::ULDel(std::wstring respath, BOOL display, UINT nums) :
  UILayer(respath, display, nums)
{

}

ULDel::~ULDel()
{

}

BOOL ULDel::ActMouseOver(const POINT& pt)
{
  RECT rc;
  GetTextureRect(rc);

  BOOL ret = FALSE;

  if (PtInRect(&rc, pt))
  {
    SetState(1);
    ret = TRUE;
  }
  else if (GetState() == 1)
  {
    SetState(0);
    ret = TRUE;
  }

  return ret;
}

BOOL ULDel::ActMouseOut(const POINT& pt)
{
  BOOL ret = FALSE;

  RECT rc;
  GetTextureRect(rc);

  if (GetState() == 1 && !PtInRect(&rc, pt))
  {
    ret = TRUE;
    SetState(0);
  }

  return ret;
}

BOOL ULDel::ActMouseLBDown(const POINT& pt)
{
  RECT rc;
  GetTextureRect(rc);

  BOOL bl = FALSE;

  if (PtInRect(&rc, pt))
    bl = TRUE;

  return bl;
}

BOOL ULDel::ActMouseLBUp(const POINT& pt)
{
  return FALSE;
}


ULPlayback::ULPlayback(std::wstring respath, BOOL display, UINT nums) :
UILayer(respath, display, nums)
{

}

ULPlayback::~ULPlayback()
{

}

BOOL ULPlayback::ActMouseOver(const POINT& pt)
{
  RECT rc;
  GetTextureRect(rc);

  BOOL ret = FALSE;

  if (PtInRect(&rc, pt))
  {
    SetState(1);
    ret = TRUE;
  }
  else if (GetState() == 1)
  {
    SetState(0);
    ret = TRUE;
  }

  return ret;
}

BOOL ULPlayback::ActMouseOut(const POINT& pt)
{
  BOOL ret = FALSE;

  RECT rc;
  GetTextureRect(rc);

  if (GetState() == 1 && !PtInRect(&rc, pt))
  {
    ret = TRUE;
    SetState(0);
  }


  return ret;
}

BOOL ULPlayback::ActMouseLBDown(const POINT& pt)
{
  RECT rc;
  GetTextureRect(rc);

  BOOL bl = FALSE;

  if (PtInRect(&rc, pt))
    bl = TRUE;

  return bl;
}

BOOL ULPlayback::ActMouseLBUp(const POINT& pt)
{
  return FALSE;
}

ULFavourite::ULFavourite(std::wstring respath, BOOL display, UINT nums):
  UILayer(respath, display, nums)
{
  
}
 
ULFavourite::~ULFavourite()
{

}

BOOL ULFavourite::ActMouseOver(const POINT& pt)
{
  RECT rc;
  GetTextureRect(rc);

  BOOL ret = FALSE;

  if (PtInRect(&rc, pt))
  {
    SetState(1);
    ret = TRUE;
  }
  else if (GetState() == 1)
  {
    SetState(0);
    ret = TRUE;
  }

  return ret;
}

BOOL ULFavourite::ActMouseOut(const POINT& pt)
{
  BOOL ret = FALSE;

  RECT rc;
  GetTextureRect(rc);

  if (GetState() == 1 && !PtInRect(&rc, pt))
  {
    ret = TRUE;
    SetState(0);
  }


  return ret;
}

BOOL ULFavourite::ActMouseLBDown(const POINT& pt)
{
  return FALSE;
}

BOOL ULFavourite::ActMouseLBUp(const POINT& pt)
{
  return FALSE;
}

ULCover::ULCover(std::wstring respath, BOOL display, UINT nums):
  UILayer(respath, display, nums)
{

}

ULCover::~ULCover()
{

}

BOOL ULCover::ActMouseOver(const POINT& pt)
{
  RECT rc;
  GetTextureRect(rc);

  BOOL ret = FALSE;

  if (PtInRect(&rc, pt))
  {
    SetState(1);
    ret = TRUE;
  }
  else if (GetState() == 1)
  {
    SetState(0);
    ret = TRUE;
  }

  return ret;
}

BOOL ULCover::ActMouseOut(const POINT& pt)
{
  BOOL ret = FALSE;

  RECT rc;
  GetTextureRect(rc);

  if (GetState() == 1 && !PtInRect(&rc, pt))
  {
    ret = TRUE;
    SetState(0);
  }


  return ret;
}

BOOL ULCover::ActMouseLBDown(const POINT& pt)
{
  RECT rc;
  GetTextureRect(rc);

  BOOL bl = FALSE;

  if (PtInRect(&rc, pt))
    bl = TRUE;

  return bl;
}

BOOL ULCover::ActMouseLBUp(const POINT& pt)
{
  return FALSE;
}

ULMask::ULMask(std::wstring respath, BOOL display, UINT nums) :
UILayer(respath, display, nums)
{

}

ULMask::~ULMask()
{

}