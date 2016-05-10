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
#define _CRT_NON_CONFORMING_SWPRINTFS

#include "parameter.h"

std::wstring
ds3t::ParameterInt::get_value_str (void)
{
  wchar_t str [32];
  _itow (value, str, 10);

  return std::wstring (str);
}

int
ds3t::ParameterInt::get_value (void)
{
  return value;
}

void
ds3t::ParameterInt::set_value (int val)
{
  value = val;

  if (ui_control != nullptr)
    ui_control->set_value_str (get_value_str ());
}


void
ds3t::ParameterInt::set_value_str (std::wstring str)
{
  value = _wtoi (str.c_str ());

  if (ui_control != nullptr) {
    ui_control->set_value_str (get_value_str ());
  }
}


std::wstring
ds3t::ParameterInt64::get_value_str (void)
{
  wchar_t str [32];
  _i64tow (value, str, 10);

  return std::wstring (str);
}

int64_t
ds3t::ParameterInt64::get_value (void)
{
  return value;
}

void
ds3t::ParameterInt64::set_value (int64_t val)
{
  value = val;

  if (ui_control != nullptr)
    ui_control->set_value_str (get_value_str ());
}


void
ds3t::ParameterInt64::set_value_str (std::wstring str)
{
  value = _wtol (str.c_str ());

  if (ui_control != nullptr) {
    ui_control->set_value_str (get_value_str ());
  }
}


std::wstring
ds3t::ParameterBool::get_value_str (void)
{
  if (value == true)
    return L"true";

  return L"false";
}

bool
ds3t::ParameterBool::get_value (void)
{
  return value;
}

void
ds3t::ParameterBool::set_value (bool val)
{
  value = val;

  if (ui_control != nullptr)
    ui_control->set_value_str (get_value_str ());
}


void
ds3t::ParameterBool::set_value_str (std::wstring str)
{
  if (str.length () == 1 &&
      str [0] == L'1')
    value = true;

  else if (str.length () == 4 &&
      towlower (str [0]) == L't' &&
      towlower (str [1]) == L'r' &&
      towlower (str [2]) == L'u' &&
      towlower (str [3]) == L'e')
    value = true;

  else
    value = false;

  if (ui_control != nullptr) {
    ui_control->set_value_str (get_value_str ());
  }
}


std::wstring
ds3t::ParameterFloat::get_value_str (void)
{
  wchar_t val_str [16];
  swprintf (val_str, L"%f", value);

  return std::wstring (val_str);
}

float
ds3t::ParameterFloat::get_value (void)
{
  return value;
}

void
ds3t::ParameterFloat::set_value (float val)
{
  value = val;

  if (ui_control != nullptr)
    ui_control->set_value_str (get_value_str ());
}


void
ds3t::ParameterFloat::set_value_str (std::wstring str)
{
  value = (float)wcstod (str.c_str (), NULL);

  if (ui_control != nullptr) {
    ui_control->set_value_str (get_value_str ());
  }
}

#include <Windows.h>
#include <windowsx.h>

std::wstring
ds3t::UI::EditBox::get_value_str (void)
{
  wchar_t val_str [32];
  Edit_GetText (handle, val_str, 32);
  return std::wstring (val_str);
}

void
ds3t::UI::EditBox::set_value_str (std::wstring str)
{
  Edit_SetText (handle, str.c_str ());
}

std::wstring
ds3t::UI::CheckBox::get_value_str (void)
{
  if (Button_GetCheck (handle)) {
    if (numeric)
      return L"1";

    return L"true";
  }
  else {
    if (numeric)
      return L"0";

    return L"false";
  }
}

void
ds3t::UI::CheckBox::set_value_str (std::wstring str)
{
  if (str.length () == 1 &&
      str [0] == L'1') {
    Button_SetCheck (handle, true);
    numeric = true;
  }
  else if (str.length () == 1 &&
           str [0] == L'0') {
    Button_SetCheck (handle, false);
    numeric = true;
  }

  else if (str.length () == 4 &&
      towlower (str [0]) == L't' &&
      towlower (str [1]) == L'r' &&
      towlower (str [2]) == L'u' &&
      towlower (str [3]) == L'e') {
    Button_SetCheck (handle, true);
    numeric = false;
  }
  else {
    Button_SetCheck (handle, false);
    numeric = false;
  }
}


std::wstring
ds3t::ParameterStringW::get_value_str (void)
{
  return value;
}

std::wstring
ds3t::ParameterStringW::get_value (void)
{
  return value;
}

void
ds3t::ParameterStringW::set_value (std::wstring val)
{
  value = val;
}


void
ds3t::ParameterStringW::set_value_str (std::wstring str)
{
  value = str;
}



template <>
ds3t::iParameter*
ds3t::ParameterFactory::create_parameter <int> (const wchar_t* name)
{
  iParameter* param = new ParameterInt ();
  params.push_back (param);

  return param;
}

template <>
ds3t::iParameter*
ds3t::ParameterFactory::create_parameter <int64_t> (const wchar_t* name)
{
  iParameter* param = new ParameterInt64 ();
  params.push_back (param);

  return param;
}

template <>
ds3t::iParameter*
ds3t::ParameterFactory::create_parameter <bool> (const wchar_t* name)
{
  iParameter* param = new ParameterBool ();
  params.push_back (param);

  return param;
}

template <>
ds3t::iParameter*
ds3t::ParameterFactory::create_parameter <float> (const wchar_t* name)
{
  iParameter* param = new ParameterFloat ();
  params.push_back (param);

  return param;
}

template <>
ds3t::iParameter*
ds3t::ParameterFactory::create_parameter <std::wstring> (const wchar_t* name)
{
  iParameter* param = new ParameterStringW ();
  params.push_back (param);

  return param;
}




ds3t::ParameterFactory ds3t::g_ParameterFactory;

ds3t::ParameterInt*     refresh_rate;
ds3t::ParameterInt*     res_x;
ds3t::ParameterInt*     res_y;

ds3t::ParameterBool*    aspect_ratio_correction;


ds3t::ParameterInt*     use_vsync;
ds3t::ParameterInt*     presentation_interval;
ds3t::ParameterBool*    flip_mode;

ds3t::ParameterBool*    center_window;
ds3t::ParameterBool*    fullscreen_window;
ds3t::ParameterBool*    borderless_window;

ds3t::ParameterInt*     anisotropy;
ds3t::ParameterInt*     texture_res;

ds3t::ParameterStringW* xml_texture_quality;
ds3t::ParameterStringW* xml_antialiasing;

ds3t::ParameterStringW* xml_ssao;
ds3t::ParameterStringW* xml_dof;
ds3t::ParameterStringW* xml_motion_blur;
ds3t::ParameterStringW* xml_shadow_quality;
ds3t::ParameterStringW* xml_lighting_quality;
ds3t::ParameterStringW* xml_effects_quality;
ds3t::ParameterStringW* xml_reflection_quality;
ds3t::ParameterStringW* xml_water_quality;
ds3t::ParameterStringW* xml_shader_quality;

//ds3t::ParameterInt*   framerate_limiting;
//ds3t::ParameterInt*   max_fps;

ds3t::ParameterBool*    decline_backup;