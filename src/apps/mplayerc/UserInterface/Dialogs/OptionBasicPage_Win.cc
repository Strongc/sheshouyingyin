#include "stdafx.h"
#include "OptionBasicPage_Win.h"
#include <Strings.h>
#include "../../mplayerc.h"
#include "../../svplib/svplib.h"
#include "../../Controller/Hotkey_Controller.h"
#include "../../Controller/PlayerPreference.h"
#include "../../Controller/SPlayerDefs.h"

BOOL OptionBasicPage::OnInitDialog(HWND hwnd, LPARAM lParam)
{
  // background image picker
  m_userbkgnd_edit.SubclassWindow(GetDlgItem(IDC_EDIT_BKGNDFILE));
  m_userbkgnd_edit.Init(ID_BKGND_PICKER);
  // upgrade strategy combo
  m_upgradestrategy_combo.Attach(GetDlgItem(IDC_COMBO_AUTOUPDATEVER));
  m_hotkeyscheme_combo.Attach(GetDlgItem(IDC_COMBO_HOTKEYSCHEME));

  WTL::CString text;
  text.LoadString(IDS_UPGRADESTRATEGY);
  std::vector<std::wstring> text_ar;
  Strings::Split(text, L"|", text_ar);
  for (std::vector<std::wstring>::iterator it = text_ar.begin();
    it != text_ar.end(); it++)
    m_upgradestrategy_combo.AddString(it->c_str());

  m_autoscalecheckbox.Attach(GetDlgItem(IDC_CHECK_BKGNDAUTOSCALE));
  m_aeroglasscheckbox.Attach(GetDlgItem(IDC_CHECK_AERO));

  AppSettings& s = AfxGetAppSettings();
  PlayerPreference* pref = PlayerPreference::GetInstance();

  // retrieve variables from preference
  m_bkgnd = s.logoext?1:0;  // logoext is "use external logo"
  m_autoscalecheckbox.EnableWindow(s.logoext);

  // feed variables onto screen
  m_userbkgnd_edit.SetWindowText(s.logofn);

  m_aeroglasscheckbox.EnableWindow(s.bAeroGlassAvalibility);

  // s.logostretch 
  // 1 keep aspect ratio
  // 2 stretch to full screen
  m_autoscalebkgnd = !!(pref->GetIntVar(INTVAR_LOGO_AUTOSTRETCH) & 1);
  m_useaero = s.bAeroGlass;
  m_repeat = s.fLoopForever;
  m_mintotray = s.fTrayIcon;
  m_autoresume =  s.autoResumePlay;
  m_autofullscreen = pref->GetIntVar(INTVAR_TOGGLEFULLSCRENWHENPLAYBACKSTARTED);
  m_leftclick2pause = !pref->GetIntVar(INTVAR_LEFTCLICK2PAUSE);
  m_autoupgrade = (s.tLastCheckUpdater < 2000000000);

  //This might need revise
  CSVPToolBox svpTool;
  CPath updPath( svpTool.GetPlayerPath(_T("UPD")));
  updPath.AddBackslash();
  CString szUpdfilesPath(updPath);
  CString szBranch = svpTool.fileGetContent(szUpdfilesPath + _T("branch") );
  if (szBranch == L"stable")
    m_upgradestrategy_combo.SetCurSel(1);
  else if (szBranch == L"beta")
    m_upgradestrategy_combo.SetCurSel(2);
  else
    m_upgradestrategy_combo.SetCurSel(0);

  m_hotkeyscheme = 0;

  PrepareHotkeySchemes();

  DoDataExchange();
  Validate();
  return TRUE;
}

void OptionBasicPage::OnDestroy()
{
  m_upgradestrategy_combo.Detach();
  m_hotkeyscheme_combo.Detach();
  m_autoscalecheckbox.Detach();
  m_aeroglasscheckbox.Detach();
}

void OptionBasicPage::OnBkgndPickerSetFocused(UINT uNotifyCode, int nID, CWindow wndCtl)
{
  m_bkgnd = 1;
  DoDataExchange();
  Validate();
}

void OptionBasicPage::OnBkgndPicker(UINT uNotifyCode, int nID, CWindow wndCtl)
{
  // call ::OpenFileDialog to prompt choosing a file
  WTL::CFileDialog fd(TRUE, NULL, NULL, OFN_EXPLORER, 
    L"*.bmp;*.jpg;*.gif;*.png\0*.bmp;*.jpg;*.gif;*.png\0\0", m_hWnd);

  if(fd.DoModal(m_hWnd) != IDOK)
    return;

  // set window text of edit control only
  m_userbkgnd_edit.SetWindowText(fd.m_szFileName);

  // we are using external bkng pic now
  m_bkgnd = 1;
  DoDataExchange();
}

