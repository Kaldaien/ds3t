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

#ifndef __DS3T__DXGI_H__
#define __DS3T__DXGI_H__

#include <string>
#include <dxgi.h>

namespace ds3t
{
  namespace DXGI
  {
    enum GPUMemoryPool {
      Dedicated,
      GART
    };

    // TODO: Allow querying this per-adapter, as one would logically expect...
    size_t GetAdapterPool (GPUMemoryPool pool);
    size_t GetGART        (void);
    size_t GetVRAM        (void);

    std::wstring
           GetGPUInfo     (void);

  }
}

#endif /* __DS3T__DXGI_H__ */