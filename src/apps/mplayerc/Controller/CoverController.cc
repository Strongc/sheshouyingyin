#include "stdafx.h"
#include "CoverController.h"
#include "HashController.h"
#include "SVPToolBox.h"
#include "Strings.h"
#include "json\json.h"
#include "logging.h"
#include "MediaCenterController.h"
#include "..\Model\MediaDB.h"

////////////////////////////////////////////////////////////////////////////////
// Normal part
CoverController::CoverController()
: m_hwnd(0)
{
  // create folder if MC folder is not existed
  MediaCenterController::CreateMCFolder();
}

CoverController::~CoverController()
{

}

////////////////////////////////////////////////////////////////////////////////
// Properties
void CoverController::SetBlockUnit(const MediaData &md)
{
  m_list.push_back(md);
}

void CoverController::SetFrame(HWND hwnd)
{
  m_hwnd = hwnd;
}

void CoverController::SetListBuff(std::list<BlockUnit*>* listbuff)
{
  std::vector<std::list<BlockUnit*>* >::iterator it = m_vtDualListbuff.begin();
  while (it != m_vtDualListbuff.end())
  {
    if (*it == listbuff)
    {
      break;
    }

    ++it;
  }

  if (it == m_vtDualListbuff.end())
  {
    if (listbuff != 0)
    {
      m_vtDualListbuff.push_back(listbuff);
    }
  }
}

void CoverController::ClearBlockUnit()
{
  m_list.clear();
}

////////////////////////////////////////////////////////////////////////////////
// Operations
void CoverController::_Thread()
{
  using namespace boost::filesystem;

  std::wstring requesturl = L"http://m.shooter.cn/api/medias/getinfoBysphash/sphash:";
  std::wstring downloadurl = L"http://img.shooter.cn/";
  std::list<MediaData>::iterator it;

  while (true)
  {
    // see if need to be stop
    if (_Exit_state(0))
      return;

    // download cover or make a snapshot for media
    it = m_list.begin();
    if (it != m_list.end())
    {
      // common
      std::wstring szFilePath = it->path + it->filename; 
      std::string szFileHash = Strings::WStringToUtf8String(HashController::GetInstance()->GetSPHash(szFilePath.c_str()));
      std::wstring sAppData;
      CSVPToolBox csvptb;
      csvptb.GetAppDataPath(sAppData);

      // if already has thumbnail then continue
      std::wstring thumbnailpath = it->thumbnailpath;
      thumbnailpath = sAppData + L"\\" + thumbnailpath;
      if (exists(thumbnailpath) && !is_directory(thumbnailpath))
      {
        ChangeCover(thumbnailpath, *it, false);
        m_list.pop_front();
        continue;
      }

      thumbnailpath = sAppData;
      thumbnailpath += L"\\mc\\cover\\";
      thumbnailpath += GetCoverSaveName(szFileHash) + L".jpg";
      if (exists(thumbnailpath) && !is_directory(thumbnailpath))
      {
        ChangeCover(thumbnailpath, *it, false);
        m_list.pop_front();
        continue;
      }

      //url = L"http://jay.webpj.com:8888/api/medias/getinfoBysphash/sphash: \
      25026521a390357bd1fcf52899268c97;c0c3ddd9b5a1c1d292131a91c9200648;cb72fdc1ff58dfc5cda9943e098c304b;e7752a7553168a73f29b0e36f09a86a8";

      // Get http response
      std::string results("");
      BOOL bGet = HttpGetResponse(szFileHash, szFilePath, requesturl, results);
      std::wstring cover;
      std::wstring coverdownloadpath;
      if (!bGet || results.empty())
      {
        // if no cover on the server, then get the snapshot by ourself
        coverdownloadpath = GetSnapshot(*it, szFileHash);
      }
      else
      {
        // Parse the respond string
        BOOL bParse = ParseRespondString(results, it->filmname, cover);
        if (!bParse || cover.empty())
        {
          // if no cover on the server, then get the snapshot by ourself
          coverdownloadpath = GetSnapshot(*it, szFileHash);
        }
        else
        {
          // Download cover
          BOOL bDownload = HttpDownloadCover(szFileHash, downloadurl, coverdownloadpath, cover);
          if (!bDownload)
          {
            // if no cover on the server, then get the snapshot by ourself
            coverdownloadpath = GetSnapshot(*it, szFileHash);
          }
        }
      }

      // Change and upload cover
      ChangeCover(coverdownloadpath, *it);

      // pop the front media data even if fail to get the cover
      m_list.pop_front();
    }

    // sleep for a while
    ::Sleep(80);
  }
}

