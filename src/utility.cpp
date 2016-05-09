/**
 * This file is part of Dark Souls 3 Tweak.
 *
 * Dark Souls 3 Tweak is free software : you can redistribute it and /
 * or modify it under the terms of the GNU General Public License as
 * published by The Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * Dark Souls 3 Tweak is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 *     See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Dark Souls 3 Tweak.
 *
 *    If no license was present, see <http://www.gnu.org/licenses/>.
**/

#include "utility.h"

#include <UserEnv.h>
#pragma comment (lib, "userenv.lib")

#include <Shlobj.h>
#pragma comment (lib, "shell32.lib")

int
DS3T_MessageBox (std::wstring caption, std::wstring title, uint32_t flags)
{
  extern bool messagebox_active;
  extern HWND hWndApp;

  HWND parent = IsWindow (hWndApp) ? hWndApp : NULL;

  messagebox_active = true;

  int ret = MessageBox (hWndApp, caption.c_str (), title.c_str (), flags | MB_SYSTEMMODAL | MB_TOPMOST | MB_SETFOREGROUND);

  messagebox_active = false;

  return ret;
}

std::wstring
DS3T_GetDocumentsDir (void)
{
  HANDLE hToken;

  if (!OpenProcessToken (GetCurrentProcess (), TOKEN_READ, &hToken))
    return NULL;

  wchar_t* str;
  SHGetKnownFolderPath (FOLDERID_Documents, 0, hToken, &str);

    std::wstring ret = str;

  CoTaskMemFree (str);

  return ret;
}

BOOL
DS3T_GetUserProfileDir (wchar_t* buf, DWORD* pdwLen)
{
  HANDLE hToken;

  if (! OpenProcessToken (GetCurrentProcess (), TOKEN_READ, &hToken))
    return FALSE;

  if (! GetUserProfileDirectory (hToken, buf, pdwLen))
    return FALSE;

  CloseHandle (hToken);
  return TRUE;
}

std::wstring
DS3T_GetRoamingAppDataDir (void)
{
  HANDLE hToken;

  if (! OpenProcessToken (GetCurrentProcess (), TOKEN_READ, &hToken))
    return NULL;

  wchar_t* str;
  SHGetKnownFolderPath (FOLDERID_RoamingAppData, 0, hToken, &str);

    std::wstring ret = str;

  CoTaskMemFree (str);

  return ret;
}

#include <string>

bool
DS3T_IsTrue (const wchar_t* string)
{
  if (std::wstring (string).length () == 1 &&
    string [0] == L'1')
    return true;

  if (std::wstring (string).length () != 4)
    return false;

  if (towlower (string [0]) != L't')
    return false;
  if (towlower (string [1]) != L'r')
    return false;
  if (towlower (string [2]) != L'u')
    return false;
  if (towlower (string [3]) != L'e')
    return false;

  return true;
}

bool
DS3T_IsAdmin (void)
{
  bool   bRet   = false;
  HANDLE hToken = 0;

  if (OpenProcessToken (GetCurrentProcess (), TOKEN_QUERY, &hToken)) {
    TOKEN_ELEVATION Elevation;
    DWORD cbSize = sizeof (TOKEN_ELEVATION);

    if (GetTokenInformation (hToken, TokenElevation, &Elevation, sizeof (Elevation), &cbSize)) {
      bRet = Elevation.TokenIsElevated != 0;
    }
  }

  if (hToken)
    CloseHandle (hToken);

  return bRet;
}

void
DS3T_DeleteAllConfigFiles (void)
{
  // Strip Read-Only
  DS3T_SetNormalFileAttribs (std::wstring (DS3T_GetRoamingAppDataDir () + L"\\DarkSoulsIII\\GraphicsConfig.XML"));

  DeleteFile (std::wstring (DS3T_GetRoamingAppDataDir () + L"\\DarkSoulsIII\\GraphicsConfig.XML").c_str ());
//  DeleteFile (std::wstring (DS3T_GetSteamUserDataDir  () + L"\\351970\\remote\\WinData.xml").c_str ());
}

bool
DS3T_HasBackupConfigFiles (void)
{
  WIN32_FIND_DATA FindFileData;

  if (FindFirstFile ( std::wstring ( DS3T_GetRoamingAppDataDir () +
                                     L"\\DarkSoulsIII\\GraphicsConfig.DS3T" ).c_str (),
                        &FindFileData ) != INVALID_HANDLE_VALUE)
    return true;

  return false;
}

