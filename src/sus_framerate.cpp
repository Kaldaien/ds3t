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

class suscfg_Framerate
{
public:
  suscfg_Framerate (void);

  bool setup_ui  (HWND hDlg);

  int  poll_prerender_limit (HWND hDlg);
  int  poll_framerate_limit (HWND hDlg);

  //protected:
  ds3t::ParameterInt*   fps;
  ds3t::ParameterBool*  minimize_latency;

  ds3t::ParameterInt*   backbuffer_count;
  ds3t::ParameterInt*   prerender_limit;

  //private:
  HWND hWndFPSPreset;
  HWND hWndFPSCustom;
  HWND hWndMinimizeLatency;
  HWND hWndTripleBuffering;
  HWND hWndPreRenderLimit;
} *framerate = nullptr;

suscfg_Framerate::suscfg_Framerate (void)
{
  ds3t::INI::File* sus_ini  = config.get_file_sus  ();
  ds3t::INI::File* dxgi_ini = config.get_file_dxgi ();

  minimize_latency = static_cast <ds3t::ParameterBool *> (
    ds3t::g_ParameterFactory.create_parameter <bool> (
      L"Minimize Input Latency"
      )
    );
  minimize_latency->register_to_ini ( sus_ini,
    L"TZFIX.FrameRate",
      L"MinimizeLatency" );

  fps = static_cast <ds3t::ParameterInt *> (
    ds3t::g_ParameterFactory.create_parameter <int> (
      L"Target Framerate"
    )
  );
  fps->register_to_ini ( dxgi_ini,
    L"Render.DXGI",
      L"TargetFPS" );

  backbuffer_count = static_cast <ds3t::ParameterInt *> (
    ds3t::g_ParameterFactory.create_parameter <int> (
      L"Backbuffer Count"
    )
  );
  backbuffer_count->register_to_ini (dxgi_ini,
    L"Render.DXGI",
      L"SwapChainBufferCount" );

  prerender_limit = static_cast <ds3t::ParameterInt *> (
    ds3t::g_ParameterFactory.create_parameter <int> (
      L"Pre-Rendered Frame Limit"
    )
  );
  prerender_limit->register_to_ini (dxgi_ini,
    L"Render.DXGI",
      L"PreRenderLimit" );

  fps->load              ();

  minimize_latency->load ();

  backbuffer_count->load ();
  prerender_limit->load  ();
}

bool
suscfg_Framerate::setup_ui (HWND hDlg)
{
  hWndFPSPreset        = GetDlgItem (hDlg, IDC_TARGET_FPS);
  hWndFPSCustom        = GetDlgItem (hDlg, IDC_CUSTOM_FPS);
  hWndMinimizeLatency  = GetDlgItem (hDlg, IDC_SUS_FRAMERATE_MIN_LATENCY);
  hWndTripleBuffering  = GetDlgItem (hDlg, IDC_SUS_TRIPLE_BUFFERING);
  hWndPreRenderLimit   = GetDlgItem (hDlg, IDC_SUS_PRERENDER_LIMIT);

  ComboBox_ResetContent (hWndFPSPreset);

  ComboBox_InsertString (hWndFPSPreset, 0, L"Don't Care");
  ComboBox_InsertString (hWndFPSPreset, 1, L"60 FPS");
  ComboBox_InsertString (hWndFPSPreset, 2, L"48 FPS");
  ComboBox_InsertString (hWndFPSPreset, 3, L"30 FPS");
  ComboBox_InsertString (hWndFPSPreset, 4, L"Custom");

  int max_fps = fps->get_value ();

  ShowWindow (hWndFPSCustom, SW_HIDE);

  if (max_fps == 0)
    ComboBox_SetCurSel (hWndFPSPreset, 0);
  else if (max_fps == 60)
    ComboBox_SetCurSel (hWndFPSPreset, 1);
  else if (max_fps == 48)
    ComboBox_SetCurSel (hWndFPSPreset, 2);
  else if (max_fps == 30)
    ComboBox_SetCurSel (hWndFPSPreset, 3);
  else {
    ComboBox_SetCurSel (hWndFPSPreset, 4);
    ShowWindow (hWndFPSCustom, SW_SHOW);
  }

  Edit_SetText (hWndFPSCustom, fps->get_value_str ().c_str ());
  fps->bind_to_control (new ds3t::UI::EditBox (hWndFPSCustom));

  minimize_latency->bind_to_control (new ds3t::UI::CheckBox (hWndMinimizeLatency));
  minimize_latency->set_value       (minimize_latency->get_value ());

  ComboBox_ResetContent (hWndPreRenderLimit);

  ComboBox_InsertString (hWndPreRenderLimit, 0, L"Driver Preference");
  ComboBox_InsertString (hWndPreRenderLimit, 1, L"1");
  ComboBox_InsertString (hWndPreRenderLimit, 2, L"2");
  ComboBox_InsertString (hWndPreRenderLimit, 3, L"3");
  ComboBox_InsertString (hWndPreRenderLimit, 4, L"4");
  ComboBox_InsertString (hWndPreRenderLimit, 5, L"5");
  ComboBox_InsertString (hWndPreRenderLimit, 6, L"6");

  int prerender = prerender_limit->get_value ();

  if (prerender == 1)
    ComboBox_SetCurSel (hWndPreRenderLimit, 1);
  else if (prerender == 2)
    ComboBox_SetCurSel (hWndPreRenderLimit, 2);
  else if (prerender == 3)
    ComboBox_SetCurSel (hWndPreRenderLimit, 3);
  else if (prerender == 4)
    ComboBox_SetCurSel (hWndPreRenderLimit, 4);
  else if (prerender == 5)
    ComboBox_SetCurSel (hWndPreRenderLimit, 5);
  else if (prerender == 6)
    ComboBox_SetCurSel (hWndPreRenderLimit, 6);
  else /*if (prerender == -1)*/
    ComboBox_SetCurSel (hWndPreRenderLimit, 0);

  bool check = backbuffer_count->get_value () == 2;
  Button_SetCheck (framerate->hWndTripleBuffering, check);

  return true;
}

