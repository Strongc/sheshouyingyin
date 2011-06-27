#pragma once

#include "UILayer.h"

class ULBackground: public UILayer
{
public:
  ULBackground(std::wstring respath, BOOL display = TRUE, UINT nums = 1);
  ~ULBackground();

  BOOL ActMouseOver(const POINT& pt);
  BOOL ActMouseOut(const POINT& pt);
  BOOL ActMouseLBDown(const POINT& pt);
  BOOL ActMouseLBUp(const POINT& pt);
};

class ULDel: public UILayer
{
public:
  ULDel(std::wstring respath, BOOL display = TRUE, UINT nums = 1);
  ~ULDel();

  BOOL ActMouseOver(const POINT& pt);
  BOOL ActMouseOut(const POINT& pt);
  BOOL ActMouseLBDown(const POINT& pt);
  BOOL ActMouseLBUp(const POINT& pt);
};

class ULPlayback: public UILayer
{
public:
  ULPlayback(std::wstring respath, BOOL display = TRUE, UINT nums = 1);
  ~ULPlayback();

  BOOL ActMouseOver(const POINT& pt);
  BOOL ActMouseOut(const POINT& pt);
  BOOL ActMouseLBDown(const POINT& pt);
  BOOL ActMouseLBUp(const POINT& pt);
};

class ULFavourite: public UILayer
{
public:
  ULFavourite(std::wstring respath, BOOL display = TRUE, UINT nums = 1);
  ~ULFavourite();

  BOOL ActMouseOver(const POINT& pt);
  BOOL ActMouseOut(const POINT& pt);
  BOOL ActMouseLBDown(const POINT& pt);
  BOOL ActMouseLBUp(const POINT& pt);
};

class ULCover: public UILayer
{
public:
  ULCover(std::wstring respath, BOOL display = TRUE, UINT nums = 1);
  ~ULCover();

  BOOL ActMouseOver(const POINT& pt);
  BOOL ActMouseOut(const POINT& pt);
  BOOL ActMouseLBDown(const POINT& pt);
  BOOL ActMouseLBUp(const POINT& pt);
};

class ULMask: public UILayer
{
public:
  ULMask(std::wstring respath, BOOL display = TRUE, UINT nums = 1);
  ~ULMask();
};

class ULMCOpenButton: public UILayer
{
public:
  ULMCOpenButton(std::wstring respath, BOOL display = TRUE, UINT nums = 1);
  ~ULMCOpenButton();

  BOOL ActMouseOver(const POINT& pt);
  BOOL ActMouseOut(const POINT& pt);
  BOOL ActMouseLBUp(const POINT& pt);
};