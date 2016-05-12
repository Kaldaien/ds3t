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

class suscfg_PerfWiz
{
public:
  suscfg_PerfWiz (void);

  bool setup_ui  (HWND hDlg);

//protected:
  //ds3t::ParameterBool*    no_sound;
  //ds3t::ParameterBool*    take_screenshot;
  //ds3t::ParameterStringW* sound_file;

//private:
  HWND hWndDisplayMode;
  HWND hWndTearing;
  HWND hWndGoal;
} *perfwiz = nullptr;

suscfg_PerfWiz::suscfg_PerfWiz (void)
{
  ds3t::INI::File* sus_ini  = config.get_file_sus  ();
  ds3t::INI::File* dxgi_ini = config.get_file_dxgi ();
}

bool
suscfg_PerfWiz::setup_ui (HWND hDlg)
{
  hWndDisplayMode = GetDlgItem (hDlg, IDC_PERF_DISP_MODE);
  hWndTearing     = GetDlgItem (hDlg, IDC_PERF_TEARING);
  hWndGoal        = GetDlgItem (hDlg, IDC_PERF_GOAL);

  ComboBox_ResetContent (hWndDisplayMode);
  ComboBox_InsertString (hWndDisplayMode, 0, L"Windowed (or Borderless)");
  ComboBox_InsertString (hWndDisplayMode, 1, L"Fullscreen (Exclusive)");
  ComboBox_InsertString (hWndDisplayMode, 2, L"Multi-GPU (SLI)");
  ComboBox_SetCurSel    (hWndDisplayMode, 0);

  ComboBox_ResetContent (hWndTearing);
  ComboBox_InsertString (hWndTearing, 0, L"Don't Care");
  ComboBox_InsertString (hWndTearing, 1, L"Disallow");
  ComboBox_InsertString (hWndTearing, 2, L"Allow");
  ComboBox_SetCurSel    (hWndTearing, 1);
  ComboBox_Enable       (hWndTearing, FALSE);

  ComboBox_ResetContent (hWndGoal);
  ComboBox_InsertString (hWndGoal, 0, L"Minimize Latency");
  ComboBox_InsertString (hWndGoal, 1, L"Balanced Performance");
  ComboBox_InsertString (hWndGoal, 2, L"Minimize Stutter");
  ComboBox_SetCurSel    (hWndGoal, 1);

  return true;
}

void setup_display_description (HWND hDlg)
{
  if (ComboBox_GetCurSel (perfwiz->hWndDisplayMode) == 0) {
    ComboBox_Enable    (perfwiz->hWndTearing, FALSE);
    ComboBox_SetCurSel (perfwiz->hWndTearing, 1);

    Static_SetText (
      GetDlgItem (hDlg,IDC_PERF_HEADER),
        L"Windowed (or Borderless)"
    );
    Static_SetText (
      GetDlgItem (hDlg,IDC_PERF_DESCRIPTION),
        L"Reduces application switching time and eliminates tearing without the performance or latency penalties of VSYNC when framerate is unstable."
    );
    Static_SetText (
      GetDlgItem (hDlg,IDC_PERF_FOOTER),
        L"Improves performance with some video capture software"
    );
  }
  else {
    if (ComboBox_GetCurSel (perfwiz->hWndDisplayMode) == 1) {
      Static_SetText (
        GetDlgItem (hDlg,IDC_PERF_HEADER),
          L"Fullscreen (Exclusive)"
      );

      Static_SetText (
        GetDlgItem (hDlg,IDC_PERF_DESCRIPTION),
          L"May improve maximum framerate, but requires VSYNC to eliminate tearing and application switching time may be significant."
      );

      Static_SetText (
        GetDlgItem (hDlg,IDC_PERF_FOOTER),
          L"Do not use Flip Mode with this selected"
      );
    }

    if (ComboBox_GetCurSel (perfwiz->hWndDisplayMode) == 2) {
      Static_SetText (
        GetDlgItem (hDlg,IDC_PERF_HEADER),
          L"Multi-GPU (SLI)"
      );

      Static_SetText (
        GetDlgItem (hDlg,IDC_PERF_DESCRIPTION),
          L"Same as Fullscreen (Exclusive), but rules out certain optimizations that only work in single-GPU rendering mode."
      );

      Static_SetText (
        GetDlgItem (hDlg,IDC_PERF_FOOTER),
          L"Consider \"Smooth VSYNC\" in the NVIDIA Driver Tweaks"
      );
    }

    ComboBox_Enable (perfwiz->hWndTearing, TRUE);
  }
}

