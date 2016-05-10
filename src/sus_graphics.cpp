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

class suscfg_Graphics
{
public:
  suscfg_Graphics (void);

  bool setup_ui  (HWND hDlg);

//protected:
  ds3t::ParameterBool* stretch_hud;

//private:
  HWND hWndAspectRatioCorrection;
  HWND hWndStretchHUD;
  HWND hWndCenterWindow;
} *graphics = nullptr;

suscfg_Graphics::suscfg_Graphics (void)
{
  ds3t::INI::File* sus_ini  = config.get_file_sus  ();
  ds3t::INI::File* dxgi_ini = config.get_file_dxgi ();

  stretch_hud = static_cast <ds3t::ParameterBool *> (
    ds3t::g_ParameterFactory.create_parameter <bool> (
      L"Stretch the HUD to correct visual glitches"
    )
  );
  stretch_hud->register_to_ini ( sus_ini,
                                   L"SUS.Display",
                                     L"StretchHUD" );

  aspect_ratio_correction->load ();
  stretch_hud->load             ();
}

bool
suscfg_Graphics::setup_ui (HWND hDlg)
{
  hWndAspectRatioCorrection = GetDlgItem (hDlg, IDC_SUS_GRAPHICS_ARC);
  hWndStretchHUD            = GetDlgItem (hDlg, IDC_SUS_STRETCH_HUD);
  hWndCenterWindow          = GetDlgItem (hDlg, IDC_SUS_CENTER_WINDOW);

  stretch_hud->bind_to_control (new ds3t::UI::CheckBox (hWndStretchHUD));
  stretch_hud->set_value (stretch_hud->get_value ());

  center_window->bind_to_control (new ds3t::UI::CheckBox (hWndCenterWindow));
  center_window->set_value (center_window->get_value ());

  ComboBox_ResetContent (hWndAspectRatioCorrection);

  ComboBox_InsertString (hWndAspectRatioCorrection, 0, L"Off");
  ComboBox_InsertString (hWndAspectRatioCorrection, 1, L"On");

  int ar_sel;

#if 0
  if (aspect_correct_videos->get_value () &&
      aspect_correct_ui->get_value     ())
    ar_sel = 3;
  else if (aspect_correct_videos->get_value ())
    ar_sel = 1;
  else if (aspect_correct_ui->get_value ())
    ar_sel = 2;
  else
    ar_sel = 0;
#else
  if (aspect_ratio_correction->get_value ())
    ar_sel = 1;
  else
    ar_sel = 0;
#endif

  ComboBox_SetCurSel (hWndAspectRatioCorrection, ar_sel);

  Button_Enable (hWndStretchHUD, aspect_ratio_correction->get_value ());

  return true;
}

INT_PTR
CALLBACK
GraphicsConfig (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch (message)
  {
    case WM_INITDIALOG:
    {
      if (graphics == nullptr) {
        graphics = new suscfg_Graphics ();
      }

      graphics->setup_ui (hDlg);
    } break;

    case WM_COMMAND:
    {
      if (LOWORD (wParam) == IDC_SUS_GRAPHICS_ARC) {
        if (HIWORD (wParam) == CBN_SELCHANGE) {
          int ar_sel = ComboBox_GetCurSel (graphics->hWndAspectRatioCorrection);
          Button_Enable (graphics->hWndStretchHUD, ar_sel >= 1);
          aspect_ratio_correction->set_value (ar_sel >= 1);

          extern HWND hDlgMain;
          HWND hWndFullscreen = GetDlgItem (hDlgMain, IDC_FULLSCREEN);

          if (ar_sel >= 1) {
            extern int  mode;
            extern void handle_window_radios (HWND hDlg, WORD ID);

            if (mode == 2) {
              mode = 1;
              handle_window_radios (hDlgMain, IDC_BORDERLESS_WINDOW);
            }
          }

          // We cannot allow fullscreen mode if aspect ratio correction is enabled
          Button_Enable (hWndFullscreen, ar_sel == 0);
        }
      }
      if (LOWORD (wParam) == IDOK)
      {
        aspect_ratio_correction->store ();
        graphics->stretch_hud->store   ();
        center_window->store           ();

        EndDialog (hDlg, LOWORD (wParam));
        return (INT_PTR)TRUE;
      }
    } break;
  }

  return (INT_PTR)FALSE;
}