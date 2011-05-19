#include "stdafx.h"
#include "MCUILayer.h"


ULBackground::ULBackground(std::wstring respath, BOOL display) :
  UILayer(respath, display)
{

}

ULBackground::~ULBackground()
{

}

BOOL ULBackground::ActMouseOver(const POINT& pt)
{
  RECT rc;
  GetTextureRect(rc);

  int prestat = GetState();
  int stat = 0;
  BOOL bl = FALSE;

  if (PtInRect(&rc, pt))
    stat = 1;

  if (prestat != stat)
    bl = TRUE;

  SetState(stat);
  
  return bl;
}

BOOL ULBackground::ActMouseOut(const POINT& pt)
{
  SetState(0);

  return TRUE;
}

BOOL ULBackground::ActMouseDown(const POINT& pt)
{
  return FALSE;
}

BOOL ULBackground::ActMouseUp(const POINT& pt)
{
  return FALSE;
}

ULDel::ULDel(std::wstring respath, BOOL display) :
  UILayer(respath, display)
{

}

ULDel::~ULDel()
{

}

BOOL ULDel::ActMouseOver(const POINT& pt)
{
  RECT rc;
  GetTextureRect(rc);

  int prestat = GetState();
  int stat = 0;
  BOOL bl = FALSE;

  if (PtInRect(&rc, pt))
    stat = 1;

  if (prestat != stat)
    bl = TRUE;

  SetState(stat);

  return bl;
}

BOOL ULDel::ActMouseOut(const POINT& pt)
{
  SetState(0);

  return TRUE;
}

BOOL ULDel::ActMouseDown(const POINT& pt)
{
  RECT rc;
  GetTextureRect(rc);

  BOOL bl = FALSE;

  if (PtInRect(&rc, pt))
    bl = TRUE;

  return bl;
}

BOOL ULDel::ActMouseUp(const POINT& pt)
{
  return FALSE;
}


ULPlayback::ULPlayback(std::wstring respath, BOOL display) :
UILayer(respath, display)
{

}

ULPlayback::~ULPlayback()
{

}

BOOL ULPlayback::ActMouseOver(const POINT& pt)
{
  RECT rc;
  GetTextureRect(rc);

  int prestat = GetState();
  int stat = 0;
  BOOL bl = FALSE;

  if (PtInRect(&rc, pt))
    stat = 1;

  if (prestat != stat)
    bl = TRUE;

  SetState(stat);

  return bl;
}

BOOL ULPlayback::ActMouseOut(const POINT& pt)
{
  SetState(0);

  return TRUE;
}

BOOL ULPlayback::ActMouseDown(const POINT& pt)
{
  RECT rc;
  GetTextureRect(rc);

  BOOL bl = FALSE;

  if (PtInRect(&rc, pt))
    bl = TRUE;

  return bl;
}

BOOL ULPlayback::ActMouseUp(const POINT& pt)
{
  return FALSE;
}

ULFavourite::ULFavourite(std::wstring respath, BOOL display):
  UILayer(respath, display)
{
  
}
 
ULFavourite::~ULFavourite()
{

}

BOOL ULFavourite::ActMouseOver(const POINT& pt)
{
  RECT rc;
  GetTextureRect(rc);

  int prestat = GetState();
  int stat = 0;
  BOOL bl = FALSE;

  if (PtInRect(&rc, pt))
    stat = 1;

  if (prestat != stat)
    bl = TRUE;

  SetState(stat);

  return bl;
}

BOOL ULFavourite::ActMouseOut(const POINT& pt)
{
  SetState(0);

  return TRUE;
}

BOOL ULFavourite::ActMouseDown(const POINT& pt)
{
  return FALSE;
}

BOOL ULFavourite::ActMouseUp(const POINT& pt)
{
  return FALSE;
}

ULCover::ULCover(std::wstring respath, BOOL display):
  UILayer(respath, display)
{

}

ULCover::~ULCover()
{

}

BOOL ULCover::ActMouseOver(const POINT& pt)
{
  RECT rc;
  GetTextureRect(rc);

  int prestat = GetState();
  int stat = 0;
  BOOL bl = FALSE;

  if (PtInRect(&rc, pt))
    stat = 1;

  if (prestat != stat)
    bl = TRUE;

  SetState(stat);

  return bl;
}

BOOL ULCover::ActMouseOut(const POINT& pt)
{
  SetState(0);

  return TRUE;
}

BOOL ULCover::ActMouseDown(const POINT& pt)
{
  return FALSE;
}

BOOL ULCover::ActMouseUp(const POINT& pt)
{
  return FALSE;
}