// Copies a file preserving file times
void
DS3T_FullCopy (std::wstring from, std::wstring to)
{
  // Strip Read-Only
  DS3T_SetNormalFileAttribs (to);
  DeleteFile (to.c_str ());
  CopyFile   (from.c_str (), to.c_str (), FALSE);

  WIN32_FIND_DATA FromFileData;
  HANDLE hFrom = FindFirstFile (from.c_str (), &FromFileData);

  OFSTRUCT ofTo;
  ofTo.cBytes = sizeof (OFSTRUCT);

  char     szFileTo [MAX_PATH];

  WideCharToMultiByte (CP_OEMCP, 0, to.c_str (), -1, szFileTo, MAX_PATH, NULL, NULL);
  HFILE hfTo = OpenFile (szFileTo, &ofTo, NULL);

  CloseHandle ((HANDLE)hfTo);

  // Here's where the magic happens, apply the attributes from the original file to the new one!
  SetFileTime ((HANDLE)hfTo, &FromFileData.ftCreationTime, &FromFileData.ftLastAccessTime, &FromFileData.ftLastWriteTime);

  FindClose   (hFrom);
}

void
DS3T_CreateBackupConfig (void)
{
  DS3T_FullCopy ( std::wstring (DS3T_GetRoamingAppDataDir () + L"\\DarkSoulsIII\\GraphicsConfig.XML"),
                  std::wstring (DS3T_GetRoamingAppDataDir () + L"\\DarkSoulsIII\\GraphicsConfig.DS3T") );

  //DS3T_FullCopy ( std::wstring (DS3T_GetSteamUserDataDir () + L"\\351970\\remote\\WinData.xml"),
                  //std::wstring (DS3T_GetSteamUserDataDir () + L"\\351970\\remote\\WinData.tzt") );
}

void
DS3T_RestoreConfigFiles (void)
{
  DS3T_FullCopy ( std::wstring (DS3T_GetRoamingAppDataDir () + L"\\DarkSoulsIII\\GraphicsConfig.DS3T"),
                  std::wstring (DS3T_GetRoamingAppDataDir () + L"\\DarkSoulsIII\\GraphicsConfig.XML") );

  //DS3T_FullCopy ( std::wstring (DS3T_GetSteamUserDataDir () + L"\\351970\\remote\\WinData.tzt"),
                  //std::wstring (DS3T_GetSteamUserDataDir () + L"\\351970\\remote\\WinData.xml") );

  // Strip Read-Only
  DS3T_SetNormalFileAttribs (std::wstring (DS3T_GetRoamingAppDataDir () + L"\\DarkSoulsIII\\GraphicsConfig.DS3T"));

  DeleteFile (std::wstring (DS3T_GetRoamingAppDataDir () + L"\\DarkSoulsIII\\GraphicsConfig.DS3T").c_str ());
}

// Gets the timestamp on the current backup file
std::wstring
DS3T_GetBackupFileTime (void)
{
  WIN32_FIND_DATA FindFileData;

  HANDLE hFileBackup =
    FindFirstFile (std::wstring (DS3T_GetRoamingAppDataDir () + L"\\DarkSoulsIII\\GraphicsConfig.DS3T").c_str (), &FindFileData);

  FindClose (hFileBackup);

  FILETIME   ftModified;
  FileTimeToLocalFileTime (&FindFileData.ftLastWriteTime, &ftModified);

  SYSTEMTIME stModified;
  FileTimeToSystemTime (&ftModified, &stModified);

  wchar_t wszFileTime [512];

  GetDateFormat (LOCALE_CUSTOM_UI_DEFAULT, DATE_AUTOLAYOUT, &stModified, NULL, wszFileTime, 512);

  std::wstring date_time = wszFileTime;

  GetTimeFormat (LOCALE_CUSTOM_UI_DEFAULT, TIME_NOSECONDS, &stModified, NULL, wszFileTime, 512);

  date_time += L" ";
  date_time += wszFileTime;

  return date_time;
}

