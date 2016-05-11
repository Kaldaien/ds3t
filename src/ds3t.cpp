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

#define _CRT_SECURE_NO_WARNINGS

#include "stdafx.h"

#include "ds3t.h"
#include "cfg.h"
#include "parameter.h"
#include "utility.h"
#include "nvapi.h"
#include "dxgi.h"

#include <map>
#include <set>

#include <cstdio>

#include <windowsx.h>
#include <CommCtrl.h> // Button_GetIdealSize

#pragma comment(lib, "Comctl32.lib")

using namespace ds3t;

#define DS3T_VERSION_STR L"0.0.3"

INT_PTR CALLBACK  Config (HWND, UINT, WPARAM, LPARAM);

bool  messagebox_active; // Workaround some particularly strange behavior
bool  first_load = true; // Some settings should only be loaded once; ignore when false

HWND  hWndApp;

//extern INT_PTR CALLBACK AudioConfig     (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
extern INT_PTR CALLBACK FramerateConfig (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
extern INT_PTR CALLBACK GraphicsConfig  (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
extern INT_PTR CALLBACK SteamConfig     (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
extern INT_PTR CALLBACK PluginsConfig   (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
extern INT_PTR CALLBACK OSDConfig       (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

HWND  hWndSUSTab;

HICON ds3t_icon;
HICON nv_icon;
HICON steam_icon;

DS3_Config config;

int APIENTRY _tWinMain(_In_ HINSTANCE     hInstance,
                       _In_opt_ HINSTANCE hPrevInstance,
                       _In_ LPTSTR        lpCmdLine,
                       _In_ int           nCmdShow)
{
  UNREFERENCED_PARAMETER(hPrevInstance);
  UNREFERENCED_PARAMETER(lpCmdLine);
  UNREFERENCED_PARAMETER (nCmdShow);

  CoInitialize (NULL);

  ds3t_icon  = LoadIcon (hInstance, MAKEINTRESOURCE (IDI_DS3T));
  steam_icon = LoadIcon (hInstance, MAKEINTRESOURCE (IDI_STEAM));

  HINSTANCE hCUDA = LoadLibrary (L"nvcuda.dll");

  if (hCUDA != NULL) {
    nv_icon = LoadIcon (hCUDA, MAKEINTRESOURCE (100));
    FreeLibrary (hCUDA);
  }

  return (int)DialogBox (hInstance, MAKEINTRESOURCE (IDD_DS3T), NULL, Config);
}

void get_resolution (HWND hDlg, int* x, int* y, int* refresh)
{
  HWND hWndResolution = GetDlgItem (hDlg, IDC_RESOLUTION);
  HWND hWndRefresh    = GetDlgItem (hDlg, IDC_REFRESH_RATE);

  wchar_t wszRes [64];
  ComboBox_GetText (hWndResolution, wszRes, 64);

  wchar_t wszRef [16];
  ComboBox_GetText (hWndRefresh, wszRef, 16);

  swscanf_s (wszRes, L"%dx%d", x, y);
  swscanf_s (wszRef, L"%d", refresh);
}

void setup_resolution (HWND hDlg)
{
  DEVMODE dmDisplay;
  ZeroMemory (&dmDisplay, sizeof DEVMODE);
  dmDisplay.dmSize = sizeof DEVMODE;

  // X,Y pairs are mapped to sets of refresh rates
  std::map <std::pair <int, int>, std::set <int> > resolutions;

  int i = 0;
  while (EnumDisplaySettings (NULL, i, &dmDisplay)) {
    i++;

    std::pair <int, int> res;
    res.first  = dmDisplay.dmPelsWidth;
    res.second = dmDisplay.dmPelsHeight;
    resolutions [res].insert (dmDisplay.dmDisplayFrequency);
  }

  std::map <std::pair <int, int>, std::set <int> >::const_iterator res = resolutions.begin ();
  std::map <std::pair <int, int>, std::set <int> >::const_iterator end = resolutions.end ();

  HWND hWndResolution = GetDlgItem (hDlg, IDC_RESOLUTION);
  HWND hWndRefresh    = GetDlgItem (hDlg, IDC_REFRESH_RATE);

  ComboBox_ResetContent (hWndResolution);
  ComboBox_ResetContent (hWndRefresh);

  int sel = -1;
        i =  0;
  while (res != end) {
    wchar_t wszRes [64];
    swprintf (wszRes, 64, L"%dx%d", res->first.first, res->first.second);
    ComboBox_InsertString (hWndResolution, i++, wszRes);
    if (res_x->get_value () == res->first.first &&
        res_y->get_value () == res->first.second)
      sel = (i - 1);
    ++res;
  }

  // Set to highest resolution if no valid resolution is selected
  if (sel == -1)
    sel = (i - 1);

  ComboBox_SetCurSel (hWndResolution, sel);

  res = resolutions.find (std::pair <int, int> (res_x->get_value (), res_y->get_value ()));

  if (res != resolutions.end ()) {
    std::set <int>::const_iterator refresh = res->second.begin ();

      i =  0;
    sel = -1;

    while (refresh != res->second.end ()) {
      wchar_t wszRefresh [64];
      swprintf (wszRefresh, 64, L"%d", *refresh);
      ComboBox_InsertString (hWndRefresh, i++, wszRefresh);
      if (refresh_rate->get_value () == *refresh)
        sel = (i - 1);
      ++refresh;
    }

    // Set to highest refresh if no valid refresh is selected
    if (sel == -1)
      sel = (i - 1);

    ComboBox_SetCurSel (hWndRefresh, sel);
  }
}

// This resizes a UI control, avoid calling it multiple times or it will continue
//   to shrink each time!
void setup_driver_tweaks (HWND hDlg)
{
  // If there is an NV GPU installed, display a special button!
  if (NVAPI::CountPhysicalGPUs () > 0) {
    std::wstring button_label = L"    NVIDIA Driver Tweaks\r\n    (Version: ";
    button_label += NVAPI::GetDriverVersion () + L")";

    SetWindowText (GetDlgItem (hDlg, IDC_NV_DRIVER_TWEAKS), button_label.c_str ());
    ShowWindow (GetDlgItem (hDlg, IDC_NV_DRIVER_TWEAKS), true);

    // Resize the GPU Info Text Window to fit the new button...
    RECT wnd_rect;
    GetWindowRect (GetDlgItem (hDlg, IDC_GPUINFO), &wnd_rect);
    SetWindowPos (GetDlgItem (hDlg, IDC_GPUINFO), NULL, 0, 0, wnd_rect.right - wnd_rect.left, wnd_rect.bottom - wnd_rect.top - 40, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
    SendMessage  (GetDlgItem (hDlg, IDC_NV_DRIVER_TWEAKS), BM_SETIMAGE, IMAGE_ICON, LPARAM (nv_icon));
  }
}

void setup_ssao (HWND hDlg)
{
  HWND hWndSSAO = GetDlgItem (hDlg, IDC_SSAO);

  ComboBox_ResetContent (hWndSSAO);
  ComboBox_InsertString (hWndSSAO, 0, L"Off");
  ComboBox_InsertString (hWndSSAO, 1, L"Medium");
  ComboBox_InsertString (hWndSSAO, 2, L"High");

  std::wstring ssao = xml_ssao->get_value ();

  if (ssao == L"DISABLE")
    ComboBox_SetCurSel (hWndSSAO, 0);
  else if (ssao == L"MEDIUM")
    ComboBox_SetCurSel (hWndSSAO, 1);
  else /*if (ssao == L"HIGH")*/
    ComboBox_SetCurSel (hWndSSAO, 2);
}

std::wstring get_ssao (HWND hDlg)
{
  int sel = ComboBox_GetCurSel (GetDlgItem (hDlg, IDC_SSAO));

  if (sel == 0)
    return L"DISABLE";
  else if (sel == 1)
    return L"MEDIUM";
  else /*if (sel == 2)*/
    return L"HIGH";
}


void setup_dof (HWND hDlg)
{
  HWND hWndDOF = GetDlgItem (hDlg, IDC_DOF);

  ComboBox_ResetContent (hWndDOF);
  ComboBox_InsertString (hWndDOF, 0, L"Off");
  ComboBox_InsertString (hWndDOF, 1, L"Low");
  ComboBox_InsertString (hWndDOF, 2, L"Medium");
  ComboBox_InsertString (hWndDOF, 3, L"High");
  ComboBox_InsertString (hWndDOF, 4, L"Maximum");

  std::wstring dof = xml_dof->get_value ();

  if (dof == L"DISABLE")
    ComboBox_SetCurSel (hWndDOF, 0);
  else if (dof == L"LOW")
    ComboBox_SetCurSel (hWndDOF, 1);
  else if (dof == L"MEDIUM")
    ComboBox_SetCurSel (hWndDOF, 2);
  else if (dof == L"HIGH")
    ComboBox_SetCurSel (hWndDOF, 3);
  else /*if (dof == L"MAX")*/
    ComboBox_SetCurSel (hWndDOF, 4);
}

std::wstring get_dof (HWND hDlg)
{
  int sel = ComboBox_GetCurSel (GetDlgItem (hDlg, IDC_DOF));

  if (sel == 0)
    return L"DISABLE";
  else if (sel == 1)
    return L"LOW";
  else if (sel == 2)
    return L"MEDIUM";
  else if (sel == 3)
    return L"HIGH";
  else /*if (sel == 4)*/
    return L"MAX";
}


void setup_motion_blur (HWND hDlg)
{
  HWND hWndMotionBlur = GetDlgItem (hDlg, IDC_MOTION_BLUR);

  ComboBox_ResetContent (hWndMotionBlur);
  ComboBox_InsertString (hWndMotionBlur, 0, L"Off");
  ComboBox_InsertString (hWndMotionBlur, 1, L"Low");
  ComboBox_InsertString (hWndMotionBlur, 2, L"Medium");
  ComboBox_InsertString (hWndMotionBlur, 3, L"High");

  std::wstring motion_blur = xml_motion_blur->get_value ();

  if (motion_blur == L"DISABLE")
    ComboBox_SetCurSel (hWndMotionBlur, 0);
  else if (motion_blur == L"LOW")
    ComboBox_SetCurSel (hWndMotionBlur, 1);
  else if (motion_blur == L"MEDIUM")
    ComboBox_SetCurSel (hWndMotionBlur, 2);
  else /*if (motion_blur == L"HIGH")*/
    ComboBox_SetCurSel (hWndMotionBlur, 3);
}

std::wstring get_motion_blur (HWND hDlg)
{
  int sel = ComboBox_GetCurSel (GetDlgItem (hDlg, IDC_MOTION_BLUR));

  if (sel == 0)
    return L"DISABLE";
  else if (sel == 1)
    return L"LOW";
  else if (sel == 2)
    return L"MEDIUM";
  else /*if (sel == 3)*/
    return L"HIGH";
}


void setup_lighting_quality (HWND hDlg)
{
  HWND hWndLightingQuality = GetDlgItem (hDlg, IDC_LIGHTING_QUALITY);

  ComboBox_ResetContent (hWndLightingQuality);
  ComboBox_InsertString (hWndLightingQuality, 0, L"Low");
  ComboBox_InsertString (hWndLightingQuality, 1, L"Medium");
  ComboBox_InsertString (hWndLightingQuality, 2, L"High");
  ComboBox_InsertString (hWndLightingQuality, 3, L"Maximum");

  std::wstring lighting_quality = xml_lighting_quality->get_value ();

  if (lighting_quality == L"LOW")
    ComboBox_SetCurSel (hWndLightingQuality, 0);
  else if (lighting_quality == L"MEDIUM")
    ComboBox_SetCurSel (hWndLightingQuality, 1);
  else if (lighting_quality == L"HIGH")
    ComboBox_SetCurSel (hWndLightingQuality, 2);
  else /*if (lighting_quality == L"MAX")*/
    ComboBox_SetCurSel (hWndLightingQuality, 3);
}

std::wstring get_lighting_quality (HWND hDlg)
{
  int sel = ComboBox_GetCurSel (GetDlgItem (hDlg, IDC_LIGHTING_QUALITY));

  if (sel == 0)
    return L"LOW";
  else if (sel == 1)
    return L"MEDIUM";
  else if (sel == 2)
    return L"HIGH";
  else /*if (sel == 3)*/
    return L"MAX";
}


void setup_effects_quality (HWND hDlg)
{
  HWND hWndEffectsQuality = GetDlgItem (hDlg, IDC_EFFECTS_QUALITY);

  ComboBox_ResetContent (hWndEffectsQuality);
  ComboBox_InsertString (hWndEffectsQuality, 0, L"Low");
  ComboBox_InsertString (hWndEffectsQuality, 1, L"Medium");
  ComboBox_InsertString (hWndEffectsQuality, 2, L"High");
  ComboBox_InsertString (hWndEffectsQuality, 3, L"Maximum");

  std::wstring effects_quality = xml_effects_quality->get_value ();

  if (effects_quality == L"LOW")
    ComboBox_SetCurSel (hWndEffectsQuality, 0);
  else if (effects_quality == L"MEDIUM")
    ComboBox_SetCurSel (hWndEffectsQuality, 1);
  else if (effects_quality == L"HIGH")
    ComboBox_SetCurSel (hWndEffectsQuality, 2);
  else /*if (effects_quality == L"MAX")*/
    ComboBox_SetCurSel (hWndEffectsQuality, 3);
}

std::wstring get_effects_quality (HWND hDlg)
{
  int sel = ComboBox_GetCurSel (GetDlgItem (hDlg, IDC_EFFECTS_QUALITY));

  if (sel == 0)
    return L"LOW";
  else if (sel == 1)
    return L"MEDIUM";
  else if (sel == 2)
    return L"HIGH";
  else /*if (sel == 3)*/
    return L"MAX";
}


void setup_reflection_quality (HWND hDlg)
{
  HWND hWndReflectionQuality = GetDlgItem (hDlg, IDC_REFLECTION_QUALITY);

  ComboBox_ResetContent (hWndReflectionQuality);
  ComboBox_InsertString (hWndReflectionQuality, 0, L"Low");
  ComboBox_InsertString (hWndReflectionQuality, 1, L"High");
  ComboBox_InsertString (hWndReflectionQuality, 2, L"Maximum");

  std::wstring reflection_quality = xml_reflection_quality->get_value ();

  if (reflection_quality == L"DISABLE")
    ComboBox_SetCurSel (hWndReflectionQuality, 0);
  else if (reflection_quality == L"HIGH")
    ComboBox_SetCurSel (hWndReflectionQuality, 1);
  else /*if (reflection_quality == L"MAX")*/
    ComboBox_SetCurSel (hWndReflectionQuality, 2);
}

std::wstring get_reflection_quality (HWND hDlg)
{
  int sel = ComboBox_GetCurSel (GetDlgItem (hDlg, IDC_REFLECTION_QUALITY));

  if (sel == 0)
    return L"DISABLE";
  else if (sel == 1)
    return L"HIGH";
  else /*if (sel == 2)*/
    return L"MAX";
}


void setup_water_quality (HWND hDlg)
{
  HWND hWndWaterQuality = GetDlgItem (hDlg, IDC_WATER_QUALITY);

  ComboBox_ResetContent (hWndWaterQuality);
  ComboBox_InsertString (hWndWaterQuality, 0, L"Low");
  ComboBox_InsertString (hWndWaterQuality, 1, L"High");

  std::wstring water_quality = xml_water_quality->get_value ();

  if (water_quality == L"DISABLE")
    ComboBox_SetCurSel (hWndWaterQuality, 0);
  else /*if (water_quality == L"HIGH")*/
    ComboBox_SetCurSel (hWndWaterQuality, 1);
}

std::wstring get_water_quality (HWND hDlg)
{
  int sel = ComboBox_GetCurSel (GetDlgItem (hDlg, IDC_WATER_QUALITY));

  if (sel == 0)
    return L"DISABLE";
  else /*if (sel == 1)*/
    return L"HIGH";
}


void setup_shader_quality (HWND hDlg)
{
  HWND hWndShaderQuality = GetDlgItem (hDlg, IDC_SHADER_QUALITY);

  ComboBox_ResetContent (hWndShaderQuality);
  ComboBox_InsertString (hWndShaderQuality, 0, L"Low");
  ComboBox_InsertString (hWndShaderQuality, 1, L"Medium");
  ComboBox_InsertString (hWndShaderQuality, 2, L"High");
  ComboBox_InsertString (hWndShaderQuality, 3, L"Maximum");

  std::wstring shader_quality = xml_shader_quality->get_value ();

  if (shader_quality == L"LOW")
    ComboBox_SetCurSel (hWndShaderQuality, 0);
  else if (shader_quality == L"MEDIUM")
    ComboBox_SetCurSel (hWndShaderQuality, 1);
  else if (shader_quality == L"HIGH")
    ComboBox_SetCurSel (hWndShaderQuality, 2);
  else /*if (shader_quality == L"MAX")*/
    ComboBox_SetCurSel (hWndShaderQuality, 3);
}

std::wstring get_shader_quality (HWND hDlg)
{
  int sel = ComboBox_GetCurSel (GetDlgItem (hDlg, IDC_SHADER_QUALITY));

  if (sel == 0)
    return L"LOW";
  else if (sel == 1)
    return L"MEDIUM";
  else if (sel == 2)
    return L"HIGH";
  else /*if (sel == 3)*/
    return L"MAX";
}



void setup_shadow_quality (HWND hDlg)
{
  HWND hWndShadowQuality = GetDlgItem (hDlg, IDC_SHADOW_QUALITY);

  ComboBox_ResetContent (hWndShadowQuality);
  ComboBox_InsertString (hWndShadowQuality, 0, L"Off");
  ComboBox_InsertString (hWndShadowQuality, 1, L"Low");
  ComboBox_InsertString (hWndShadowQuality, 2, L"Medium");
  ComboBox_InsertString (hWndShadowQuality, 3, L"High");
  ComboBox_InsertString (hWndShadowQuality, 4, L"Maximum");

  std::wstring shadows = xml_shadow_quality->get_value ();

  if (shadows == L"DISABLE")
    ComboBox_SetCurSel (hWndShadowQuality, 0);
  else if (shadows == L"LOW")
    ComboBox_SetCurSel (hWndShadowQuality, 1);
  else if (shadows == L"MEDIUM")
    ComboBox_SetCurSel (hWndShadowQuality, 2);
  else if (shadows == L"HIGH")
    ComboBox_SetCurSel (hWndShadowQuality, 3);
  else /*if (shadows == L"MAX")*/
    ComboBox_SetCurSel (hWndShadowQuality, 4);
}

std::wstring get_shadow_quality (HWND hDlg)
{
  int sel = ComboBox_GetCurSel (GetDlgItem (hDlg, IDC_SHADOW_QUALITY));

  if (sel == 0)
    return L"DISABLE";
  else if (sel == 1)
    return L"LOW";
  else if (sel == 2)
    return L"MEDIUM";
  else if (sel == 3)
    return L"HIGH";
  else /*if (sel == 4)*/
    return L"MAX";
}


void setup_antialiasing (HWND hDlg)
{
#if 0
  HWND hWndAntialiasing = GetDlgItem (hDlg, IDC_FXAA_LEVEL);

  ComboBox_ResetContent (hWndAntialiasing);
  ComboBox_InsertString (hWndAntialiasing, 0, L"Off");
  ComboBox_InsertString (hWndAntialiasing, 1, L"Low");
  ComboBox_InsertString (hWndAntialiasing, 2, L"Medium");
  ComboBox_InsertString (hWndAntialiasing, 3, L"High");

  std::wstring fxaa = antialiasing->get_value ();

  if (fxaa == L"off")
    ComboBox_SetCurSel (hWndAntialiasing, 0);
  else if (fxaa == L"fxaa_low")
    ComboBox_SetCurSel (hWndAntialiasing, 1);
  else if (fxaa == L"fxaa_medium")
    ComboBox_SetCurSel (hWndAntialiasing, 2);
  else /*if (fxaa == L"fxaa_high")*/
    ComboBox_SetCurSel (hWndAntialiasing, 3);
#endif
}

void setup_tex_filter (HWND hDlg)
{
  HWND hWndAnisotropy = GetDlgItem (hDlg, IDC_ANISOTROPY);

  ComboBox_ResetContent (hWndAnisotropy);
  ComboBox_InsertString (hWndAnisotropy, 0, L"Trilinear");
  ComboBox_InsertString (hWndAnisotropy, 1, L"2x Anisotropic");
  ComboBox_InsertString (hWndAnisotropy, 2, L"4x Anisotropic");
  ComboBox_InsertString (hWndAnisotropy, 3, L"8x Anisotropic");
  ComboBox_InsertString (hWndAnisotropy, 4, L"16x Anisotropic");

  switch (anisotropy->get_value ()) {
    case 0:
    case 1:
      // 1 sample = Bilinear + Linear/Point filter on Mip
      ComboBox_SetCurSel (hWndAnisotropy, 0);
      break;
    case 2:
      // 2X AF
      ComboBox_SetCurSel (hWndAnisotropy, 1);
      break;

      /// Believe it or not, anisotropy doesn't have to be a power-of-two
      ///   or even an integer... but the engine might not accept
      ///     those sorts of values, so let's not confuse it ;)

    default: // Always fallback to 4x AF if bad values are passed.
    case 4:  // 4x AF
      ComboBox_SetCurSel (hWndAnisotropy, 2);
      break;
    case 8: // 8x AF
      ComboBox_SetCurSel (hWndAnisotropy, 3);
      break;
    case 16: // 16x AF
      ComboBox_SetCurSel (hWndAnisotropy, 4);
      break;
  }
}

int get_tex_filter (HWND hDlg)
{
  int sel = ComboBox_GetCurSel (GetDlgItem (hDlg, IDC_ANISOTROPY));

  if (sel == 0)
    return 0; // 1 is trilinear, but the game uses 0
  else if (sel == 1)
    return 2;
  else if (sel == 2)
    return 4;
  else if (sel == 3)
    return 8;
  else
    return 16;
}

void setup_tex_res (HWND hDlg)
{
  HWND hWndTexRes = GetDlgItem (hDlg, IDC_TEXTURE_RES);

  ComboBox_ResetContent (hWndTexRes);
  ComboBox_InsertString (hWndTexRes, 0, L"CANNOT ADJUST ME");//Low (128x128 - 512x512) - 2 GiB VRAM");
  ComboBox_InsertString (hWndTexRes, 1, L"Normal (128x128 - 1024x1024) - 4 GiB VRAM");
  ComboBox_InsertString (hWndTexRes, 2, L"High (128x128 - 2048x2048) - 6 GiB VRAM");

  if (texture_res->get_value () == 2)      // High
    ComboBox_SetCurSel (hWndTexRes, 2);
  else if (texture_res->get_value () == 1) // Med
    ComboBox_SetCurSel (hWndTexRes, 1);
  else
    ComboBox_SetCurSel (hWndTexRes, 0);    // Anything Else = Low
}

int get_tex_res (HWND hDlg)
{
  return ComboBox_GetCurSel (GetDlgItem (hDlg, IDC_TEXTURE_RES));
}

bool DS3T_EpsilonTest (float in, float test)
{
  const float eps = 0.0001f;

  if (in + eps > test && in - eps < test)
    return true;

  return false;
}

INT_PTR CALLBACK DriverConfigNV (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);



// What a nasty kludge this is, fix this ASAP!!!!
int mode = 0;

void handle_window_radios (HWND hDlg, WORD ID)
{
  switch (ID) {
    case IDC_BORDER_WINDOW:
      mode = 0;
      arc_start_fullscreen->set_value (false);
      arc_start_fullscreen->store     ();
      break;
    case IDC_BORDERLESS_WINDOW:
      if (mode == 1) {
        mode = 3;
        arc_start_fullscreen->set_value (true);
        arc_start_fullscreen->store     ();
      }
      else {
        mode = 1;
        arc_start_fullscreen->set_value (false);
        arc_start_fullscreen->store     ();
      }
      break;
    case IDC_FULLSCREEN:
      mode = 2;
      arc_start_fullscreen->set_value (true);
      arc_start_fullscreen->store     ();
      break;
  }

  if (mode < 3) {
    LONG style = GetWindowLong (GetDlgItem (hDlg, IDC_BORDERLESS_WINDOW), GWL_STYLE);
         style = (style & ~BS_AUTO3STATE) | BS_AUTORADIOBUTTON;
    SetWindowLong (GetDlgItem (hDlg, IDC_BORDERLESS_WINDOW), GWL_STYLE, style);
  }

  if (mode == 0) {
    Button_SetCheck (GetDlgItem (hDlg, IDC_BORDER_WINDOW),     1);
    Button_SetCheck (GetDlgItem (hDlg, IDC_BORDERLESS_WINDOW), 0);
    Button_SetCheck (GetDlgItem (hDlg, IDC_FULLSCREEN),        0);
  }
  else if (mode == 2) {
    Button_SetCheck (GetDlgItem (hDlg, IDC_BORDER_WINDOW),     0);
    Button_SetCheck (GetDlgItem (hDlg, IDC_BORDERLESS_WINDOW), 0);
    Button_SetCheck (GetDlgItem (hDlg, IDC_FULLSCREEN),        1);
  }
  else {
    Button_SetCheck (GetDlgItem (hDlg, IDC_BORDER_WINDOW), 0);
    Button_SetCheck (GetDlgItem (hDlg, IDC_FULLSCREEN),    0);

    // Regular borderless
    if (mode == 1) {
      LONG style = GetWindowLong (GetDlgItem (hDlg, IDC_BORDERLESS_WINDOW), GWL_STYLE);
           style = (style & ~BS_AUTO3STATE) | BS_AUTORADIOBUTTON;
      SetWindowLong   (GetDlgItem (hDlg, IDC_BORDERLESS_WINDOW), GWL_STYLE, style);
      Button_SetCheck (GetDlgItem (hDlg, IDC_BORDERLESS_WINDOW), 1);
    }
    else {
      LONG style = GetWindowLong (GetDlgItem (hDlg, IDC_BORDERLESS_WINDOW), GWL_STYLE);
           style = (style & ~BS_AUTORADIOBUTTON) | BS_AUTO3STATE;
      SetWindowLong   (GetDlgItem (hDlg, IDC_BORDERLESS_WINDOW), GWL_STYLE, style);
      Button_SetCheck (GetDlgItem (hDlg, IDC_BORDERLESS_WINDOW), 2);
    }
  }

  bool sus_installed = false;

  if (! (config.get_file_sus  ()->get_sections ().empty () ||
         config.get_file_dxgi ()->get_sections ().empty ()))
    sus_installed = true;

  // Only enable this button in fullscreen mode...
  Button_Enable (GetDlgItem (hDlg, IDC_VSYNC), (/*mode == 2 &&*/ sus_installed));

  //if (mode != 2)
    //Button_SetCheck (GetDlgItem (hDlg, IDC_VSYNC), 0);
  if (presentation_interval != nullptr)
    Button_SetCheck (GetDlgItem (hDlg, IDC_VSYNC), presentation_interval->get_value ());
}

void setup_debug_utils (HWND hDlg, bool debug)
{
  ShowWindow (GetDlgItem (hDlg, IDC_GPUINFO),          (! debug));
  ShowWindow (GetDlgItem (hDlg, IDC_GPU_GROUP),        (! debug));
  if (NVAPI::CountPhysicalGPUs () > 0) {
    // Show/hide the NVIDIA driver tweaks button if applicable

    ShowWindow (GetDlgItem (hDlg, IDC_NV_DRIVER_TWEAKS), (!debug));
  }
  ShowWindow (GetDlgItem (hDlg, IDC_DEBUG_GROUP),         debug);
  ShowWindow (GetDlgItem (hDlg, IDC_NUKE_CONFIG),         debug);
  ShowWindow (GetDlgItem (hDlg, IDC_BACKUP_CONFIG),       debug);
  ShowWindow (GetDlgItem (hDlg, IDC_RESTORE_CONFIG),      debug);

  if (debug) {
    if (DS3T_HasBackupConfigFiles ()) {
      std::wstring backup_time = DS3T_GetBackupFileTime ();
      SetWindowText (GetDlgItem (hDlg, IDC_RESTORE_CONFIG), std::wstring (L" Restore Config Files\n " +
                                                                            backup_time).c_str ());
      EnableWindow (GetDlgItem (hDlg, IDC_RESTORE_CONFIG), TRUE);
      //EnableWindow (GetDlgItem (hDlg, IDC_BACKUP_CONFIG), FALSE);
      EnableWindow (GetDlgItem (hDlg, IDC_BACKUP_CONFIG),  TRUE);
      std::wstring config_time = DS3T_GetConfigFileTime ();
      SetWindowText (GetDlgItem (hDlg, IDC_BACKUP_CONFIG), std::wstring (L" Backup Config Files\n " +
                                                                         config_time).c_str ());
    }
    else {
      std::wstring config_time = DS3T_GetConfigFileTime ();
      SetWindowText (GetDlgItem (hDlg, IDC_BACKUP_CONFIG), std::wstring (L" Backup Config Files\n " +
                                                                         config_time).c_str ());
      EnableWindow (GetDlgItem (hDlg, IDC_BACKUP_CONFIG),  TRUE);
      EnableWindow (GetDlgItem (hDlg, IDC_RESTORE_CONFIG), FALSE);
      SetWindowText (GetDlgItem (hDlg, IDC_RESTORE_CONFIG), L" Restore Config Files");
    }
  }
}

enum {
  SUS_WIZARD_TAB    = 0,
  SUS_FRAMERATE_TAB = 1,
  SUS_GRAPHICS_TAB  = 2,
  SUS_OSD_TAB       = 3,
  SUS_STEAM_TAB     = 4,
  SUS_PLUGINS_TAB   = 5
} tabs;

void
setup_sus_config (HWND hDlg)
{
  HIMAGELIST hImgList =
    ImageList_Create ( 16, 16,
                         ILC_COLOR32 | ILC_HIGHQUALITYSCALE,
                           10, 32 );

  HINSTANCE  hAccessibility = LoadLibrary (L"accessibilitycpl.dll");
  HICON      hIconFramerate = LoadIcon    (hAccessibility, MAKEINTRESOURCE (339));

  HINSTANCE  hDisplay       = LoadLibrary (L"Display.dll");
  HICON      hIconGraphics  = LoadIcon    (hDisplay, MAKEINTRESOURCE (1));

  HINSTANCE  hDDORes        = LoadLibrary (L"DDORes.dll");
  HICON      hIconGamepad   = LoadIcon    (hDDORes, MAKEINTRESOURCE (2207));

  HINSTANCE  hShell32       = LoadLibrary (L"SHELL32.DLL");
  HICON      hIconOSD       = LoadIcon    (hShell32, MAKEINTRESOURCE (22));

  HINSTANCE  hPowerCpl      = LoadLibrary (L"powercpl.dll");
  HICON      hIconPlugin    = LoadIcon    (hPowerCpl, MAKEINTRESOURCE (507));

  HICON      hIconWizard    = LoadIcon    (hShell32, MAKEINTRESOURCE (324));

  ImageList_AddIcon (hImgList, hIconWizard);
  ImageList_AddIcon (hImgList, hIconFramerate);
  ImageList_AddIcon (hImgList, hIconGraphics);
  ImageList_AddIcon (hImgList, hIconOSD);
  ImageList_AddIcon (hImgList, hIconPlugin);
  ImageList_AddIcon (hImgList, steam_icon);

  DestroyIcon (hIconWizard);
  DestroyIcon (hIconFramerate);
  DestroyIcon (hIconGraphics);
  DestroyIcon (hIconOSD);
  DestroyIcon (hIconPlugin);
  DestroyIcon (hIconGamepad);

  TabCtrl_SetImageList (GetDlgItem (hDlg, IDC_SUS_TABS), hImgList);

  FreeLibrary (hAccessibility);
  FreeLibrary (hDisplay);
  FreeLibrary (hDDORes);
  FreeLibrary (hShell32);
  FreeLibrary (hPowerCpl);

  TCITEM wizard_tab;

  wizard_tab.mask    = TCIF_TEXT | TCIF_PARAM | TCIF_IMAGE;
  wizard_tab.iImage  = 0;
  wizard_tab.lParam  = SUS_WIZARD_TAB;
  wizard_tab.pszText = L"Performance Wizard";

  TabCtrl_InsertItem (GetDlgItem (hDlg, IDC_SUS_TABS), 0, &wizard_tab);

  TCITEM framerate_tab;

  framerate_tab.mask    = TCIF_TEXT | TCIF_PARAM | TCIF_IMAGE;
  framerate_tab.iImage  = 1;
  framerate_tab.lParam  = SUS_FRAMERATE_TAB;
  framerate_tab.pszText = L"Framerate";

  TabCtrl_InsertItem (GetDlgItem (hDlg, IDC_SUS_TABS), 1, &framerate_tab);

  TCITEM graphics_tab;

  graphics_tab.mask    = TCIF_TEXT | TCIF_PARAM | TCIF_IMAGE;
  graphics_tab.iImage  = 2;
  graphics_tab.lParam  = SUS_GRAPHICS_TAB;
  graphics_tab.pszText = L"Graphics";

  TabCtrl_InsertItem (GetDlgItem (hDlg, IDC_SUS_TABS), 2, &graphics_tab);

  TCITEM OSD_tab;

  OSD_tab.mask    = TCIF_TEXT | TCIF_PARAM | TCIF_IMAGE;
  OSD_tab.iImage  = 3;
  OSD_tab.lParam  = SUS_OSD_TAB;
  OSD_tab.pszText = L"OSD";

  TabCtrl_InsertItem (GetDlgItem (hDlg, IDC_SUS_TABS), 3, &OSD_tab);

  TCITEM steam_tab;

  steam_tab.mask    = TCIF_TEXT | TCIF_PARAM | TCIF_IMAGE;
  steam_tab.iImage  = 5;
  steam_tab.lParam  = SUS_STEAM_TAB;
  steam_tab.pszText = L"Steam";

  TabCtrl_InsertItem (GetDlgItem (hDlg, IDC_SUS_TABS), 4, &steam_tab);

  TCITEM plugins_tab;

  plugins_tab.mask    = TCIF_TEXT | TCIF_PARAM | TCIF_IMAGE;
  plugins_tab.iImage  = 4;
  plugins_tab.lParam  = SUS_PLUGINS_TAB;
  plugins_tab.pszText = L"Plug-Ins";

  TabCtrl_InsertItem (GetDlgItem (hDlg, IDC_SUS_TABS), 5, &plugins_tab);
}

using namespace ds3t;
using namespace ds3t::UI;

HWND hDlgMain;

INT_PTR
CALLBACK
Config (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  hDlgMain = hDlg;

  switch (message)
  {
    case WM_INITDIALOG:
    {
      SendMessage (hDlg, WM_SETICON, ICON_BIG,   (LPARAM)ds3t_icon);
      SendMessage (hDlg, WM_SETICON, ICON_SMALL, (LPARAM)ds3t_icon);

      // Wow this code is ugly, it all needs to be wrapped...
      HINSTANCE hShell32 = LoadLibrary (L"shell32.dll");
      HICON     hIcon    = LoadIcon    (hShell32, MAKEINTRESOURCE (16761));

      SIZE size;
      SendMessage (GetDlgItem (hDlg, IDOK), BM_SETIMAGE, IMAGE_ICON, (LPARAM)hIcon);
      Button_GetIdealSize (GetDlgItem (hDlg, IDOK), &size);
      SetWindowPos (GetDlgItem (hDlg, IDOK), NULL, 0, 0, size.cx + 6, size.cy, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);

      hIcon = LoadIcon (hShell32, MAKEINTRESOURCE (200));

      SendMessage (GetDlgItem (hDlg, IDCANCEL), BM_SETIMAGE, IMAGE_ICON, (LPARAM)hIcon);
      Button_GetIdealSize (GetDlgItem (hDlg, IDCANCEL), &size);
      SetWindowPos (GetDlgItem (hDlg, IDCANCEL), NULL, 0, 0, size.cx + 6, size.cy, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);

      hIcon = LoadIcon (hShell32, MAKEINTRESOURCE (145));
      SendMessage (GetDlgItem (hDlg, IDC_NUKE_CONFIG), BM_SETIMAGE, IMAGE_ICON, (LPARAM)hIcon);

      hIcon = LoadIcon (hShell32, MAKEINTRESOURCE (16771));
      SendMessage (GetDlgItem (hDlg, IDC_BACKUP_CONFIG), BM_SETIMAGE, IMAGE_ICON, (LPARAM)hIcon);

      hIcon = LoadIcon (hShell32, MAKEINTRESOURCE (16741));
      SendMessage (GetDlgItem (hDlg, IDC_RESTORE_CONFIG), BM_SETIMAGE, IMAGE_ICON, (LPARAM)hIcon);

      FreeLibrary (hShell32);

      SetWindowText (hDlg, L"Dark Souls 3 Tweak v " DS3T_VERSION_STR);

      hWndApp = hDlg;

      NVAPI::InitializeLibrary ();

      // Make sure if the config file does not alread exist, we generate one before trying to parse it...
      DS3T_InitializeConfig ();

      std::wstring config_path (DS3T_GetRoamingAppDataDir () + L"\\DarkSoulsIII\\");
      config.load (config_path);

      bool sus_installed = false;

      if (! (config.get_file_sus  ()->get_sections ().empty () ||
             config.get_file_dxgi ()->get_sections ().empty ()))
        sus_installed = true;

      // Setup the config variables on first load only
      if (first_load) {
        refresh_rate =
          static_cast <ParameterInt *> (
            g_ParameterFactory.create_parameter <int> (L"Refresh Rate")
          );

        res_x =
          static_cast <ParameterInt *> (
            g_ParameterFactory.create_parameter <int> (L"X Resolution")
          );

        res_y =
          static_cast <ParameterInt *> (
            g_ParameterFactory.create_parameter <int> (L"Y Resolution")
          );

        use_vsync =
          static_cast <ParameterInt *> (
            g_ParameterFactory.create_parameter <int> (L"Use VSYNC")
          );

        // In some APIs this is a float, but let's just keep things simple (int).
        anisotropy =
          static_cast <ParameterInt *> (
            g_ParameterFactory.create_parameter <int> (L"Maximum Anisotropic Filter")
          );

        texture_res =
          static_cast <ParameterInt *> (
            g_ParameterFactory.create_parameter <int> (L"Texture Resolution Level")
          );

        xml_ssao =
          static_cast <ParameterStringW *> (
            g_ParameterFactory.create_parameter <std::wstring> (L"SSAO")
          );

        xml_shadow_quality =
          static_cast <ParameterStringW *> (
            g_ParameterFactory.create_parameter <std::wstring> (L"Shadow Quality")
          );

        xml_dof =
          static_cast <ParameterStringW *> (
            g_ParameterFactory.create_parameter <std::wstring> (L"Depth of Field")
          );

        xml_motion_blur =
          static_cast <ParameterStringW *> (
            g_ParameterFactory.create_parameter <std::wstring> (L"Motion Blur")
          );

        xml_lighting_quality =
          static_cast <ParameterStringW *> (
            g_ParameterFactory.create_parameter <std::wstring> (L"Lighting Quality")
          );

        xml_effects_quality =
          static_cast <ParameterStringW *> (
            g_ParameterFactory.create_parameter <std::wstring> (L"Effects Quality")
          );

        xml_reflection_quality =
          static_cast <ParameterStringW *> (
            g_ParameterFactory.create_parameter <std::wstring> (L"Reflection Quality")
          );

        xml_water_quality =
          static_cast <ParameterStringW *> (
            g_ParameterFactory.create_parameter <std::wstring> (L"Water Quality")
          );

        xml_shader_quality =
          static_cast <ParameterStringW *> (
            g_ParameterFactory.create_parameter <std::wstring> (L"Shader Quality")
          );

        if (sus_installed) {
          ds3t::INI::File* sus_ini =
            config.get_file_sus ();

          ds3t::INI::File* dxgi_ini =
            config.get_file_dxgi ();

          aspect_ratio_correction =
            static_cast <ds3t::ParameterBool *> (
              ds3t::g_ParameterFactory.create_parameter <bool> (
                L"Aspect Ratio Correction"
              )
            );
          aspect_ratio_correction->register_to_ini ( sus_ini,
                                                       L"SUS.Render",
                                                         L"CorrectAspect" );

          arc_start_fullscreen =
            static_cast <ds3t::ParameterBool *> (
              ds3t::g_ParameterFactory.create_parameter <bool> (
                L"Start In Fullscreen Mode (for ARC)"
              )
            );
          arc_start_fullscreen->register_to_ini ( sus_ini,
                                                    L"SUS.Render",
                                                      L"StartFullscreen" );

          borderless_window =
            static_cast <ds3t::ParameterBool *> (
              ds3t::g_ParameterFactory.create_parameter <bool> (
                L"Borderless Window Mode"
              )
            );
          borderless_window->register_to_ini ( sus_ini,
                                                       L"SUS.Window",
                                                         L"Borderless" );

          fullscreen_window =
            static_cast <ds3t::ParameterBool *> (
              ds3t::g_ParameterFactory.create_parameter <bool> (
                L"Fullscreen Borderless Window Mode"
              )
            );
          fullscreen_window->register_to_ini ( sus_ini,
                                                 L"SUS.Window",
                                                   L"Fullscreen" );

          center_window =
            static_cast <ds3t::ParameterBool *> (
              ds3t::g_ParameterFactory.create_parameter <bool> (
                L"Center Windows"
              )
            );
          center_window->register_to_ini ( sus_ini,
                                             L"SUS.Window",
                                               L"Center" );

          flip_mode =
            static_cast <ds3t::ParameterBool *> (
              ds3t::g_ParameterFactory.create_parameter <bool> (
                L"Use Flip Mode"
              )
            );
          flip_mode->register_to_ini ( sus_ini,
                                         L"SUS.Render",
                                           L"FlipMode" );

          presentation_interval =
            static_cast <ds3t::ParameterInt *> (
              ds3t::g_ParameterFactory.create_parameter <int> (
                L"Presentation Interval (VSYNC)"
              )
            );
          presentation_interval->register_to_ini ( dxgi_ini,
                                                     L"Render.DXGI",
                                                       L"PresentationInterval" );
        }
      }

      if (sus_installed) {
        aspect_ratio_correction->load ();
        arc_start_fullscreen->load    ();

        borderless_window->load ();
        fullscreen_window->load ();
        center_window->load     ();

        flip_mode->load             ();
        presentation_interval->load ();
      }

      ////refresh_rate->register_to_cfg (config.get_file (), L"refreshRateHz");
      ////refresh_rate->load ();

      res_x->register_to_xml (ds3t::XML::ds3_root, L"Resolution-WindowScreenWidth");
      res_x->load ();

      res_y->register_to_xml (ds3t::XML::ds3_root, L"Resolution-WindowScreenHeight");
      res_y->load ();

      if (sus_installed) {
        res_x->register_to_ini ( config.get_file_sus  (),
                                   L"SUS.Render",
                                     L"DefaultResX" );
        res_y->register_to_ini ( config.get_file_sus  (),
                                   L"SUS.Render",
                                     L"DefaultResY" );
      }

      Button_SetCheck ( GetDlgItem (hDlg, IDC_VSYNC),
                        presentation_interval->get_value () > 0 );

      ////anisotropy->register_to_cfg (config.get_file (), L"anisotropy");
      ////anisotropy->load ();

      //texture_res->register_to_ini (settings.get_file (), L"SystemSettings", L"TextureResolution");
      //texture_res->load ();

      xml_shadow_quality->register_to_xml (ds3t::XML::ds3_root, L"ShadowQuality");
      xml_shadow_quality->load ();

      xml_ssao->register_to_xml (ds3t::XML::ds3_root, L"SSAO");
      xml_ssao->load ();

      xml_dof->register_to_xml (ds3t::XML::ds3_root, L"DepthOfField");
      xml_dof->load ();

      xml_motion_blur->register_to_xml (ds3t::XML::ds3_root, L"MotionBlur");
      xml_motion_blur->load ();

      xml_lighting_quality->register_to_xml (ds3t::XML::ds3_root, L"LightingQuality");
      xml_lighting_quality->load ();

      xml_effects_quality->register_to_xml (ds3t::XML::ds3_root, L"EffectsQuality");
      xml_effects_quality->load ();

      xml_reflection_quality->register_to_xml (ds3t::XML::ds3_root, L"ReflectionQuality");
      xml_reflection_quality->load ();

      xml_water_quality->register_to_xml (ds3t::XML::ds3_root, L"WaterSurfaceQuality");
      xml_water_quality->load ();

      xml_shader_quality->register_to_xml (ds3t::XML::ds3_root, L"ShadeQuality");
      xml_shader_quality->load ();

      //antialiasing->register_to_cfg (config.get_file (), L"antialiasing");
      //antialiasing->load ();

#if 0
      //
      // Now, for the non-game settings -- [BMT.User]
      //
      if (first_load) { // Keep this preference across configuration restorations
        decline_backup =
          static_cast <ParameterBool *> (
            g_ParameterFactory.create_parameter <bool> (L"NoBackup")
          );

        decline_backup->register_to_ini (settings.get_file (), L"BMT.User", L"NoBackup");
        decline_backup->load ();
      }
      else {
        // This isn't enough, we need to actually save BmSystemSettings.ini to make sure
        //   config files retain this preference -- @FIXME: Persistent backup preference.
        decline_backup->store ();
      }
#endif

      setup_resolution       (hDlg);
      setup_tex_filter       (hDlg);
      setup_tex_res          (hDlg);

      setup_ssao               (hDlg);
      setup_dof                (hDlg);
      setup_motion_blur        (hDlg);
      setup_shadow_quality     (hDlg);
      setup_lighting_quality   (hDlg);
      setup_effects_quality    (hDlg);
      setup_reflection_quality (hDlg);
      setup_water_quality      (hDlg);
      setup_shader_quality     (hDlg);

      setup_driver_tweaks    (hDlg);

      //setup_config_status    (hDlg);
      //setup_framerate_limiting (hDlg);

      std::wstring gpu_str = DXGI::GetGPUInfo ();

      HFONT font = CreateFont (11, 0, 0, 0, FW_LIGHT, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_NATURAL_QUALITY, MONO_FONT, L"Consolas");

      SetWindowFont (GetDlgItem (hDlg, IDC_GPUINFO), font, true);
      Edit_SetText (GetDlgItem (hDlg, IDC_GPUINFO), gpu_str.c_str ());

      SetActiveWindow (GetDlgItem (hDlg, IDOK));
      SetFocus        (GetDlgItem (hDlg, IDOK));

      EnableWindow    (GetDlgItem (hDlg, IDC_TEXTURE_RES), FALSE);

      int window_mode =
        config.lookup_value (L"ScreenMode") == std::wstring (L"FULLSCREEN");

      if (sus_installed && (! window_mode) && (! borderless_window->get_value ())) {
        if (arc_start_fullscreen->get_value ())
          window_mode = 1;
      }

      // @TODO: Radio button wrapper, doing it this way is just plain ridiculous!

      EnableWindow (GetDlgItem (hDlg, IDC_BORDER_WINDOW), TRUE);

      if (sus_installed) {
        EnableWindow (GetDlgItem (hDlg, IDC_BORDERLESS_WINDOW), TRUE);
        EnableWindow (GetDlgItem (hDlg, IDC_VSYNC),             TRUE);
      } else {
        EnableWindow (GetDlgItem (hDlg, IDC_BORDERLESS_WINDOW), FALSE);
        EnableWindow (GetDlgItem (hDlg, IDC_VSYNC),             FALSE);
      }

      if (window_mode == 0) {
        if ((! sus_installed) || (! borderless_window->get_value ())) {
          mode = 0;
          handle_window_radios (hDlg, IDC_BORDER_WINDOW);
        } else {
          if (fullscreen_window->get_value ())
            mode = 1;
          else
            mode = 3;
          handle_window_radios (hDlg, IDC_BORDERLESS_WINDOW);
        }
      }

      else {
        bool fullscreen = (! (sus_installed && aspect_ratio_correction->get_value ()));

        if (! fullscreen)
          fullscreen = (arc_start_fullscreen->get_value ());

        if (fullscreen) {
          mode = 2;
          handle_window_radios (hDlg, IDC_FULLSCREEN);
        }
      }

#if 0
      if (sus_installed) {
        HWND hWndFullscreen = GetDlgItem (hDlg, IDC_FULLSCREEN);

        if (aspect_ratio_correction->get_value ()) {
          extern int  mode;

          if (mode == 2) {
            mode = 1;
            handle_window_radios (hDlgMain, IDC_BORDERLESS_WINDOW);
          }
        }

        // We cannot allow fullscreen mode if aspect ratio correction is enabled
        Button_Enable (hWndFullscreen, aspect_ratio_correction->get_value () == 0);
      }
#endif

      // Leave an empty space where the Souls Unsqueezed Config tab dialog should
      //   be if DarkSoulsIII.exe is not run from the correct directory.
      if (! (config.get_file_sus  ()->get_sections ().empty () ||
             config.get_file_dxgi ()->get_sections ().empty ())) {
        setup_sus_config   (hDlg);
///////        hWndSUSTab = CreateDialog (GetWindowInstance (hDlg), MAKEINTRESOURCE (IDD_AUDIO), hDlg, AudioConfig);
      } else {
        EnableWindow (GetDlgItem (hDlg, IDC_SUS_TABS), FALSE);
      }

      first_load = false;

      return (INT_PTR)TRUE;
    }

    case WM_MOVE:
    {
      RECT pos;
      GetWindowRect (GetDlgItem (hDlg, IDC_SUS_TABS), &pos);

      RECT rc;
      GetClientRect      (GetDlgItem (hDlg, IDC_SUS_TABS), &rc);
      TabCtrl_AdjustRect (GetDlgItem (hDlg, IDC_SUS_TABS), FALSE, &rc);
      SetWindowPos       (hWndSUSTab, hDlg, pos.left + rc.left - 3, pos.top + rc.top - 2, rc.right - rc.left + 4, rc.bottom - rc.top + 4, SWP_SHOWWINDOW);
    } break;

    case WM_NOTIFY:
    {
      switch (((LPNMHDR)lParam)->code)
      {
        case TCN_SELCHANGING:
        {
          if (hWndSUSTab != 0) {
            SendMessage (hWndSUSTab, WM_COMMAND, MAKEWPARAM (IDOK, 0), 0);
            hWndSUSTab = 0;
          }
        } break;

        case TCN_SELCHANGE:
        {
          int sel = TabCtrl_GetCurSel (GetDlgItem (hDlg, IDC_SUS_TABS));

          HINSTANCE hInstance = GetWindowInstance (hDlg);

          if (sel == SUS_WIZARD_TAB) {
/////            hWndSUSTab = CreateDialog (GetWindowInstance (hDlg), MAKEINTRESOURCE (IDD_PERFORMANCE), hDlg, PerformanceWizard);
          } else if (sel == SUS_FRAMERATE_TAB) {
            hWndSUSTab = CreateDialog (GetWindowInstance (hDlg), MAKEINTRESOURCE (IDD_FRAMERATE), hDlg, FramerateConfig);
          } else if (sel == SUS_GRAPHICS_TAB) {
            hWndSUSTab = CreateDialog (GetWindowInstance (hDlg), MAKEINTRESOURCE (IDD_GRAPHICS), hDlg, GraphicsConfig);
          } else if (sel == SUS_STEAM_TAB) {
            hWndSUSTab = CreateDialog (GetWindowInstance (hDlg), MAKEINTRESOURCE (IDD_STEAM), hDlg, SteamConfig);
          } else if (sel == SUS_OSD_TAB) {
            hWndSUSTab = CreateDialog (GetWindowInstance (hDlg), MAKEINTRESOURCE (IDD_OSD), hDlg, OSDConfig);
          } else if (sel == SUS_PLUGINS_TAB) {
            hWndSUSTab = CreateDialog (GetWindowInstance (hDlg), MAKEINTRESOURCE (IDD_PLUGINS), hDlg, PluginsConfig);
          }

          RECT pos;
          GetWindowRect (GetDlgItem (hDlg, IDC_SUS_TABS), &pos);

          RECT rc;
          GetClientRect      (GetDlgItem (hDlg, IDC_SUS_TABS), &rc);
          TabCtrl_AdjustRect (GetDlgItem (hDlg, IDC_SUS_TABS), FALSE, &rc);
          SetWindowPos       (hWndSUSTab, hDlg, pos.left + rc.left - 3, pos.top + rc.top - 2, rc.right - rc.left + 4, rc.bottom - rc.top + 4, SWP_SHOWWINDOW);
        } break;
      }
    } break;

    case WM_LBUTTONDBLCLK:
    {
      bool debug = IsWindowVisible (GetDlgItem (hDlg, IDC_DEBUG_GROUP));
      debug = !debug;

      setup_debug_utils (hDlg, debug);
    } break;

    case WM_COMMAND:
    {
      // What insanity is this?! The message pump shouldn't be working while a message box is up!
      if (messagebox_active) // Ignore button clicks while a Message Box is active
        return 0;

      if (LOWORD (wParam) == IDC_BORDERLESS_WINDOW || 
          LOWORD (wParam) == IDC_BORDER_WINDOW     ||
          LOWORD (wParam) == IDC_FULLSCREEN) {
        handle_window_radios (hDlg, LOWORD (wParam));
      }
      if (LOWORD (wParam) == IDC_MATCH_DESKTOP) {
        DEVMODE dmNow;

        memset (&dmNow, 0, sizeof (DEVMODE));
            dmNow.dmSize = sizeof (DEVMODE);

        EnumDisplaySettings (NULL, ENUM_CURRENT_SETTINGS, &dmNow);

        res_x->set_value        (dmNow.dmPelsWidth);
        res_y->set_value        (dmNow.dmPelsHeight);
        refresh_rate->set_value (dmNow.dmDisplayFrequency);

        setup_resolution (hDlg);
      }

      if (LOWORD (wParam) == IDC_NV_DRIVER_TWEAKS) {
        NVAPI::ds3t_fullscreen = (mode == 2); // This is a hack based on another hack (see handle_window_radios), yuck.
        return (int)DialogBox (GetWindowInstance (hDlg), MAKEINTRESOURCE (IDD_DRIVER_TWEAKS), hDlg, DriverConfigNV);
      }

      if (LOWORD (wParam) == IDC_RESOLUTION) {
        if (HIWORD (wParam) == CBN_SELCHANGE) {
          int x, y, refresh;
          get_resolution (hDlg, &x, &y, &refresh);

          res_x->set_value (x);
          res_y->set_value (y);

          // Set the refresh rate whenever the resolution changes...
          setup_resolution (hDlg);
        }
      }

      // Update refresh rate immediately
      if (LOWORD (wParam) == IDC_REFRESH_RATE) {
        if (HIWORD (wParam) == CBN_SELCHANGE) {
          int x, y, refresh;
          get_resolution (hDlg, &x, &y, &refresh);
          refresh_rate->set_value (refresh);
        }
      }

      if (LOWORD (wParam) == IDC_NUKE_CONFIG)
      {
        if (MessageBox (NULL, L"WARNING: This will delete all game settings and restore you to default settings the next time you run the game.\n\n"
                              L"\tDo you really wish to continue?",
                              L"Please Confirm An Irreversible Operation",
                              MB_OKCANCEL | MB_ICONEXCLAMATION) == IDOK) {
          DS3T_DeleteAllConfigFiles ();
          EndDialog (hDlg, LOWORD (wParam));
          return (INT_PTR)TRUE;
        }
      }

      if (LOWORD (wParam) == IDC_RESTORE_CONFIG)
      {
        int status =
          DS3T_MessageBox (L"This will discard any changes you've made and restore configuration to an earlier state.\n\n"
                           L"\tDo you wish to continue?",
                           L"Restore Configuration From Backup?",
            MB_YESNO | MB_ICONQUESTION);

        if (status == IDYES) {

          HINSTANCE hInst = GetWindowInstance (hDlg);

          DS3T_RestoreConfigFiles ();
          EndDialog (hDlg, LOWORD (wParam));

          // We're effectively going to recurse here, there's a slim
          //  possibility of a stack overflow if the user does this enough.
          return (int)DialogBox (hInst, MAKEINTRESOURCE (IDD_DS3T), NULL, Config);
        }
      }

      if (LOWORD (wParam) == IDC_BACKUP_CONFIG)
      {
        bool allow_backup = true;

        // Confirm before overwriting an existing backup.
        if (DS3T_HasBackupConfigFiles ()) {
          int status =
            DS3T_MessageBox (L"A previous backup already exists.\n\n"
                             L"\tWould you like to replace it?",
                             L"Replace Existing Configuration Backup?",
               MB_YESNO | MB_ICONQUESTION);

          if (status != IDYES)
            allow_backup = false;
        }

        // Honor the user's selection
        if (allow_backup)
          DS3T_CreateBackupConfig ();

        // Basically just refresh the file timestamps
        setup_debug_utils (hDlg, true);
      }

      if (LOWORD (wParam) == IDCANCEL)
      {
        NVAPI::UnloadLibrary ();
        EndDialog (hDlg, LOWORD (wParam));
        return (INT_PTR)TRUE;
      }

      else if (LOWORD (wParam) == IDOK) {
        int x, y, refresh;
        get_resolution (hDlg, &x, &y, &refresh);

        res_x->set_value    (x);
        res_x->store        ();

        res_y->set_value    (y);
        res_y->store        ();

        // Duplicate the X and Y resolution for both Window and Fullscren
        config.set_value ( L"Resolution-FullScreenWidth",
                             res_x->get_value_str () );
        config.set_value ( L"Resolution-FullScreenHeight",
                             res_y->get_value_str () );

        refresh_rate->set_value (refresh);
        refresh_rate->store     ();

        presentation_interval->set_value (
          Button_GetCheck ( GetDlgItem (hDlg, IDC_VSYNC) )
        );

        presentation_interval->store ();

        use_vsync->store    ();

        anisotropy->set_value (get_tex_filter (hDlg));
        anisotropy->store     ();

        ////

        xml_ssao->set_value (get_ssao (hDlg));
        xml_ssao->store     ();

        xml_dof->set_value (get_dof (hDlg));
        xml_dof->store     ();

        xml_motion_blur->set_value (get_motion_blur (hDlg));
        xml_motion_blur->store     ();

        xml_shadow_quality->set_value (get_shadow_quality (hDlg));
        xml_shadow_quality->store     ();

        xml_lighting_quality->set_value (get_lighting_quality (hDlg));
        xml_lighting_quality->store     ();

        xml_effects_quality->set_value (get_effects_quality (hDlg));
        xml_effects_quality->store     ();

        xml_reflection_quality->set_value (get_reflection_quality (hDlg));
        xml_reflection_quality->store     ();

        xml_water_quality->set_value (get_water_quality (hDlg));
        xml_water_quality->store     ();

        xml_shader_quality->set_value (get_shader_quality (hDlg));
        xml_shader_quality->store     ();

        //xml_antialiasing->set_value   (get_antialiasing (hDlg));
        //xml_antialiasing->store       ();

        texture_res->set_value (get_tex_res (hDlg));
        texture_res->store     ();

        int window_mode = 0;

        if (Button_GetCheck (GetDlgItem (hDlg, IDC_BORDER_WINDOW))) {
          window_mode = 0;
          config.set_value (L"ScreenMode", L"WINDOW");

          if (borderless_window != nullptr) {
            borderless_window->set_value (false);
            borderless_window->store     ();
          }
        }

        if (Button_GetCheck (GetDlgItem (hDlg, IDC_BORDERLESS_WINDOW)) == 1) {
          window_mode = 1;
          ///display_mode_val->value (L"1");
          config.set_value (L"ScreenMode", L"WINDOW");

          if (borderless_window != nullptr) {
            borderless_window->set_value (true);
            borderless_window->store     ();

            fullscreen_window->set_value (false);
            fullscreen_window->store     ();
          }
        }

        // We created a custom 4th mode, in which BMT changes the desktop resolution...
        if (Button_GetCheck (GetDlgItem (hDlg, IDC_BORDERLESS_WINDOW)) == 2) {
          window_mode = 1;
          config.set_value (L"ScreenMode", L"WINDOW");

          if (borderless_window != nullptr) {
            borderless_window->set_value (true);
            borderless_window->store     ();

            fullscreen_window->set_value (true);
            fullscreen_window->store     ();
          }
        }

        if (Button_GetCheck (GetDlgItem (hDlg, IDC_FULLSCREEN))) {
          window_mode = 2;
          ///display_mode_val->value (L"2");

          if ((! aspect_ratio_correction) || (! aspect_ratio_correction->get_value ()))
            config.set_value (L"ScreenMode", L"FULLSCREEN");
          else {
            config.set_value (L"ScreenMode", L"WINDOW");
            arc_start_fullscreen->set_value (1);
            arc_start_fullscreen->store     ();
          }

          if (borderless_window != nullptr) {
            borderless_window->set_value (false);
            borderless_window->store     ();

            fullscreen_window->set_value (false);
            fullscreen_window->store     ();
          }
        }

        // Do not automagically engage the fullscreen mode switch
        //   at application start...
        if (window_mode != 2 && arc_start_fullscreen != nullptr) {
          arc_start_fullscreen->set_value (false);
          arc_start_fullscreen->store     ();
        }

        bool cancel = false;

        // System to make an automatic backup if INI files at save-time
        if (! DS3T_HasBackupConfigFiles ()) {
          // So, no backups exist - maybe the user doesn't want backups?
          //if (! decline_backup->get_value ()) {
            //int status =
              //DS3T_MessageBox (L"No backup configuration files were detected, would you like to backup your configuration now?\n\n\tIf you press [No], you will never be prompted again.",
                               //L"Create Backup Configuration Files?",
                               //MB_YESNOCANCEL | MB_ICONQUESTION);

            //if (status == IDCANCEL)
              //return (INT_PTR)TRUE; 

            //if (status == IDYES) {
              DS3T_CreateBackupConfig    ();
              //decline_backup->set_value (false);
            //}
            //else {
              //decline_backup->set_value (true);
            //}

            //decline_backup->store ();
          //}
        }

        // Close the currently open tab so that its config is saved.
        if (hWndSUSTab != 0) {
          SendMessage (hWndSUSTab, WM_COMMAND, MAKEWPARAM (IDOK, 0), 0);
        }

        std::wstring config_path (DS3T_GetRoamingAppDataDir () + L"\\DarkSoulsIII\\");
        config.save (config_path);

        NVAPI::UnloadLibrary ();

        EndDialog (hDlg, TRUE);
        return (INT_PTR)TRUE;
      }
    } break;

    case WM_DESTROY:
    {
      NVAPI::UnloadLibrary ();
      //int ret = MessageBox (NULL, L"Quit Without Saving?", L"Settings Have Not Been Saved", MB_YESNOCANCEL);

      //if (ret == IDOK)
        PostQuitMessage (0);
      //else

    } break;
  }

  return (INT_PTR)FALSE;
}