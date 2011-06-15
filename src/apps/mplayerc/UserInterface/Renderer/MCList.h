#pragma once

#include <vector>
#include "SPScrollBar.h"
#include "../../Model/MCBlockUnit.h"
#include "../../Model/MCDBSource.h"

class SPMCList
{
public:
  SPMCList();
  ~SPMCList();

  SPScrollBar* GetScrollBar();
  CSize GetMCSize();

  void Update(DWORD deltatime);
  void DoPaint(WTL::CDC& dc, RECT& rcclient);

  BOOL ActMouseMove(const POINT& pt);
  BOOL ActMouseLBDown(const POINT& pt);
  BOOL ActMouseLBUp(const POINT& pt);
  BOOL ActWindowChange(int w, int h);

private:
  void SetCover();
  void AddScrollBar();
  void BlocksMouseMove(const POINT& pt);

  void AlignColumns();
  void AlignRows();

private:
  MCDBSource* m_dbsource;
  SPScrollBar* m_sbar;
  CSize m_wndsize;

  std::vector<int> m_x;
  std::vector<int> m_y;
  
  BOOL m_listempty;

  BlockUnit* m_selblockunit;

  int m_blockw;
  int m_blockh;

  int m_blockspacing;
  int m_blocktop;
  
  RECT m_wndmargin;

  BOOL m_lockpaint;

  DWORD m_deltatime;

  WTL::CBitmap m_cover;
  CSize m_coversize;
};