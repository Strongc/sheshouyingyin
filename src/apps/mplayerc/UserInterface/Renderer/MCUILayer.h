#pragma once

#include "UILayer.h"

class ULDel: public UILayer
{
public:
  ULDel(std::wstring respath, BOOL display = TRUE);
  ~ULDel();

  void ActMouseOver();
  void ActMouseOut();
  void ActMouseDown();
  void ActMouseUp();
};

class ULPlayback: public UILayer
{
public:
  ULPlayback(std::wstring respath, BOOL display = TRUE);
  ~ULPlayback();

  void ActMouseOver();
  void ActMouseOut();
  void ActMouseDown();
  void ActMouseUp();
};