int
suscfg_Framerate::poll_prerender_limit (HWND hDlg)
{
  int sel = ComboBox_GetCurSel (hWndPreRenderLimit);

  if (sel == 0)
    return -1;
  else if (sel == 1)
    return 1;
  else if (sel == 2)
    return 2;
  else if (sel == 3)
    return 3;
  else if (sel == 4)
    return 4;
  else if (sel == 5)
    return 5;
  else if (sel == 6)
    return 6;
  else /*if (sel == 0)*/
    return -1;
}

int
suscfg_Framerate::poll_framerate_limit (HWND hDlg)
{
  int sel = ComboBox_GetCurSel (hWndFPSPreset);

  if (sel == 0)
    return 0;
  else if (sel == 1)
    return 60;
  else if (sel == 2)
    return 48;
  else if (sel == 3)
    return 30;
  else {
    wchar_t wszFPSNum [16];
    Edit_GetLine (hWndFPSCustom, 0, wszFPSNum, 16);
    return _wtoi (wszFPSNum);
  }
}

INT_PTR
CALLBACK
FramerateConfig (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch (message)
  {
  case WM_INITDIALOG:
  {
    if (framerate == nullptr) {
      framerate = new suscfg_Framerate ();
    }

    framerate->setup_ui (hDlg);
  } break;

  case WM_COMMAND:
  {
    if (LOWORD (wParam) == IDC_TARGET_FPS) {
      if (HIWORD (wParam) == CBN_SELCHANGE) {
        int fps_sel = ComboBox_GetCurSel (framerate->hWndFPSPreset);
        ShowWindow (framerate->hWndFPSCustom, fps_sel >= 4);
        framerate->fps->set_value (framerate->poll_framerate_limit (hDlg));
      }
    }

    if (LOWORD (wParam) == IDOK)
    {
      BOOL check;

      //check = Button_GetCheck (framerate->hWndMinimizeLatency);
      //framerate->minimize_latency->set_value (check);

      check = Button_GetCheck (framerate->hWndTripleBuffering);
      if (check) {
        framerate->backbuffer_count->set_value (2);
      } else {
        framerate->backbuffer_count->set_value (-1);
      }

      framerate->prerender_limit->set_value (
        framerate->poll_prerender_limit (hDlg)
      );

      framerate->fps->set_value (
        framerate->poll_framerate_limit (hDlg)
      );
      framerate->fps->store ();

      //framerate->minimize_latency->store ();

      framerate->backbuffer_count->store ();
      framerate->prerender_limit->store  ();

      EndDialog (hDlg, LOWORD (wParam));
      return (INT_PTR)TRUE;
    }
  } break;
  }

  return (INT_PTR)FALSE;
}