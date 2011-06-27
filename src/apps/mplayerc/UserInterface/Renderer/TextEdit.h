#pragma once

class TextEdit : public CEdit
{
// Normal part
public:
  TextEdit();
  ~TextEdit();

// Properties
public:
  void SetTextColor(int red, int green, int blue);
  COLORREF GetTextColor();

  void SetBKColor(int red, int green, int blue);
  COLORREF GetBKColor();
  void SetTextVCenter();

// Message handler
public:
  afx_msg HBRUSH CtlColor(CDC* pDC, UINT nCtlColor);

  DECLARE_MESSAGE_MAP()

// data
private:
  COLORREF m_crTextColor;
  
  COLORREF m_crBKColor;
  HBRUSH m_brBK;
};