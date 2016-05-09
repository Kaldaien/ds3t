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

class suscfg_Plugins
{
public:
  suscfg_Plugins (void);

  bool setup_ui  (HWND hDlg);

//protected:

//private:
} *plugins = nullptr;

suscfg_Plugins::suscfg_Plugins (void)
{
  ds3t::INI::File* sus_ini  = config.get_file_sus  ();
  ds3t::INI::File* dxgi_ini = config.get_file_dxgi ();
}

bool
suscfg_Plugins::setup_ui (HWND hDlg)
{
  return true;
}

INT_PTR
CALLBACK
PluginsConfig (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch (message)
  {
    case WM_INITDIALOG:
    {
      if (plugins == nullptr) {
        plugins = new suscfg_Plugins ();
      }

      plugins->setup_ui (hDlg);
    } break;

    case WM_COMMAND:
    {
      if (LOWORD (wParam) == IDOK)
      {
        EndDialog (hDlg, LOWORD (wParam));
        return (INT_PTR)TRUE;
      }
    } break;
  }

  return (INT_PTR)FALSE;
}