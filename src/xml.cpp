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

#include "xml.h"
#include "utility.h"

#include <Windows.h>
#include <cstring>

xml_node<wchar_t>*
ds3t::XML::FindNode (xml_node<wchar_t>* parent_node, std::wstring name)
{
  if (parent_node == NULL)
    return NULL;

  xml_node<wchar_t>* node = parent_node->first_node ();
  while (node != NULL) {
    if (std::wstring (node->name ()) == name)
      return node->first_node ();
    node = node->next_sibling ();
  }

  return NULL;
}

xml_attribute<wchar_t>*
ds3t::XML::FindAttrib (xml_node<wchar_t>* parent_node, std::wstring name)
{
  if (parent_node == NULL)
    return NULL;

  xml_attribute<wchar_t>* attrib = parent_node->first_attribute ();
  while (attrib != NULL) {
    if (std::wstring (attrib->name ()) == name)
      return attrib;
    attrib = attrib->next_attribute ();
  }

  return NULL;
}

xml_document<wchar_t> ds3t::XML::ds3_xml;
xml_node<wchar_t>*    ds3t::XML::ds3_root;

void
ds3t::XML::SaveXML (void)
{
  std::wstring documents_dir = DS3T_GetRoamingAppDataDir ();

  wchar_t wszXML [1024];

  wsprintf (wszXML, L"%s\\DarkSoulsIII\\GraphicsConfig.XML", documents_dir.c_str ());

  wchar_t* wszOut = new wchar_t [8192];
  wchar_t* wszEnd = print (wszOut, ds3_xml, 0);

  int last_brace = 0;

  // XML parser doesn't like the TM symbol, so get it the hell out of there!
  for (int i = 0; i < 8192; i++) {
    if (wszOut [i] == L'™')
      wszOut [i] = L' ';
    if (wszOut [i] == L'>')
      last_brace = i;
  }

  wszOut [last_brace + 1] = L'\0';

  FILE* fXML;
  errno_t ret = _wfopen_s (&fXML, wszXML, L"w,ccs=UTF-16LE");

  if (ret != 0 || fXML == 0) {
    delete [] wszOut;
    DS3T_MessageBox ( L"Could not open GraphicsConfig.XML for writing!\n",
                        L"Unable to save XML settings",
                          MB_OK | MB_ICONSTOP );
    return;
  }

  fputws (L"<?xml version=\"1.0\" encoding=\"UTF-16\" ?>\n", fXML);
  fputws (wszOut, fXML);

  delete [] wszOut;

  fflush (fXML);
  fclose (fXML);

  //
  // Windows 10 File Permission Fixes
  //

  // Normalize file ownership and attributes (Win10 fix)
  DS3T_SetNormalFileAttribs (wszXML);

  // Now normalize the directory as a whole
  wsprintf (wszXML, L"%s\\DarkSoulsIII", documents_dir.c_str ());
  DS3T_SetNormalFileAttribs (wszXML);
}

bool
ds3t::XML::LoadXML (void)
{
  wchar_t wszXML [1024];

  std::wstring documents_dir = DS3T_GetRoamingAppDataDir ();

  //
  // Windows 10 File Permission Fixes
  //

  // Normalize the directory as a whole
  wsprintf (wszXML, L"%s\\DarkSoulsIII", documents_dir.c_str ());
  DS3T_SetNormalFileAttribs (wszXML);

  wsprintf (wszXML, L"%s\\DarkSoulsIII\\GraphicsConfig.XML", documents_dir.c_str ());

  // Normalize file ownership and attributes (Win10 fix)
  DS3T_SetNormalFileAttribs (wszXML);

  FILE* fXML;
  errno_t ret = _wfopen_s (&fXML, wszXML, L"r,ccs=UTF-16LE");

  if (ret != 0) {
    DS3T_MessageBox (L"Unable to locate GraphicsConfig.XML.\n\n  "
      L"This file is created when the game starts, try running the "
      L"game once.\n", L"Missing XML File", MB_OK | MB_ICONERROR);

    return false;
  }

  fseek (fXML, 0, SEEK_END);
  DWORD dwLen = ftell (fXML);
  rewind (fXML);

  wchar_t* xml = new wchar_t [dwLen + 1];
  fread (xml, 1, dwLen, fXML);

  int last_brace = 0;

  // XML parser doesn't like the TM symbol, so get it the hell out of there!
  for (unsigned int i = 0; i < dwLen; i++) {
    if (xml [i] == L'™')
      xml [i] = L' ';
    if (xml [i] == L'>')
      last_brace = i;
  }

  xml [last_brace + 1] = 0;

  ds3_xml.parse <0> (xml);

  fclose (fXML);

  ds3_root = ds3_xml.first_node ();

  if (ds3_root == NULL) {
    DS3T_MessageBox (L"GraphicsConfig.XML appears to be corrupt, please delete it and re-load the game\n", L"Corrupt XML File", MB_OK | MB_ICONHAND);
    return false;
  }

  return true;
}