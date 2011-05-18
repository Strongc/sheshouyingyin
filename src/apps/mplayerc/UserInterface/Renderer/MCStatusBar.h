#pragma once

class MCStatusBar
{
/* Normal part */
public:
  MCStatusBar();
  ~MCStatusBar();

/* Properties */
public:
  void SetFrame(HWND hwnd);
  void SetRect(const CRect &rc);
  void SetText(const std::wstring &str);
  void SetVisible(bool bVisible);
  void SetBKColor(COLORREF cr);

  CRect GetRect() const;
  bool GetVisible() const;
  COLORREF GetBKColor() const;

/* Operations */
public:
  void Update();

/* Event handler */
protected:
  void OnPaint();

/* Private data */
private:
  HWND m_hwnd;
  CRect m_rc;  // this rect is relative to the m_hwnd's client area!!!
  bool m_bVisible;
  
  std::wstring m_str;
  COLORREF m_crBKColor;
};