void setup_tearing_description (HWND hDlg)
{
  if (ComboBox_GetCurSel (perfwiz->hWndTearing) == 0) {
    Static_SetText (
      GetDlgItem (hDlg,IDC_PERF_HEADER),
        L"Tearing: Don't Care"
    );
    Static_SetText (
      GetDlgItem (hDlg,IDC_PERF_DESCRIPTION),
        L"Selects settings that work whether VSYNC is ON or OFF."
    );
    Static_SetText (
      GetDlgItem (hDlg,IDC_PERF_FOOTER),
        L""
    );
  }
  else {
    if (ComboBox_GetCurSel (perfwiz->hWndTearing) == 1) {
      Static_SetText (
        GetDlgItem (hDlg,IDC_PERF_HEADER),
          L"Tearing: Disallow"
      );
      Static_SetText (
        GetDlgItem (hDlg,IDC_PERF_DESCRIPTION),
          L"Enables VSYNC and selects settings optimized to work with it."
      );
      Static_SetText (
        GetDlgItem (hDlg,IDC_PERF_FOOTER),
          L""
      );
    }

    if (ComboBox_GetCurSel (perfwiz->hWndTearing) == 2) {
      Static_SetText (
        GetDlgItem (hDlg,IDC_PERF_HEADER),
          L"Tearing: Allow"
      );
      Static_SetText (
        GetDlgItem (hDlg,IDC_PERF_DESCRIPTION),
          L"Disables VSYNC and selects settings optimized to work without it."
      );
      Static_SetText (
        GetDlgItem (hDlg,IDC_PERF_FOOTER),
          L""
      );
    }

    ComboBox_Enable (perfwiz->hWndTearing, TRUE);
  }
}

void setup_goal_description (HWND hDlg)
{
  if (ComboBox_GetCurSel (perfwiz->hWndGoal) == 0) {
    Static_SetText (
      GetDlgItem (hDlg,IDC_PERF_HEADER),
        L"Performance Goal: Minimize Latency"
    );
    Static_SetText (
      GetDlgItem (hDlg,IDC_PERF_DESCRIPTION),
        L"Favors low input latency above all else, even if it introduces stuttering."
    );
    Static_SetText (
      GetDlgItem (hDlg,IDC_PERF_FOOTER),
        L"Use if framerate is stable but controls are sluggish"
    );
  }
  else {
    if (ComboBox_GetCurSel (perfwiz->hWndGoal) == 1) {
      Static_SetText (
        GetDlgItem (hDlg,IDC_PERF_HEADER),
          L"Performance Goal: Balanced Performance"
      );
      Static_SetText (
        GetDlgItem (hDlg,IDC_PERF_DESCRIPTION),
          L"Tries to balance stuttering against input latency; avoids aggressive frame pacing optimizations that may lead to unresponsive input."
      );
      Static_SetText (
        GetDlgItem (hDlg,IDC_PERF_FOOTER),
          L"Try this first"
      );
    }

    if (ComboBox_GetCurSel (perfwiz->hWndGoal) == 2) {
      Static_SetText (
        GetDlgItem (hDlg,IDC_PERF_HEADER),
          L"Performance Goal: Minimize Stutter"
      );
      Static_SetText (
        GetDlgItem (hDlg,IDC_PERF_DESCRIPTION),
          L"Tries to maximize GPU work queue to deal with CPU load fluctuations (stuttering)."
      );
      Static_SetText (
        GetDlgItem (hDlg,IDC_PERF_FOOTER),
          L"Adds input delay with Fullscreen (Exclusive)"
      );
    }
  }
}