////////////////////////////////////////////////////////////////////////////////
// Helper functions
// For common use
void CoverController::ChangeCover(const std::wstring &coverdownloadpath, MediaData &md, bool bUpload /* = true */)
{
  // Change and upload cover
  std::wstring cover;
  std::wstring::size_type nFind = coverdownloadpath.find(L"mc");
  if (nFind != std::wstring::npos)
    cover = coverdownloadpath.substr(nFind);

  if (!cover.empty())
  {
    for (size_t i = 0; i < m_vtDualListbuff.size(); ++i)
    {
      std::list<BlockUnit*>* listbuff = m_vtDualListbuff[i];
      std::list<BlockUnit*>::iterator itListBuff = listbuff->begin();
      while (itListBuff != listbuff->end())
      {
        // reset cover
        if (((*itListBuff)->m_mediadata.path == md.path)
          && ((*itListBuff)->m_mediadata.filename == md.filename))
        {
          md.thumbnailpath = cover;
          (*itListBuff)->m_mediadata.thumbnailpath = cover;

          // upload the cover
          if (bUpload)
            SetCover(*itListBuff, coverdownloadpath);

          // save thumbnail path to db
          MediaDB<>::exec(L"begin transaction");

          media_tree::model &treeModel = MediaCenterController::GetInstance()->GetMediaTree();
          treeModel.addFile((*itListBuff)->m_mediadata);
          treeModel.save2DB();
          treeModel.delTree();

          MediaDB<>::exec(L"end transaction");

          // invalidate
          CRect rc;
          rc = (*itListBuff)->GetHittest();
          InvalidateRect(m_hwnd, &rc, FALSE);
        }

        ++itListBuff;
      }
    }
  }
}

// For download
BOOL CoverController::HttpGetResponse(std::string szFileHash, std::wstring szFilePath, std::wstring requesturl, std::string& responsestr)
{
  refptr<pool> pool = pool::create_instance();
  refptr<task> task = task::create_instance();
  refptr<config> cfg = config::create_instance();
  task->use_config(cfg);
  refptr<request> req = request::create_instance();

  requesturl += Strings::Utf8StringToWString(szFileHash);

  req->set_request_url(requesturl.c_str());
  req->set_request_method(REQ_GET);
  task->append_request(req);
  pool->execute(task);

  while (pool->is_running_or_queued(task))
  {
    if (_Exit_state(500))
      return FALSE;
  }

  if (req->get_response_errcode() != 0 )
    return FALSE;

  si_buffer buff = req->get_response_buffer();
  buff.push_back(0);
  responsestr = (char*)&buff[0];
  return TRUE;
}

BOOL CoverController::ParseRespondString(std::string& parsestr, std::wstring& title, std::wstring& cover)
{
  Json::Reader reader;
  Json::Value json_object;
  if (!reader.parse(parsestr, json_object))
    return FALSE;
  std::string coverstr = json_object["mediainfo"]["cover"].asString();

  std::string titlestr;
  const Json::Value titlevalue = json_object["mediainfo"]["title"];
  for (size_t index = 0; index < titlevalue.size(); ++index )
    titlestr = titlevalue[index]["name"].asString();

  if (!titlestr.empty())
    title = Strings::Utf8StringToWString(titlestr);
  else
    title.clear();

  if (!coverstr.empty())
    cover = Strings::Utf8StringToWString(coverstr);
  else
    cover.clear();

  return TRUE;
}

BOOL CoverController::HttpDownloadCover(std::string szFileHash, std::wstring downloadurl, std::wstring& downloadpath, std::wstring cover)
{
  refptr<pool> pool = pool::create_instance();
  refptr<task> task = task::create_instance();
  refptr<config> cfg = config::create_instance();
  task->use_config(cfg);
  refptr<request> req = request::create_instance();

  std::wstring url(downloadurl);
  url += cover.substr(cover.size() - 4, 2) + L"/";
  url += cover.substr(cover.size() -2) + L"/";
  url += cover + L"_100x0.jpg";

  CSVPToolBox csvptb;
  csvptb.GetAppDataPath(downloadpath);
  downloadpath += L"\\mc\\cover\\";
  downloadpath += GetCoverSaveName(szFileHash) + L".jpg";

  req->set_request_url(url.c_str());
  req->set_request_method(REQ_GET);
  req->set_request_outmode(REQ_OUTFILE);
  req->set_outfile(downloadpath.c_str());
  task->append_request(req);
  pool->execute(task);

  while (pool->is_running_or_queued(task))
  {
    if (_Exit_state(500))
      return FALSE;
  }

  if (req->get_response_errcode() != 0 )
    return FALSE;

  if (req->get_response_size() != req->get_retrieved_size())
    return FALSE;

  return TRUE;
}

