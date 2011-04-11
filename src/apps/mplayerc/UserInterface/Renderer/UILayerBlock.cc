#include "stdafx.h"
#include "UILayerBlock.h"

#define  BEHITTEST 11
#define  BEDELETE   12
#define  BEPLAY    13

UILayerBlock::UILayerBlock():
m_lastbtnstate(FALSE)
{

}

UILayerBlock::~UILayerBlock()
{

}

BOOL UILayerBlock::GetUILayer(std::wstring key, UILayer** layer)
{
  *layer = NULL;

  if (m_layers.empty())
    return FALSE;

  if (m_layers.find(key) == m_layers.end())
    return FALSE;

  *layer = m_layers[key];
  return TRUE;
}

BOOL UILayerBlock::AddUILayer(std::wstring key, UILayer* layer)
{
  if (m_layers.find(key) != m_layers.end())
    return FALSE;

  m_layers[key] = layer;
  return TRUE;
}

BOOL UILayerBlock::DelUILayer(std::wstring key)
{
  if (m_layers.find(key) == m_layers.end())
    return FALSE;

  m_layers.erase(key);
  return TRUE;
}

BOOL UILayerBlock::DoPaint(WTL::CDC& dc)
{
  if (m_layers.empty())
    return FALSE;

  std::map<std::wstring, UILayer*>::iterator it;
  for (it = m_layers.begin(); it != m_layers.end(); it++)
  {
    (it->second)->DoPaint(dc);
  }

  return TRUE;
}

BOOL UILayerBlock::DeleteAllLayer()
{
  if (m_layers.empty())
    return FALSE;

  std::map<std::wstring, UILayer*>::iterator it;
  for (it = m_layers.begin(); it != m_layers.end(); it++)
  {
    delete (it->second);
  }

  m_layers.clear();

  return TRUE;
}

int UILayerBlock::OnHittest(POINT pt, BOOL blbtndown)
{
  if (blbtndown == -1)
    blbtndown = m_lastbtnstate;
  else
    m_lastbtnstate = blbtndown;

  int state = 0;
  for (std::map<std::wstring, UILayer*>::iterator it = m_layers.begin();
       it != m_layers.end(); ++it)
  {
    BOOL bl = it->second->OnHittest(pt, blbtndown);
    if (it->first.find(L"del") != std::wstring::npos && blbtndown && bl)
      return BEDELETE;
    else if (it->first.find(L"play") != std::wstring::npos && blbtndown && bl)
      return BEPLAY;
    else if (bl)
      state = BEHITTEST;
  }
  return state;
}