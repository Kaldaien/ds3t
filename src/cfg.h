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

#ifndef __DS3T__CFG_H__
#define __DS3T__CFG_H__

#include <string>
#include <vector>
#include <unordered_map>

namespace ds3t {
namespace CFG {

  class File
  {
  public:
             File (wchar_t* filename);
    virtual ~File (void);

    void parse  (void);
    void import (std::wstring import_data);
    void write  (std::wstring fname);

    class Section
    {
    public:
      Section (void) {
      }

      Section (std::wstring section_name) {
        name = section_name;
      }

      // Technically, this isn't 1:1 ... but as far as WE'RE concerned, all the
      //   keys we're interested in _are_.
      std::wstring& get_value     (std::wstring key);
      bool          contains_key  (std::wstring key);
      void          add_key_value (std::wstring key, std::wstring value);

      //protected:
      //private:
      std::wstring                                         name;
      std::unordered_multimap <std::wstring, std::wstring> pairs;
      std::vector             <std::wstring>               order;
    };

    const std::unordered_map <std::wstring, Section>& get_sections (void);

    Section& get_section      (std::wstring section);
    bool     contains_section (std::wstring section);

  protected:
  private:
    FILE*     fCFG;
    wchar_t*  wszName;
    wchar_t*  wszData;
    std::unordered_map <std::wstring, Section>
              sections;
  };

}
}

#endif /* __DS3T__CFG_H__ */
