#ifndef PLAYERPREFERENCE_H
#define PLAYERPREFERENCE_H

#include "LazyInstance.h"

class SQLliteapp;

class PlayerPreference :
  public LazyInstanceImpl<PlayerPreference>
{
public:
  PlayerPreference(void);
  ~PlayerPreference(void);

  int GetIntVar(int id);
  void SetIntVar(int id, int value_in);

  long long GetInt64Var(int id);
  void SetInt64Var(int id, long long value_in);

  std::wstring GetStringVar(int id);
  void SetStringVar(int id, std::wstring &value_in);

  std::vector<int> GetIntArray(int id);
  void SetIntArray(int id, std::vector<int> &value_in);

  std::vector<std::wstring> GetStrArray(int id);
  void SetStrArray(int id, std::vector<std::wstring> &value_in);

  SQLliteapp* GetSqliteSettingPtr() {return sqlite_setting;}
private:
  void Init();
  void Uninit();

  BOOL WriteProfileInt(LPCTSTR lpszSection, LPCTSTR lpszEntry, int nValue);
  UINT GetProfileInt(LPCTSTR lpszSection, LPCTSTR lpszEntry, int nDefault);

  BOOL WriteProfileString(LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpszValue);
  CString GetProfileString(LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpszDefault);

  std::map<int, int>          m_map_intvar;
  std::map<int, long long>    m_map_int64var;
  std::map<int, std::wstring> m_map_strvar;
  std::map<int, std::vector<int>>          m_map_intarray;
  std::map<int, std::vector<std::wstring>> m_map_strarray;

  SQLliteapp* sqlite_setting; 

};

#endif //PLAYERPREFERENCE_H
