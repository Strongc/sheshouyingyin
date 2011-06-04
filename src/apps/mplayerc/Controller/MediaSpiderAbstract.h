#pragma once

#include <threadhelper.h>

// The spider base class, abstract
template<class T>
class MediaSpiderAbstract :
  public ThreadHelperImpl<T>
{
public:

  BOOL AddDetectPath(const std::wstring& path)
  {
    if (path.empty())
      return FALSE;

    std::vector<std::wstring>::iterator it;
    for (it = m_detectpath.begin(); it != m_detectpath.end(); ++it)
    {
      if ((*it) == path)
        return FALSE;
    }

    m_detectpath.push_back(path);
    return TRUE;
  }

  BOOL DelDetectPath(const std::wstring& path)
  {
    if (path.empty())
      return FALSE;

    std::vector<std::wstring>::iterator it;
    for (it = m_detectpath.begin(); it != m_detectpath.end(); ++it)
    {
      if ((*it) == path)
      {
        m_detectpath.erase(it);
        return TRUE;
      }
    }
    return FALSE;
  }

  BOOL IsSupportExtension(const std::wstring& sFilePath)
  {
    // Find if the file is a media file
    std::wstring sExt = ::PathFindExtension(sFilePath.c_str());
    if (find(m_supportexts.begin(), m_supportexts.end(), sExt) != m_supportexts.end())
      return TRUE;

    return FALSE;
  }

  BOOL IsExcludePath(const std::wstring& path)
  {
    BOOL bRet = FALSE;

    // do not spide the removable disk
    if (path.size() >= 3)
    {
      if (path.substr(0, 2) == L"\\\\")
      {
        // determine the network path
        bRet = FALSE;
      }
      else
      {
        // determine other path
        UINT type = ::GetDriveType(path.substr(0, 3).c_str());
        switch (type)
        {
        case DRIVE_FIXED:
          bRet = FALSE;
          break;

        default:
          bRet = TRUE;
          break;
        }
      }
    }

    // find in the exclude folder list
    if (find(m_exincludepaths.begin(), m_exincludepaths.end(), path) != m_exincludepaths.end())
      bRet = TRUE;

    return bRet;
  }

  void SetExcludePath(const std::wstring& path)
  {
    m_exincludepaths.push_back(path);
  }

  void SetSupportExtension(const std::wstring& ext)
  {
    m_supportexts.push_back(ext);
  }

  bool IsFilteredItem(const std::wstring &str)
  {
    std::vector<std::wstring>::iterator it = m_filtered_items.begin();
    while (it != m_filtered_items.end())
    {
      if (str.find(*it) != std::wstring::npos)
        return true;

      ++it;
    }

    return false;
  }

  void SetFilteredItem(const std::wstring &str)
  {
    m_filtered_items.push_back(str);
  }

private:
  std::vector<std::wstring> m_supportexts;
  std::vector<std::wstring> m_exincludepaths;
  std::vector<std::wstring> m_detectpath;
  std::vector<std::wstring> m_filtered_items;  // filter user's some private media
};