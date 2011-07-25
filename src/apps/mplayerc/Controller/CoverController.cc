#include "stdafx.h"
#include "CoverController.h"
#include "HashController.h"
#include "SVPToolBox.h"
#include "Strings.h"
#include "json\json.h"
#include "logging.h"
#include "MediaCenterController.h"
#include "..\Model\MediaDB.h"
#include "PlayerPreference.h"
#include "SPlayerDefs.h"

////////////////////////////////////////////////////////////////////////////////
// Normal part
CoverController::CoverController()
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

////////////////////////////////////////////////////////////////////////////////
// Operations
void CoverController::_Thread()
{
  using namespace boost::filesystem;

  std::wstring requesturl = PlayerPreference::GetInstance()->GetStringVar(STRVAR_COVER_REQUESTURL);
  std::wstring downloadurl = PlayerPreference::GetInstance()->GetStringVar(STRVAR_COVER_DOWNLOADURL);

  UINT uniqueid;
  std::wstring path;
  std::wstring filename;
  wchar_t execsql[500];

  wchar_t* updatesql = L"UPDATE media_data SET thumbnailpath='%s', hash='%s' WHERE uniqueid=%u";
  wchar_t* selectsql = L"SELECT uniqueid, path, filename FROM media_data WHERE hash='' limit 0,1";

  while (true)
  {
    path = L"";
    uniqueid = 0;
    filename = L"";

    MediaDB<UINT, std::wstring, std::wstring>::exec(selectsql, &uniqueid, &path, &filename);
    if (!uniqueid || filename.empty())
    {
      if (_Exit_state(15 * 1000))
        return;
      continue;
    }

    std::wstring filmfile = path + filename;
    std::wstring coverpath = GetSnapshot(filmfile);
    if (exists(coverpath))
    {
      memset(execsql, 0, 500);
      std::wstring coverhash = MediaCenterController::GetMediaHash(filmfile);
      wsprintf(execsql, updatesql, coverpath.c_str(), coverhash.c_str(), uniqueid);
      MediaDB<>::exec(execsql);
    }

    if (_Exit_state(5 * 1000))
      return;
  }
}

////////////////////////////////////////////////////////////////////////////////
// Helper functions
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

BOOL CoverController::HttpDownloadCover(std::wstring downloadurl, std::wstring& downloadpath, std::wstring cover)
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

std::wstring CoverController::GetSnapshot(const std::wstring &file)
{
  CSVPToolBox toolbox;
  std::wstring sPlayerPath = toolbox.GetPlayerPath_STL();
  std::wstring sParameters = L" /snapshot \"" + file + L"\" 128_128 5";
  std::wstring format = L"128_128_5";

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
  BOOL bRet = ::ShellExecuteEx(&shExecInfo);
  if (bRet)
  {
    // waiting for the snapshot process
    HANDLE hHandles[] = {m_stopevent, shExecInfo.hProcess};
    DWORD dwRet = ::WaitForMultipleObjects(2, hHandles, FALSE, 30000);  // wait for max 30s

    switch (dwRet)
    {
    case WAIT_FAILED:
      Logging(L"wait for snapshot process failed!");
      break;

    case WAIT_TIMEOUT:
      {
          // terminate the snapshot process
          ::TerminateProcess(shExecInfo.hProcess, -1);
      }
      break;

    default:
      {
        if (_Exit_state(0))  // stop event is triggered
        {
          // terminate the snapshot process
          ::TerminateProcess(shExecInfo.hProcess, -1);
        }
      }
    }

    ::CloseHandle(shExecInfo.hProcess);
  }

  std::wstring sRet = MediaCenterController::GetCoverPath(file, format);

  return sRet;
}

// For upload
void CoverController::UploadCover(const MediaData &md)
{
  std::wstring url = PlayerPreference::GetInstance()->GetStringVar(STRVAR_COVER_UPLOADURL);

  std::wstring szFilePath = md.path + md.filename; 
  std::wstring szFileHash = HashController::GetInstance()->GetSPHash(szFilePath.c_str());

  std::wstring coverPath;
  CSVPToolBox csvptb;
  csvptb.GetAppDataPath(coverPath);
  coverPath += L"\\" + md.thumbnailpath;

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