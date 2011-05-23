#include "stdafx.h"
#include "CoverUploadController.h"
#include "HashController.h"
#include "SVPToolBox.h"
#include "Strings.h"

CoverUploadController::CoverUploadController(void)
{
}

CoverUploadController::~CoverUploadController(void)
{
}

void CoverUploadController::SetFrame(HWND hwnd)
{
  m_hwnd = hwnd;
}

void CoverUploadController::SetCover(BlockUnit* unit, std::wstring orgpath)
{
  std::wstring destpath;
  BOOL bsuccess;

  std::wstring szFilePath = unit->m_mediadata.path + unit->m_mediadata.filename;
  std::string szFileHash = Strings::WStringToUtf8String(HashController::GetInstance()->GetSPHash(szFilePath.c_str()));

  CSVPToolBox csvptb;
  csvptb.GetAppDataPath(destpath);
  destpath += L"\\mc\\cover\\";
  destpath += GetCoverNameString(szFileHash) + L".jpg";
  bsuccess = ::CopyFile(orgpath.c_str(), destpath.c_str(), TRUE);

  if (unit != 0 && bsuccess)
  {
    unit->m_mediadata.thumbnailpath = destpath.substr(destpath.find(L"mc"));
    unit->ResetCover();
    m_list.push_back(unit);
    _Start();
  }
}

std::wstring CoverUploadController::GetCoverNameString(std::string szFileHash)
{
  std::wstring szJpgName = HashController::GetInstance()->GetMD5Hash(szFileHash.c_str(), szFileHash.length());
  return szJpgName;
}

void CoverUploadController::_Thread()
{
  std::wstring uploadurl = L"http://zz.webpj.com:8888/api/medias/add_sfScreenshot";
  std::list<BlockUnit*>::iterator it = m_list.begin();
  
  while(it != m_list.end())
  {
    if (_Exit_state(0))
      return;

    UploadCover(*it, uploadurl);

    m_list.pop_front();
    it = m_list.begin();
  }
}

void CoverUploadController::UploadCover(BlockUnit* unit, std::wstring url)
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