// Gets the timestamp on the current config file
std::wstring
DS3T_GetConfigFileTime (void)
{
  WIN32_FIND_DATA FindFileData;

  HANDLE hFileBackup =
    FindFirstFile ( std::wstring ( DS3T_GetRoamingAppDataDir () +
                                   L"\\DarkSoulsIII\\GraphicsConfig.XML" ).c_str (),
                      &FindFileData );

  FILETIME   ftModified;
  FileTimeToLocalFileTime (&FindFileData.ftLastWriteTime, &ftModified);

  SYSTEMTIME stModified;
  FileTimeToSystemTime (&ftModified, &stModified);

  FindClose (hFileBackup);

  wchar_t wszFileTime [512];

  GetDateFormat (LOCALE_CUSTOM_UI_DEFAULT, DATE_AUTOLAYOUT, &stModified, NULL, wszFileTime, 512);

  std::wstring date_time = wszFileTime;

  GetTimeFormat (LOCALE_CUSTOM_UI_DEFAULT, TIME_NOSECONDS, &stModified, NULL, wszFileTime, 512);

  date_time += L" ";
  date_time += wszFileTime;

  return date_time;
}

void
DS3T_SetNormalFileAttribs (std::wstring file)
{
  SetFileAttributes (file.c_str (), FILE_ATTRIBUTE_NORMAL);
}



std::wstring
DS3T_ErrorMessage (errno_t        err,
                   const char*    args,
                   const wchar_t* ini_name,
                   UINT           line_no,
                   const char*    function_name,
                   const char*    file_name)
{
  wchar_t wszFile           [256];
  wchar_t wszFunction       [256];
  wchar_t wszArgs           [256];
  wchar_t wszFormattedError [1024];

  MultiByteToWideChar (CP_OEMCP, 0, file_name,     -1, wszFile,     256);
  MultiByteToWideChar (CP_OEMCP, 0, function_name, -1, wszFunction, 256);
  MultiByteToWideChar (CP_OEMCP, 0, args,          -1, wszArgs,     256);
  *wszFormattedError = L'\0';

  swprintf (wszFormattedError, 1024,
    L"Line %u of %s (in %s (...)):\n"
    L"------------------------\n\n"
    L"%s\n\n  File: %s\n\n"
    L"\t>> %s <<",
    line_no,
    wszFile,
    wszFunction,
    wszArgs,
    ini_name,
    _wcserror (err));

  return wszFormattedError;
}

#define TRY_FILE_IO(x,y,z) { (z) = ##x; if ((z) != 0) MessageBox (NULL, DS3T_ErrorMessage ((z), #x, (y), __LINE__, __FUNCTION__, __FILE__).c_str (), L"File I/O Error", MB_OK | MB_ICONSTOP ); }

struct ds3_cfg_t
{
  int res_x,
    res_y,
    refresh;

  bool fullscreen;
  bool vsync;

  void init (void) {
    std::wstring fname = DS3T_GetRoamingAppDataDir ();

    fname += L"\\DarkSoulsIII";
    CreateDirectory (fname.c_str (), NULL);

    fname += L"\\GraphicsConfig.XML";

    //printf ("Writing configuration to '%ls'... ", fname.c_str ());

    FILE*   fCFG = nullptr;
    errno_t err;

    char fname_s [MAX_PATH];
    sprintf (fname_s, "%ls", fname.c_str ());

    //
    // If the config file cannot be opened, then create a new one.
    //
    if (! (fCFG = fopen (fname_s, "r"))) {
      DeleteFile (fname.c_str ());

#if 0
      TRY_FILE_IO ( fopen_s (&fCFG, fname_s, "a"),
        fname.c_str (),
        err );

      if (err == 0 && fCFG != nullptr) {
        fprintf (fCFG, "resolution_X=%d\n"
          "resolution_Y=%d\n"
          "refreshRateHz=%d\n"
          "fullscreen=%d\n"
          "vsync=%d\n"
          "%hs",
          res_x, res_y, refresh, fullscreen, vsync,
          everything_else);
        fclose (fCFG);
      }
#endif
    }
    else {
      fclose (fCFG);
    }
  }
} cfg;

void
DS3T_InitializeConfig (void)
{
  DEVMODE dmNow;

  memset (&dmNow, 0, sizeof (DEVMODE));
  dmNow.dmSize = sizeof (DEVMODE);

  EnumDisplaySettings (NULL, ENUM_CURRENT_SETTINGS, &dmNow);

  cfg.fullscreen = 0;
  cfg.vsync      = 1;
  cfg.res_x      = dmNow.dmPelsWidth;
  cfg.res_y      = dmNow.dmPelsHeight;
  cfg.refresh    = dmNow.dmDisplayFrequency;

  cfg.init ();
}

