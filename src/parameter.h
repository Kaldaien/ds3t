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

#ifndef __DS3T__PARAMETER_H__
#define __DS3T__PARAMETER_H__

#include "cfg.h"
#include "ini.h"
#include "xml.h"

#include <Windows.h>
#include <vector>

namespace ds3t {
  namespace UI {

class Control {
  public:
    Control (void) {
      handle = NULL;
    }

    virtual std::wstring get_value_str (void) = 0;
    virtual void         set_value_str (std::wstring val_str) = 0;

    BOOL         exists (void) { return IsWindow (handle); }

  protected:
    HWND    handle;

  private:
};

class EditBox : public Control {
public:
  EditBox (HWND hWnd) {
    handle = hWnd;
  }

  virtual std::wstring get_value_str (void);
  virtual void         set_value_str (std::wstring val_str);
};

class CheckBox : public Control {
public:
  CheckBox (HWND hWnd) {
    handle  = hWnd;
    numeric = false; // When true, input will be 1/0 and output will be 1/0
  }

  virtual std::wstring get_value_str (void);
  virtual void         set_value_str (std::wstring val_str);

private:
  bool numeric;
};

};

class iParameter {
public:
  iParameter (void) {
    xml_node   = nullptr;
    ini        = nullptr;
    ui_control = nullptr;
  }

  virtual std::wstring get_value_str (void) = 0;
  virtual void         set_value_str (std::wstring str) = 0;

  // Read value from CFG
  bool load (void)
  {
    if (xml_node != nullptr) {
      set_value_str (xml_node->value ());
      return true;
    }

    if (ini != nullptr) {
      INI::File::Section& section = ini->get_section (ini_section);

      if (section.contains_key (ini_key)) {
        set_value_str (section.get_value (ini_key));
        return true;
      }
    }

    return false;
  }

  // Store value in CFG or INI
  bool store (void)
  {
    bool ret = false;

    if (ui_control != nullptr)
      set_value_str (ui_control->get_value_str ());

    wcsncpy_s (backing_string, get_value_str ().c_str (), MAX_PATH);

    if (xml_node != nullptr) {
      xml_node->value (backing_string);
      ret = true;
    }

    if (ini != nullptr) {
      INI::File::Section& section = ini->get_section (ini_section);

      if (section.contains_key (ini_key)) {
        section.get_value (ini_key) = backing_string;
        ret = true;
      }

      // Add this key/value if it doesn't already exist.
      else {
        section.add_key_value (ini_key, backing_string);
        ret = true;// +1;
      }
    }

    return ret;
  }

  void register_to_xml (xml_node <wchar_t>* node, std::wstring attrib_name)
  {
    xml_node = XML::FindNode (node, attrib_name);
  }

  void register_to_ini (INI::File* file, std::wstring section, std::wstring key)
  {
    ini         = file;
    ini_section = section;
    ini_key     = key;
  }

  void bind_to_control (UI::Control* ui_ctl)
  {
    ui_control = ui_ctl;
  }

protected:
  UI::Control*             ui_control;
  wchar_t                  backing_string [MAX_PATH]; // Required by XML

private:
  rapidxml::xml_node<wchar_t>*
                           xml_node;

  INI::File*               ini;
  std::wstring             ini_section;
  std::wstring             ini_key;
};

template <typename _T>
class Parameter : public iParameter {
public:
  virtual std::wstring get_value_str (void) = 0;
  virtual _T           get_value     (void) = 0;

  virtual void         set_value     (_T val)           = 0;
  virtual void         set_value_str (std::wstring str) = 0;

protected:
  _T                       value;
};

class ParameterInt : public Parameter <int>
{
public:
  std::wstring get_value_str (void);
  int          get_value     (void);

  void         set_value     (int val);
  void         set_value_str (std::wstring str);

protected:
  int value;
};

class ParameterInt64 : public Parameter <int64_t>
{
public:
  std::wstring get_value_str (void);
  int64_t      get_value (void);

  void         set_value (int64_t val);
  void         set_value_str (std::wstring str);

protected:
  int64_t value;
};

class ParameterBool : public Parameter <bool>
{
public:
  std::wstring get_value_str (void);
  bool         get_value     (void);

  void         set_value     (bool val);
  void         set_value_str (std::wstring str);

protected:
  bool value;
};

class ParameterFloat : public Parameter <float>
{
public:
  std::wstring get_value_str (void);
  float        get_value (void);

  void         set_value (float val);
  void         set_value_str (std::wstring str);

protected:
  float value;
};

class ParameterStringW : public Parameter <std::wstring>
{
public:
  std::wstring get_value_str (void);
  std::wstring get_value     (void);

  void         set_value     (std::wstring str);
  void         set_value_str (std::wstring str);

protected:
  std::wstring value;
};

class ParameterFactory {
public:
  template <typename _T> iParameter* create_parameter  (const wchar_t* name);
protected:
private:
  std::vector <iParameter *> params;
} extern g_ParameterFactory;

}


extern ds3t::ParameterInt*     refresh_rate;
extern ds3t::ParameterInt*     res_x;
extern ds3t::ParameterInt*     res_y;

extern ds3t::ParameterBool*    aspect_ratio_correction;
extern ds3t::ParameterBool*    arc_start_fullscreen;

extern ds3t::ParameterStringW* xml_texture_quality;
extern ds3t::ParameterStringW* xml_ssao;
extern ds3t::ParameterStringW* xml_dof;
extern ds3t::ParameterStringW* xml_motion_blur;
extern ds3t::ParameterStringW* xml_shadow_quality;
extern ds3t::ParameterStringW* xml_lighting_quality;
extern ds3t::ParameterStringW* xml_effects_quality;
extern ds3t::ParameterStringW* xml_reflection_quality;
extern ds3t::ParameterStringW* xml_water_quality;
extern ds3t::ParameterStringW* xml_shader_quality;

//extern ds3t::ParameterInt* max_fps;

extern ds3t::ParameterInt*     use_vsync;
extern ds3t::ParameterInt*     presentation_interval;
extern ds3t::ParameterBool*    flip_mode;

extern ds3t::ParameterBool*    center_window;
extern ds3t::ParameterBool*    fullscreen_window;
extern ds3t::ParameterBool*    borderless_window;

extern ds3t::ParameterInt*     anisotropy;
extern ds3t::ParameterInt*     texture_res;

// This controls the auto-backup behavior (it's another non-game setting)
extern ds3t::ParameterBool*    decline_backup;

#endif /* __DS3T__PARAMETER_H__ */