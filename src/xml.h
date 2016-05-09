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

#ifndef __DS3T__XML_H__
#define __DS3T__XML_H__

#include "rapidxml-1.13/rapidxml.hpp"
#include "rapidxml-1.13/rapidxml_print.hpp"

using namespace rapidxml;

namespace ds3t {
namespace XML {

  xml_node<wchar_t>*
    FindNode (xml_node<wchar_t>* parent_node, std::wstring name);

  xml_attribute<wchar_t>*
    FindAttrib (xml_node<wchar_t>* parent_node, std::wstring name);

  extern xml_document<wchar_t> ds3_xml;
  extern xml_node<wchar_t>*    ds3_root;
  extern xml_node<wchar_t>*    ds3_config;

  void SaveXML (void);
  bool LoadXML (void);

}
}

#endif /* __DS3T__XML_H__ */