// event when setting is updated
void OptionBasicPage::OnBkgndUpdated(UINT uNotifyCode, int nID, CWindow wndCtl)
{
  m_bkgnd = IsDlgButtonChecked(IDC_RADIO_NOBKGND)?0:1;
  Validate();
}

void OptionBasicPage::OnAutoUpgradeChanged(UINT uNotifyCode, int nID, CWindow wndCtl)
{
  m_autoupgrade = IsDlgButtonChecked(IDC_CHECK_AUTOUPGRADE)?1:0;
  Validate();
}

void OptionBasicPage::OnCustomHotkeyChanged(UINT uNotifyCode, int nID, CWindow wndCtl)
{
  m_hotkeyscheme = IsDlgButtonChecked(IDC_CHECK_HOTKEYSCHEME)?1:0;
  Validate();
}

int OptionBasicPage::OnSetActive()
{
  return 0;
}

int OptionBasicPage::OnApply()
{
  // retrieve variables from screen
  DoDataExchange(TRUE);
  AppSettings& s = AfxGetAppSettings();
  // feed variables into preference
  s.logoext = m_bkgnd==1?true:false;
  m_userbkgnd_edit.GetWindowText(s.logofn);

  PlayerPreference* pref = PlayerPreference::GetInstance();
  pref->SetIntVar(INTVAR_LOGO_AUTOSTRETCH, m_autoscalebkgnd?3:0);
  s.bAeroGlass = m_useaero;
  s.fLoopForever = m_repeat?true:false;
  s.fTrayIcon = m_mintotray?true:false;
  s.autoResumePlay = m_autoresume;
  pref->SetIntVar(INTVAR_TOGGLEFULLSCRENWHENPLAYBACKSTARTED, m_autofullscreen?true:false);
  pref->SetIntVar(INTVAR_LEFTCLICK2PAUSE, m_leftclick2pause?false:true);

  if (m_autoupgrade)
    s.tLastCheckUpdater = (UINT)time(NULL) - 100000;
  else
    s.tLastCheckUpdater = 2000000000;

  //This might need revise
  CSVPToolBox svpTool;
  CPath updPath( svpTool.GetPlayerPath(_T("UPD")));
  updPath.AddBackslash();
  CString szUpdfilesPath(updPath);
  switch (m_upgradestrategy_combo.GetCurSel())
  {
    case 1:
      svpTool.filePutContent( szUpdfilesPath + _T("branch") , L"stable");
      break;
    case 2:
      svpTool.filePutContent( szUpdfilesPath + _T("branch") , L"beta");
      break;
    default:
      ::DeleteFile(szUpdfilesPath + _T("branch"));
      break;
  }

  // apply hotkeyscheme
  std::wstring hotkeyscheme;
  if (m_hotkeyscheme)
  {
    int index = m_hotkeyscheme_combo.GetCurSel();
    if (index >= 0 && index < (int)m_files.size())
      hotkeyscheme = m_files[index];
  }
  pref->SetStringVar(STRVAR_HOTKEYSCHEME, hotkeyscheme);

  return PSNRET_NOERROR;
}

void OptionBasicPage::Validate()
{
  // update scale checkbox and exit box
  m_autoscalecheckbox.EnableWindow(m_bkgnd == 1);
  m_upgradestrategy_combo.EnableWindow(m_autoupgrade == 1);
  m_hotkeyscheme_combo.EnableWindow(m_hotkeyscheme != 0);
}

void OptionBasicPage::PrepareHotkeySchemes()
{
  int cmdshow;

  HotkeyController* con = HotkeyController::GetInstance();
  con->EnumAvailableSchemes(m_files, m_schemenames);
  
  cmdshow = (!m_schemenames.size())?SW_HIDE:SW_SHOW;

  GetDlgItem(IDC_CHECK_HOTKEYSCHEME).ShowWindow(cmdshow);
  GetDlgItem(IDC_COMBO_HOTKEYSCHEME).ShowWindow(cmdshow);

  if (cmdshow == SW_HIDE)
    return;

  for (std::vector<std::wstring>::iterator it = m_schemenames.begin();
    it != m_schemenames.end(); it++)
    m_hotkeyscheme_combo.AddString((*it).c_str());
  m_hotkeyscheme_combo.SetCurSel(0);

  PlayerPreference* pref = PlayerPreference::GetInstance();
  std::wstring scheme = pref->GetStringVar(STRVAR_HOTKEYSCHEME);
  if (scheme.length() > 0)
    m_hotkeyscheme = 1;
  else
    return;

  for (std::vector<std::wstring>::iterator it = m_files.begin();
    it != m_files.end(); it++)
  {
    if (_wcsicmp(scheme.c_str(), (*it).c_str()) == 0)
    {
      m_hotkeyscheme_combo.SetCurSel(distance(m_files.begin(), it));
      break;
    }
  }
}