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
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers

#include <windows.h>

#include "cfg.h"
#include "utility.h"
#include <string>

std::wstring
ErrorMessage (errno_t        err,
              const char*    args,
              const wchar_t* cfg_name,
              UINT           line_no,
              const char*    function_name,
              const char*    file_name)
{
  wchar_t wszFormattedError [1024];
  *wszFormattedError = L'\0';

  swprintf (wszFormattedError, 1024,
    L"Line %u of %hs (in %hs (...)):\n"
    L"------------------------\n\n"
    L"%hs\n\n  File: %s\n\n"
    L"\t>> %s <<",
    line_no,
    file_name,
    function_name,
    args,
    cfg_name,
    _wcserror (err));

  return wszFormattedError;
}

#define TRY_FILE_IO(x,y,z) {                                                  \
  (z) = ##x;                                                                  \
                                                                              \
  if ((z) != 0)                                                               \
    DS3T_MessageBox (ErrorMessage ((z),#x,(y),__LINE__,__FUNCTION__,__FILE__),\
                     L"File I/O Error", MB_OK | MB_ICONSTOP );                \
}

ds3t::CFG::File::File (wchar_t* filename)
{
  wszName = _wcsdup (filename);

  char name [MAX_PATH];
  sprintf (name, "%ls", wszName);

  errno_t ret;
  TRY_FILE_IO (fopen_s (&fCFG, name, "r"), filename, ret);

  if (ret == 0 && fCFG != 0) {
                fseek  (fCFG, 0, SEEK_END);
    long size = ftell  (fCFG);
                rewind (fCFG);

    char*  szData = new char    [size];
          wszData = new wchar_t [size];

    fread (szData, 1, size, fCFG);

    // We process everything in Unicode internally, so this is the easiest way
    //   to deal with that...
    MultiByteToWideChar (CP_OEMCP, 0, szData, -1, wszData, size);

    parse ();

    delete [] wszData;
    wszData = nullptr;

    fflush (fCFG);
    fclose (fCFG);
  }
  else {
    DS3T_MessageBox (L"Unable to Locate CFG File", filename, MB_OK);
  }
}

ds3t::CFG::File::~File (void)
{
  if (wszName != nullptr) {
    delete [] wszName;
    wszName = nullptr;
  }

  if (wszData != nullptr) {
    delete[] wszData;
    wszData = nullptr;
  }
}

ds3t::CFG::File::Section
Process_Section (wchar_t* buffer, wchar_t* name, int start, int end)
{
  ds3t::CFG::File::Section section (name);

  int key = start;
  for (int k = key; k < end; k++) {
    if (k < end - 1 && buffer [k] == L'=') {
      wchar_t* key_str = new wchar_t [k - key + 1];
      wcsncpy (key_str, buffer + key, k - key);
      key_str [k - key] = L'\0';
      //if (! section.name.compare (L"IniVersion"))
      //MessageBoxW (NULL, key_str, L"Key", MB_OK);

      int value = k + 1;
      for (int l = value; l <= end; l++) {
        if (l > end - 1 || buffer [l] == L'\n') {
          key = l + 1;
          k   = l + 1;
          wchar_t* val_str = new wchar_t [l - value + 1];
          wcsncpy (val_str, buffer + value, l - value);
          val_str [l - value] = L'\0';

          //MessageBoxW (NULL, key_str, L"Key", MB_OK);
          section.add_key_value (key_str, val_str);

          delete [] val_str;
          l = end;
          break;
        }
      }

      delete [] key_str;
    }
  }

  return section;
}

bool
Import_Section (ds3t::CFG::File::Section& section, wchar_t* buffer, int start, int end)
{
#if 0
  int key = start;
  for (int k = key; k < end; k++) {
    if (k < end - 1 && buffer [k] == L'=') {
      wchar_t* key_str = new wchar_t [k - key + 1];
      wcsncpy (key_str, buffer + key, k - key);
      key_str [k - key] = L'\0';

      int value = k + 1;
      for (int l = value; l < end; l++) {
        if (l > end - 1 || buffer [l] == L'\n') {
          key = l + 1;
          k = l + 1;
          wchar_t* val_str = new wchar_t [l - value + 1];
          wcsncpy (val_str, buffer + value, l - value);
          val_str [l - value] = L'\0';

          // Prefer to change an existing value
          if (section.contains_key (key_str)) {
            std::wstring& val = section.get_value (key_str);
            val = val_str;
          }

          // But create a new one if it doesn't already exist
          else {
            section.add_key_value (key_str, val_str);
          }

          delete [] val_str;
          l = end;
        }
      }

      delete [] key_str;
    }
  }
#endif

  return true;
}

void
ds3t::CFG::File::parse (void)
{
  if (wszData != nullptr) {
    int len = lstrlenW (wszData);

    // We don't want CrLf, just Lf
    bool strip_cr = false;

    // Find if the file has any Cr's
    for (int i = 0; i < len; i++) {
      if (wszData [i] == L'\r')
        strip_cr = true;
    }

    if (strip_cr) {
      // Remove all Cr's and then re-NUL terminate the truncated file
      int out = 0;
      for (int i = 0; i < len; i++) {
        if (wszData [i] != L'\r')
          wszData [out++] = wszData [i];
      }
      wszData [out] = L'\0';

      len = out;
    }

    wchar_t* sec_name = L"GLOBAL";

    Section section = Process_Section (wszData, sec_name, 0, len);

    sections.insert (std::pair <std::wstring, Section> (sec_name, section));
  }
}