std::wstring
DS3T_GetSteamDir (void)
{
  DWORD len = MAX_PATH;
  wchar_t wszSteamPath [MAX_PATH];

  RegGetValueW ( HKEY_CURRENT_USER,
                   L"SOFTWARE\\Valve\\Steam\\",
                     L"SteamPath",
                       RRF_RT_REG_SZ,
                         NULL,
                           wszSteamPath,
                             (LPDWORD)&len );

  return wszSteamPath;
}

std::wstring
DS3T_GetSteamExecutable (void)
{
  return DS3T_GetSteamDir () + L"\\Steam.exe";
}

std::wstring
DS3T_GetSteamUIDLL (void)
{
  std::wstring steam_path =
    DS3T_GetSteamDir ();

  steam_path += L"\\SteamUI.dll";

  return steam_path;
}

std::wstring
DS3T_GetSteamUserDataDir (void)
{
  std::wstring steam_path =
    DS3T_GetSteamDir ();

  DWORD len = wcslen (DS3T_GetSteamDir ().c_str ());
  wchar_t wszSteamPath [MAX_PATH] = { L'\0' };

  wcsncpy (wszSteamPath, steam_path.c_str (), len);

  WIN32_FIND_DATA find_data;
  HANDLE          hFind;

  for (int i = 0; i < len; i++) {
    if (wszSteamPath [i] == L'/')
      wszSteamPath [i] = L'\\';
  }

  wchar_t wszPath [MAX_PATH];
  wsprintf (wszPath, L"%s\\userdata\\*", wszSteamPath);

  if (hFind = FindFirstFile (wszPath, &find_data)) {
    do
    {
      if (wcslen (find_data.cFileName) > 2) {
        std::wstring ret (wszSteamPath);
        ret += L"\\userdata\\";
        ret += find_data.cFileName;
        return ret;
      }
    } while (FindNextFile (hFind, &find_data));
    FindClose (hFind);
  }

  return L"<No Such Directory>";
}



#include <tlhelp32.h>

PROCESSENTRY32
FindProcessByName (const wchar_t* wszName)
{
  HANDLE         hProcessSnap;
  PROCESSENTRY32 pe32;

  hProcessSnap = CreateToolhelp32Snapshot (TH32CS_SNAPPROCESS, 0);

  if (hProcessSnap == INVALID_HANDLE_VALUE)
    return pe32;

  pe32.dwSize = sizeof (PROCESSENTRY32);

  if (! Process32First (hProcessSnap, &pe32))
  {
    CloseHandle (hProcessSnap);
    return pe32;
  }

  do
  {
    if (wcsstr (pe32.szExeFile, L"RTSS.exe"))
      return pe32;
  } while (Process32Next (hProcessSnap, &pe32));

  CloseHandle (hProcessSnap);
  return pe32;
}

std::wstring
DS3T_GetRTSSInstallDir (void)
{
  PROCESSENTRY32 pe32 = FindProcessByName (L"RTSS.exe");

  wchar_t wszPath [MAX_PATH] = { '\0' };

  if (wcsstr (pe32.szExeFile, L"RTSS.exe")) {
    HANDLE hProcess = OpenProcess (PROCESS_QUERY_LIMITED_INFORMATION , FALSE, pe32.th32ProcessID);

    DWORD len = MAX_PATH;
    QueryFullProcessImageName (hProcess, 0, wszPath, &len);

    *(wcsstr (wszPath, L"RTSS.exe")) = L'\0';

    CloseHandle (hProcess);
  }

  return wszPath;
}


// Instance creation helper
HRESULT
CDialogEventHandler_CreateInstance (REFIID riid, void **ppv)
{
  *ppv = NULL;

  CDialogEventHandler *pDialogEventHandler =
    new (std::nothrow) CDialogEventHandler ();

  HRESULT hr = pDialogEventHandler ? S_OK : E_OUTOFMEMORY;

  if (SUCCEEDED (hr))
  {
    hr = pDialogEventHandler->QueryInterface (riid, ppv);
    pDialogEventHandler->Release ();
  }

  return hr;
}