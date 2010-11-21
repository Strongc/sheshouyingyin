#ifndef SUBTRANSCONTROLLER_H
#define SUBTRANSCONTROLLER_H


#define ID_COMPLETE_QUERY_SUBTITLE 32931

class SubTransController
{
public:
  SubTransController(void);
  ~SubTransController(void);

  typedef enum {
    Unknown,
    DownloadSubtitle,
    UploadSubtitle
  } SubTransOperation;

  typedef std::vector<std::wstring> StringList;

  // set frame window HWND
  void SetFrame(HWND hwnd);

  void SetSubfile(std::wstring subfile);
  void SetDelayMs(int ms);
  void SetOemTitle(std::wstring str);
  void SetSubperf(std::wstring str);
  void SetLanuage(std::wstring str);

  void SetMsgs(std::list<std::wstring>* msgs);
  // starting and ending main thread for upload / download
  void Start(const wchar_t* video_filename, SubTransOperation operation,
             StringList files_upload = StringList());
  void Stop();

  // primary thread logics, should not be called directly
  static void _thread_dispatch(void* param);
  void _thread();

private:
  void _thread_download();
  void _thread_upload();

  SubTransOperation m_operation;

  HWND    m_frame;
  HANDLE  m_thread;
  HANDLE  m_stopevent;

  std::wstring m_subfile;
  std::wstring m_videofile;
  int m_delayms;

  std::wstring m_oemtitle;
  std::wstring m_subperf;
  std::wstring m_language;

  std::list<std::wstring>* m_handlemsgs;
};
#endif // SUBTRANSCONTROLLER_H