void
ds3t::CFG::File::import (std::wstring import_data)
{
#if 0
  wchar_t* wszImport = _wcsdup (import_data.c_str ());

  if (wszImport != nullptr) {
    int len = lstrlenW (wszImport);

    // We don't want CrLf, just Lf
    bool strip_cr = false;

    // Find if the file has any Cr's
    for (int i = 0; i < len; i++) {
      if (wszImport [i] == L'\r')
        strip_cr = true;
    }

    if (strip_cr) {
      // Remove all Cr's and then re-NUL terminate the truncated file
      int out = 0;
      for (int i = 0; i < len; i++) {
        if (wszImport [i] != L'\r')
          wszImport [out++] = wszImport [i];
      }
      wszImport [out] = L'\0';

      len = out;
    }

    int begin = -1;
    int end = -1;

    for (int i = 0; i < len; i++)
    {
      if (wszImport [i] == L'[' && (i == 0 || wszImport [i - 1] == L'\n')) {
        begin = i + 1;
      }

      if (wszImport [i] == L']' && (i == len - 1 || wszImport [i + 1] == L'\n'))
        end = i;

      if (end != -1) {
        wchar_t* sec_name = new wchar_t [end - begin + 1];
        wcsncpy (sec_name, wszImport + begin, end - begin);
        sec_name [end - begin] = L'\0';
        //MessageBoxW (NULL, sec_name, L"Section", MB_OK);

        int start = end + 2;
        int finish = start;

        bool eof = false;
        for (int j = start; j <= len; j++) {
          if (j == len) {
            finish = j;
            eof = true;
            break;
          }

          if (wszImport [j - 1] == L'\n' && wszImport [j] == L'[') {
            finish = j - 1;
            break;
          }
        }

        // Import if the section already exists
        if (contains_section (sec_name)) {
          Section& section = get_section (sec_name);

          Import_Section (section, wszImport, start, finish);
        }

        // Insert otherwise
        else {
          Section section = Process_Section (wszImport, sec_name, start, finish);

          sections.insert (std::pair <std::wstring, Section> (sec_name, section));
        }
        delete [] sec_name;

        if (eof)
          break;

        i = finish;

        end = -1;
        begin = -1;
      }
    }
  }

  delete [] wszImport;
#endif
}

std::wstring invalid = L"Invalid";

std::wstring&
ds3t::CFG::File::Section::get_value (std::wstring key)
{
  for (std::unordered_multimap <std::wstring, std::wstring>::iterator it = pairs.begin ();
         it != pairs.end ();
           it++) {
    if ((*it).first == key)
      return (*it).second;
  }

  return invalid;

  // Only works if this is set-associative -- it's not.
  //  * This "INI" can have multiple keys with the same name!
  //return pairs [key];
}

bool
ds3t::CFG::File::Section::contains_key (std::wstring key)
{
  for (std::unordered_multimap <std::wstring, std::wstring>::iterator it = pairs.begin ();
  it != pairs.end ();
    it++) {
    if ((*it).first == key)
      return true;
  }

  return false;
}

void
ds3t::CFG::File::Section::add_key_value (std::wstring key, std::wstring value)
{
  pairs.insert    (std::pair <std::wstring, std::wstring> (key, value));
  order.push_back (key);
}

bool
ds3t::CFG::File::contains_section (std::wstring section)
{
  return sections.find (section) != sections.end ();
}

ds3t::CFG::File::Section&
ds3t::CFG::File::get_section (std::wstring section)
{
  return sections [section];// sections.find (section);
}

void
ds3t::CFG::File::write (std::wstring fname)
{
  FILE*   fOut;
  errno_t ret;

  // Strip Read-Only
  DS3T_SetNormalFileAttribs (fname);

  char name [MAX_PATH];
  sprintf (name, "%ls", fname.c_str ());

  TRY_FILE_IO (fopen_s (&fOut, name, "w"), fname.c_str (), ret);

  if (ret != 0 || fOut == 0) {
    //DS3T_MessageBox (L"ERROR: Cannot open INI file for writing. Is it read-only?", fname.c_str (), MB_OK | MB_ICONSTOP);
    return;
  }

  std::unordered_map <std::wstring, Section>::iterator it  = sections.begin ();
  std::unordered_map <std::wstring, Section>::iterator end = sections.end ();

  while (it != end) {
    // DS3's Config File is stupid as all hell, it only has one section ;)
    Section& section = (*it).second;
    ///////////fwprintf (fOut, L"[%s]\n", section.name.c_str ());

#if 0
    std::unordered_map <std::wstring, std::wstring>::iterator key_it  = section.pairs.begin ();
    std::unordered_map <std::wstring, std::wstring>::iterator key_end = section.pairs.end   ();

    while (key_it != key_end) {
      fwprintf (fOut, L"%s=%s\n", key_it->first.c_str (), key_it->second.c_str ());
      ++key_it;
    }
#else
    // Preserve insertion order so we don't jumble the config file up
    for (int i = 0; i < section.order.size (); i++) {
      std::wstring val = section.get_value (section.order [i]);
      fprintf (fOut, "%ls=%ls", section.order [i].c_str (), val.c_str ());

      // Add a new-line for every key-value pair except the last
      if (i < section.order.size () - 1)
        fprintf (fOut, "\n");
    }
#endif

    ++it;
    break;
  }

  fflush (fOut);
  fclose (fOut);
}


const std::unordered_map <std::wstring, ds3t::CFG::File::Section>&
ds3t::CFG::File::get_sections (void)
{
  return sections;
}