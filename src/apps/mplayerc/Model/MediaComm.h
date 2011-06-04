#pragma once

#include <boost/filesystem.hpp>
#include <boost/regex.hpp>
#include <tcl/tree.h>

// *****************************************************************************
// MediaData and MediaPath
struct MediaData
{
  MediaData() : uniqueid(0), videotime(0), bHide(false) {}

  long long uniqueid;
  std::wstring path;
  std::wstring filename;
  std::wstring thumbnailpath;
  std::wstring filmname;
  std::wstring hash;   // media's hash value, using media center to generate
  int createtime;      // file's first been found time, using time() to generate it
  int videotime;
  bool bHide;  // is hide this media?
};

struct MediaPath
{
  MediaPath() : uniqueid(0), merit(0) {}

  long long uniqueid;
  std::wstring path;
  int merit;
};

typedef std::vector<MediaData> MediaDatas;
typedef std::vector<MediaPath> MediaPaths;

// When search the media, 
// use this to judge if the media should be searched
typedef struct
{
  long long uniqueid;
  std::wstring filename;
} MediaFindCondition;

typedef struct
{
  long long uniqueid;
  std::wstring path;
  int merit;
} MediaPathCondition;

// *****************************************************************************
// Media folder tree related structures
// time_t's unit is second!!!!!!!!!
namespace media_tree {

struct root
{
};

struct file
{
  file() : tFileCreateTime(-1) {}

  MediaData file_data;
  time_t tFileCreateTime; // -1 indicate invalid
  //std::wstring sFileHash;  unused now
};

struct folder
{
  folder() : tFolderCreateTime(-1), tNextSpiderInterval(3) {}

  MediaPath folder_data;
  time_t tFolderCreateTime;  // -1 indicate invalid
  time_t tNextSpiderInterval;  // 0 indicate spider can start next loop immediate
  //std::wstring sFolderHash;  unused now

  std::list<media_tree::file> lsFiles;
};

inline bool operator<(const folder &left, const folder &right)
{
  return left.folder_data.path < right.folder_data.path;
}

} // namespace

typedef std::list<media_tree::file> MediaTreeFiles;
typedef tcl::tree<media_tree::folder> MediaTreeFolders;

// *****************************************************************************
// some helper functions
inline bool isHiddenPath(const std::wstring &sPath)
{
  DWORD dwAttr = ::GetFileAttributes(sPath.c_str());
  bool bRet = dwAttr & FILE_ATTRIBUTE_HIDDEN ? true : false;

  return bRet;
}

inline std::wstring fullFolderPath(const MediaTreeFolders *pFolder)
{
  std::wstring sCurFolderPath;
  const MediaTreeFolders *pCur = pFolder;
  while (pCur && pCur->parent())
  {
    sCurFolderPath = pCur->get()->folder_data.path + L"\\" + sCurFolderPath;

    pCur = pCur->parent();
  }

  return sCurFolderPath;
}

inline std::wstring makePathPreferred(const std::wstring &sPath)
{
  using namespace boost::filesystem;
  using namespace boost;
  using std::wstring;

  // ***************************************************************************
  // note:
  // we deal with normal path and UNC path, will erase the filename
  // ***************************************************************************
  // if it's a UNC path, then save the prefix: "\\"
  wstring sPrefix;
  if (regex_search(sPath, wregex(L"^\\\\\\\\")))
    sPrefix = L"\\\\";

  wstring sPathResult;
  if (!sPrefix.empty())
    sPathResult.assign(sPath.begin() + 2, sPath.end());
  else
    sPathResult = sPath;

  // remove the filename in the end or add backslash if the path is a directory
  if (!is_directory(sPrefix + sPathResult))
    sPathResult = regex_replace(sPathResult, wregex(L"\\\\[^\\\\]+$"), std::wstring(L"\\"));
  else
    sPathResult = regex_replace(sPathResult, wregex(L"\\\\*$"), std::wstring(L"\\"));

  // modify the path, let it to be normalized
  // replace all '/' to '\'
  sPathResult = regex_replace(sPathResult, wregex(L"/"), std::wstring(L"\\"));

  // replace multi '\' to single '\'
  sPathResult = regex_replace(sPathResult, wregex(L"\\\\+"), std::wstring(L"\\"));

  // append slash to the end if the path
  // after this, the path looks like this: "c:\cjbw1234\test\"
  if (sPathResult[sPathResult.size() - 1] != L'\\')
    sPathResult += L"\\";

  // add the prefix
  sPathResult = sPrefix + sPathResult;

  return sPathResult;
}