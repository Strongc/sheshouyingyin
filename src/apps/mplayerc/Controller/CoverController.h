#pragma once

#include <threadhelper.h>
#include "NetworkControlerImpl.h"
#include "../Model/MediaComm.h"
#include "../UserInterface/Renderer/BlockList.h"

class CoverController
: public ThreadHelperImpl<CoverController>
, public NetworkControlerImpl
{
/* Normal part */
public:
  CoverController();
  ~CoverController();

/* Properties */
public:
  void SetBlockUnit(const MediaData &md);
  void SetFrame(HWND hwnd);
  void SetListBuff(std::list<BlockUnit*>* listbuff);
  void ClearBlockUnit();

/* Operations */
public:
  void _Thread();

/* Helper functions */
protected:
  // For download
  BOOL HttpGetResponse(std::string szFileHash, std::wstring szFilePath, std::wstring requesturl, std::string& responsestr);
  BOOL ParseRespondString(std::string& parsestr, std::wstring& title, std::wstring& cover);
  BOOL HttpDownloadCover(std::string szFileHash, std::wstring downloadurl, std::wstring& downloadpath, std::wstring cover);
  std::wstring GetCoverSaveName(std::string szFileHash);
  std::wstring GetSnapshot(const MediaData &md, const std::string &szFileHash);

  // For common use
  void ChangeCover(const std::wstring &coverdownloadpath, MediaData &md, bool bUpload = true);

  // For upload
  void SetCover(BlockUnit* unit, std::wstring orgpath);
  void UploadCover(BlockUnit* unit, std::wstring url);

/* Private */
private:
  std::list<MediaData> m_list;  // store each media's media data
  std::vector<std::list<BlockUnit*>* > m_vtDualListbuff;
  HWND m_hwnd;
};