std::wstring CoverController::GetCoverSaveName(std::string szFileHash)
{
  std::wstring szJpgName = HashController::GetInstance()->GetMD5Hash(szFileHash.c_str(), szFileHash.length());
  return szJpgName;
}

std::wstring CoverController::GetSnapshot(const MediaData &md, const std::string &szFileHash)
{
  CSVPToolBox toolbox;
  std::wstring sPlayerPath = toolbox.GetPlayerPath_STL();
  std::wstring sParameters = L" /snapshot \"" + md.path + md.filename + L"\"";

  SHELLEXECUTEINFO shExecInfo = {0};
  shExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
  shExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
  shExecInfo.hwnd = 0;
  shExecInfo.lpVerb = 0;
  shExecInfo.lpFile = sPlayerPath.c_str();
  shExecInfo.lpParameters = sParameters.c_str();
  shExecInfo.lpDirectory = 0;
  shExecInfo.nShow = SW_HIDE;
  shExecInfo.hInstApp = 0;
  ::ShellExecuteEx(&shExecInfo);
  ::WaitForSingleObject(shExecInfo.hProcess, INFINITE);

  std::wstring sRet;
  toolbox.GetAppDataPath(sRet);
  sRet += L"\\mc\\cover\\";
  sRet += GetCoverSaveName(szFileHash) + L".jpg";
  return sRet;
}

// For upload
void CoverController::SetCover(BlockUnit* unit, std::wstring orgpath)
{
  using namespace boost::filesystem;
  using namespace boost::system;

  std::wstring uploadurl = L"http://zz.webpj.com:8888/api/medias/add_sfScreenshot";
  std::wstring szFilePath = unit->m_mediadata.path + unit->m_mediadata.filename;
  std::string szFileHash = Strings::WStringToUtf8String(HashController::GetInstance()->GetSPHash(szFilePath.c_str()));

  CSVPToolBox csvptb;
  std::wstring destpath;
  csvptb.GetAppDataPath(destpath);
  destpath += L"\\mc\\cover\\";
  destpath += GetCoverSaveName(szFileHash) + L".jpg";

  error_code err;
  copy_file(orgpath, destpath, copy_option::overwrite_if_exists, err);

  if (unit != 0 && (err == errc::success))
  {
    unit->m_mediadata.thumbnailpath = destpath.substr(destpath.find(L"mc"));
    unit->ResetCover();
    UploadCover(unit, uploadurl);
  }
}

void CoverController::UploadCover(BlockUnit* unit, std::wstring url)
{
  std::wstring szFilePath = unit->m_mediadata.path + unit->m_mediadata.filename; 
  std::wstring szFileHash = HashController::GetInstance()->GetSPHash(szFilePath.c_str());

  std::wstring coverPath;
  CSVPToolBox csvptb;
  csvptb.GetAppDataPath(coverPath);
  coverPath += L"\\" + unit->m_mediadata.thumbnailpath;

  refptr<pool> pool = pool::create_instance();
  refptr<task> task = task::create_instance();
  refptr<request> req = request::create_instance();
  refptr<postdata> pd = postdata::create_instance();

  req->set_request_url(url.c_str());
  req->set_request_method(REQ_POST);

  refptr<postdataelem> pelem1 = postdataelem::create_instance();
  pelem1->set_name(L"sphash");
  pelem1->setto_text(szFileHash.c_str());
  pd->add_elem(pelem1);

  refptr<postdataelem> pelem2 = postdataelem::create_instance();
  pelem2->set_name(L"img");
  pelem2->setto_file(coverPath.c_str());
  pd->add_elem(pelem2);

  req->set_postdata(pd);
  task->append_request(req);
  pool->execute(task);

  while (pool->is_running_or_queued(task))
  {
    if (_Exit_state(500))
      return;
  }

  if (req->get_response_errcode() != 0 )
    return;
}