#pragma once

#include "UILayer.h"

class ULBackground: public UILayer
{
public:
  ULBackground(std::wstring respath, BOOL display = TRUE);
  ~ULBackground();

  BOOL ActMouseOver(const POINT& pt);
  BOOL ActMouseOut(const POINT& pt);
  BOOL ActMouseDown(const POINT& pt);
  BOOL ActMouseUp(const POINT& pt);
};

class ULDel: public UILayer
{
public:
  ULDel(std::wstring respath, BOOL display = TRUE);
  ~ULDel();

  BOOL ActMouseOver(const POINT& pt);
  BOOL ActMouseOut(const POINT& pt);
  BOOL ActMouseDown(const POINT& pt);
  BOOL ActMouseUp(const POINT& pt);
};

class ULPlayback: public UILayer
{
public:
  ULPlayback(std::wstring respath, BOOL display = TRUE);
  ~ULPlayback();

  BOOL ActMouseOver(const POINT& pt);
  BOOL ActMouseOut(const POINT& pt);
  BOOL ActMouseDown(const POINT& pt);
  BOOL ActMouseUp(const POINT& pt);
};

class ULFavourite: public UILayer
{
public:
  ULFavourite(std::wstring respath, BOOL display = TRUE);
  ~ULFavourite();

  BOOL ActMouseOver(const POINT& pt);
  BOOL ActMouseOut(const POINT& pt);
  BOOL ActMouseDown(const POINT& pt);
  BOOL ActMouseUp(const POINT& pt);
};

class ULCover: public UILayer
{
public:
  ULCover(std::wstring respath, BOOL display = TRUE);
  ~ULCover();

  BOOL ActMouseOver(const POINT& pt);
  BOOL ActMouseOut(const POINT& pt);
  BOOL ActMouseDown(const POINT& pt);
  BOOL ActMouseUp(const POINT& pt);
};