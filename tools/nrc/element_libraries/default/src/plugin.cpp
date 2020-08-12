/*
  plugin.cpp

  Copyright (c) 2019, 2020 Leigh Johnston.  All Rights Reserved.

  This program is free software: you can redistribute it and / or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <neogfx/neogfx.hpp>
#include <neolib/plugin/i_plugin.hpp>
#include <neogfx/app/app.hpp>
#include <neogfx/tools/nrc/ui_element_library_plugin.hpp>
#include "library.hpp"

API void entry_point(neolib::i_application& aApplication, const neolib::i_string& aPluginFolder, neolib::i_ref_ptr<neolib::i_plugin>& aPluginInstance)
{
    thread_local neogfx::app app{ aApplication.info() };
    neolib::ref_ptr<neolib::i_plugin> ref{ new neogfx::nrc::ui_element_library_plugin<neogfx::nrc::default_ui_element_library>{ aApplication } };
    aPluginInstance.reset(ref.ptr());
}

