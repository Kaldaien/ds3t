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

#pragma once

#include "resource.h"

#include <string>
#include "cfg.h"
#include "ini.h"
#include "utility.h"
#include "xml.h"

class DS3_Config {
public:
           DS3_Config (void) { sus_ini  = nullptr;
                               dxgi_ini = nullptr;
                               rtss_ini = nullptr;
                              };
  virtual ~DS3_Config (void)
  {
    if (sus_ini != nullptr) {
      delete sus_ini;
      sus_ini = nullptr;
    }

    if (dxgi_ini != nullptr) {
      delete dxgi_ini;
      dxgi_ini = nullptr;
    }

    if (rtss_ini != nullptr) {
      delete rtss_ini;
      rtss_ini = nullptr;
    }
  }


  void load (std::wstring path) {
    ds3t::XML::LoadXML ();

    sus_ini  = new ds3t::INI::File (L"SoulsUnsqueezed.ini");
    dxgi_ini = new ds3t::INI::File (L"dxgi.ini");

    std::wstring rtss_path = DS3T_GetRTSSInstallDir ();
    rtss_path += L"Profiles\\DarkSoulsIII.exe.cfg";

    rtss_ini  = new ds3t::INI::File (rtss_path.c_str ());
  }

  void save (std::wstring path) {
    ds3t::XML::SaveXML ();

    if (! (sus_ini->get_sections  ().empty () ||
           dxgi_ini->get_sections ().empty ())) {
      sus_ini->write  (L"SoulsUnsqueezed.ini");
      dxgi_ini->write (L"dxgi.ini");
    }

    std::wstring rtss_path = DS3T_GetRTSSInstallDir ();
    rtss_path += L"Profiles\\DarkSoulsIII.exe.cfg";

    rtss_ini->write (rtss_path.c_str ());
  }

  void reload_rtss (void) {
    if (rtss_ini != nullptr)
      delete rtss_ini;

    std::wstring rtss_path = DS3T_GetRTSSInstallDir ();
    rtss_path += L"Profiles\\DarkSoulsIII.exe.cfg";

    rtss_ini  = new ds3t::INI::File (rtss_path.c_str ());
  }


  std::wstring
  lookup_value (std::wstring attrib_name) {
    rapidxml::xml_node <wchar_t>* attrib =
      ds3t::XML::FindNode (ds3t::XML::ds3_root, attrib_name);

    if (attrib != nullptr)
      return attrib->value ();
    else
      return L"";
  }

  std::wstring
  lookup_value_sus (std::wstring section_name, std::wstring key_name) {
    ds3t::INI::File::Section& section = sus_ini->get_section (section_name);
    return section.get_value (key_name);
  }

  std::wstring
  lookup_value_dxgi (std::wstring section_name, std::wstring key_name) {
    ds3t::INI::File::Section& section = dxgi_ini->get_section (section_name);
    return section.get_value (key_name);
  }

  std::wstring
  lookup_value_rtss (std::wstring section_name, std::wstring key_name) {
    ds3t::INI::File::Section& section = rtss_ini->get_section (section_name);
    return section.get_value (key_name);
  }


  void
  set_value (std::wstring attrib_name, std::wstring value) {
    rapidxml::xml_node <wchar_t>* attrib =
      ds3t::XML::FindNode (ds3t::XML::ds3_root, attrib_name);

    if (attrib != nullptr) {
      // This leaks, but it's necessary for the XML parser ...
      //   it does not really matter in the end, we only save once.
      attrib->value (_wcsdup (value.c_str ()));
    }
  }

  void
  set_value_sus (std::wstring section_name, std::wstring key_name, std::wstring value) {
    ds3t::INI::File::Section& section = sus_ini->get_section (section_name);
    section.get_value (key_name) = value;
  }

  void
  set_value_dxgi (std::wstring section_name, std::wstring key_name, std::wstring value) {
    ds3t::INI::File::Section& section = dxgi_ini->get_section (section_name);
    section.get_value (key_name) = value;
  }

  void
  set_value_rtss (std::wstring section_name, std::wstring key_name, std::wstring value) {
    ds3t::INI::File::Section& section = rtss_ini->get_section (section_name);
    section.get_value (key_name) = value;
  }

  void import_sus (std::wstring imp_data) {
    sus_ini->import (imp_data);
  }

  void import_dxgi (std::wstring imp_data) {
    dxgi_ini->import (imp_data);
  }

  void import_rtss (std::wstring imp_data) {
    rtss_ini->import (imp_data);
  }

  ds3t::INI::File* get_file_sus  (void) { return sus_ini;  }
  ds3t::INI::File* get_file_dxgi (void) { return dxgi_ini; }
  ds3t::INI::File* get_file_rtss (void) { return rtss_ini; }


private:
  ds3t::INI::File* sus_ini;
  ds3t::INI::File* dxgi_ini;
  ds3t::INI::File* rtss_ini;
} extern config;