#pragma once

#include <string>

// Load resource from disk or exe, return a handle to the caller
class ResLoader
{
public:
  ResLoader();
  ~ResLoader();

public:
  // Automatically choose a load function below
  HBITMAP LoadBitmap(const std::wstring& sBitmapPath,
    const std::wstring& sResModuleName = L"");

  // sBitmapPath is a relative or absolute path
  // if bPathRelToSplayer is true, the sBitmapPath is a relative path rel to the splayer.exe
  // if bPathRelToSplayer is false, the sBitmapPath is a absolute path
  HBITMAP LoadBitmapFromDisk(const std::wstring& sBitmapPath, bool bPathRelToSplayer = true);

  // sBitmapName is the resource name in the module
  // sResModuleName is the resource module name, L"" indicate it's the exe
  HBITMAP LoadBitmapFromModule(const std::wstring& sBitmapName,
    const std::wstring& sResModuleName = L"");
};
