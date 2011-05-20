#pragma once

class TextEdit : public WTL::CEdit
{
// Properties
public:
  void SetTextVCenter();

// Message handler
public:
  BEGIN_MSG_MAP_EX(TextEdit)
  END_MSG_MAP()
};