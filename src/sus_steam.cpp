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

#include "Resource.h"
#include <Windows.h>
#include <windowsx.h>

#include "parameter.h"
#include "ds3t.h"

class suscfg_Steam
{
public:
  suscfg_Steam (void);

  bool setup_ui  (HWND hDlg);

//protected:
  ds3t::ParameterBool*    no_sound;
  ds3t::ParameterBool*    take_screenshot;
  ds3t::ParameterStringW* sound_file;

//private:
  HWND hWndPlaySound;
  HWND hWndTakeScreenshot;
} *steam = nullptr;

suscfg_Steam::suscfg_Steam (void)
{
  ds3t::INI::File* sus_ini  = config.get_file_sus  ();
  ds3t::INI::File* dxgi_ini = config.get_file_dxgi ();

  no_sound = static_cast <ds3t::ParameterBool *> (
    ds3t::g_ParameterFactory.create_parameter <bool> (
      L"Do Not Play Sound on Achievement Unlock"
    )
  );
  no_sound->register_to_ini ( dxgi_ini,
                                L"Steam.Achievements",
                                  L"NoSound" );

  take_screenshot = static_cast <ds3t::ParameterBool *> (
    ds3t::g_ParameterFactory.create_parameter <bool> (
      L"Take Screenshot on Achievement Unlock"
    )
  );
  take_screenshot->register_to_ini ( dxgi_ini,
                                       L"Steam.Achievements",
                                         L"TakeScreenshot" );

  sound_file = static_cast <ds3t::ParameterStringW *> (
    ds3t::g_ParameterFactory.create_parameter <std::wstring> (
      L"Sound to Play"
    )
  );
  sound_file->register_to_ini ( dxgi_ini,
                                  L"Steam.Achievements",
                                    L"SoundFile" );

  no_sound->load         ();
  take_screenshot->load  ();
  sound_file->load       ();
}

bool
suscfg_Steam::setup_ui (HWND hDlg)
{
  hWndPlaySound       = GetDlgItem (hDlg, IDC_SUS_STEAM_PLAY_SOUND);
  hWndTakeScreenshot  = GetDlgItem (hDlg, IDC_SUS_STEAM_TAKE_SCREENSHOT);

  no_sound->bind_to_control (new ds3t::UI::CheckBox (hWndPlaySound));
  no_sound->set_value       (! no_sound->get_value ());

  take_screenshot->bind_to_control (new ds3t::UI::CheckBox (hWndTakeScreenshot));
  take_screenshot->set_value       (take_screenshot->get_value ());

  return true;
}

HRESULT SelectSoundFile (void)
{
  IFileDialog *pfd = nullptr;

  HRESULT hr =
    CoCreateInstance ( CLSID_FileOpenDialog,
                         NULL,
                           CLSCTX_INPROC_SERVER,
                             IID_PPV_ARGS (&pfd) );

  if (SUCCEEDED (hr)) {
    IFileDialogEvents *pfde = NULL;

    hr = CDialogEventHandler_CreateInstance (IID_PPV_ARGS (&pfde));

    if (SUCCEEDED (hr)) {
      DWORD dwCookie;

      hr = pfd->Advise (pfde, &dwCookie);
      if (SUCCEEDED (hr)) {
        DWORD dwFlags;

        hr = pfd->GetOptions (&dwFlags);

        if (SUCCEEDED (hr)) {
          hr = pfd->SetOptions (dwFlags | FOS_FORCEFILESYSTEM);

          if (SUCCEEDED (hr)) {
            COMDLG_FILTERSPEC rgSpec [] = {
                { L"PCM Audio (.wav)", L"*.wav;*.wave" },
            };

            hr = pfd->SetFileTypes (ARRAYSIZE (rgSpec), rgSpec);

            if (SUCCEEDED (hr)) {
              hr = pfd->SetFileTypeIndex (1);

              if (SUCCEEDED (hr)) {
                hr = pfd->SetDefaultExtension (L"wav;wave");

                if (SUCCEEDED (hr)) {
                  hr = pfd->Show (NULL);

                  if (SUCCEEDED (hr)) {
                    IShellItem *psiResult;

                    hr = pfd->GetResult(&psiResult);

                    if (SUCCEEDED (hr)) {
                      PWSTR pszFilePath = NULL;

                      hr =
                        psiResult->GetDisplayName ( SIGDN_FILESYSPATH,
                                                      &pszFilePath );

                      if (SUCCEEDED (hr)) {
#if 0
                        TaskDialog ( NULL,
                                     NULL,
                                     L"CommonFileDialogApp",
                                     pszFilePath,
                                     NULL,
                                     TDCBF_OK_BUTTON,
                                     TD_INFORMATION_ICON,
                                     NULL);
#else
                        steam->sound_file->set_value (pszFilePath);
#endif
                        CoTaskMemFree (pszFilePath);
                      }

                      psiResult->Release ();
                    }
                  }
                }
              }
            }
          }
        }

        pfd->Unadvise (dwCookie);
      }

      pfde->Release ();
    }

    pfd->Release ();
  }

  return hr;
}


INT_PTR
CALLBACK
SteamConfig (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch (message)
  {
    case WM_INITDIALOG:
    {
      if (steam == nullptr) {
        steam = new suscfg_Steam ();
      }

      steam->setup_ui (hDlg);
    } break;

    case WM_COMMAND:
    {
      if (LOWORD(wParam) == IDC_ACHIEVEMENT_SOUND)
      {
        SelectSoundFile ();
      }

      if (LOWORD (wParam) == IDOK)
      {
        bool check = Button_GetCheck (steam->hWndPlaySound);
        steam->no_sound->set_value (! check);

        check = Button_GetCheck (steam->hWndTakeScreenshot);
        steam->take_screenshot->set_value (check);

        steam->no_sound->store         ();
        steam->take_screenshot->store  ();
        steam->sound_file->store       ();

        EndDialog (hDlg, LOWORD (wParam));
        return (INT_PTR)TRUE;
      }
    } break;
  }

  return (INT_PTR)FALSE;
}