INT_PTR
CALLBACK
PerformanceWizard (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch (message)
  {
    case WM_INITDIALOG:
    {
      if (perfwiz == nullptr) {
        perfwiz = new suscfg_PerfWiz ();
      }

      HFONT header_font =
        CreateFont ( 14,
                       0, 0, 0,
                         FW_HEAVY,
                           FALSE, TRUE, FALSE,
                             DEFAULT_CHARSET,
                               OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                 CLEARTYPE_NATURAL_QUALITY,
                                   MONO_FONT,
                                     L"Consolas" );

      HFONT body_font =
        CreateFont ( 13,
                       0, 0, 0,
                         FW_NORMAL,
                           TRUE, FALSE, FALSE,
                             DEFAULT_CHARSET,
                               OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                 CLEARTYPE_NATURAL_QUALITY,
                                   MONO_FONT,
                                     L"Consolas" );

      HFONT footer_font =
        CreateFont ( 13,
                       0, 0, 0,
                         FW_HEAVY,
                           TRUE, FALSE, FALSE,
                             DEFAULT_CHARSET,
                               OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                 CLEARTYPE_NATURAL_QUALITY,
                                   MONO_FONT,
                                     L"Consolas" );

      HINSTANCE hShell32 = LoadLibrary (L"Shell32.dll");

      static HICON hIconOptimize = LoadIcon (hShell32, MAKEINTRESOURCE (16752));
      static HICON hIconCompat   = LoadIcon (hShell32, MAKEINTRESOURCE (16741));

      SendMessage  ( GetDlgItem (hDlg, IDC_PERF_OPTIMIZE),
                       BM_SETIMAGE, IMAGE_ICON,
                        LPARAM (hIconOptimize) );

      SendMessage  ( GetDlgItem (hDlg, IDC_PERF_COMPATIBILITY),
                       BM_SETIMAGE, IMAGE_ICON,
                         LPARAM (hIconCompat) );

      FreeLibrary (hShell32);

      SetWindowFont (GetDlgItem (hDlg, IDC_PERF_HEADER),      header_font, true);
      SetWindowFont (GetDlgItem (hDlg, IDC_PERF_DESCRIPTION), body_font,   true);
      SetWindowFont (GetDlgItem (hDlg, IDC_PERF_FOOTER),      footer_font, true);

      perfwiz->setup_ui (hDlg);

      setup_display_description (hDlg);
    } break;

    case WM_COMMAND:
    {
      if (LOWORD (wParam) == IDC_PERF_DISP_MODE) {
        if ( HIWORD (wParam) == CBN_SELCHANGE || 
             HIWORD (wParam) == CBN_SETFOCUS ) {
          setup_display_description (hDlg);
        }
      }

      if (LOWORD (wParam) == IDC_PERF_TEARING) {
        if ( HIWORD (wParam) == CBN_SELCHANGE || 
             HIWORD (wParam) == CBN_SETFOCUS ) {
          setup_tearing_description (hDlg);
        }
      }

      if (LOWORD (wParam) == IDC_PERF_GOAL) {
        if ( HIWORD (wParam) == CBN_SELCHANGE || 
             HIWORD (wParam) == CBN_SETFOCUS ) {
          setup_goal_description (hDlg);
        }
      }

      if (LOWORD(wParam) == IDC_ACHIEVEMENT_SOUND)
      {
        //SelectSoundFile ();
      }

      if (LOWORD (wParam) == IDOK)
      {
        EndDialog (hDlg, LOWORD (wParam));
        return (INT_PTR)TRUE;
      }
    } break;
  }

  return (INT_PTR)FALSE;
}