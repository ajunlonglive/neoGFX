// menu_bar.cpp
/*
neogfx C++ GUI Library
Copyright(C) 2016 Leigh Johnston

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

#include "neogfx.hpp"
#include "app.hpp"
#include "menu_bar.hpp"
#include "menu_item_widget.hpp"
#include "popup_menu.hpp"

namespace neogfx
{
	menu_bar::menu_bar() : menu(MenuBar), iLayout(*this)
	{
		init();
	}

	menu_bar::menu_bar(i_widget& aParent) : widget(aParent), menu(MenuBar), iLayout(*this)
	{
		init();
	}

	menu_bar::menu_bar(i_layout& aLayout) : widget(aLayout), menu(MenuBar), iLayout(*this)
	{
		init();
	}

	menu_bar::~menu_bar()
	{
		close_sub_menu();
		item_added.unsubscribe(this);
		item_removed.unsubscribe(this);
		item_selected.unsubscribe(this);
		open_sub_menu.unsubscribe(this);
		remove_widgets();
	}

	size_policy menu_bar::size_policy() const
	{
		if (widget::has_size_policy())
			return widget::size_policy();
		return neogfx::size_policy::Minimum;
	}

	bool menu_bar::visible() const
	{
		if (app::instance().basic_services().has_shared_menu_bar())
			return false;
		return widget::visible();
	}

	bool menu_bar::key_pressed(scan_code_e aScanCode, key_code_e, key_modifiers_e)
	{
		bool handled = true;
		switch (aScanCode)
		{
		case ScanCode_LEFT:
			break;
		case ScanCode_RIGHT:
			break;
		default:
			handled = false;
			break;
		}
		return handled;
	}

	void menu_bar::init()
	{
		set_margins(neogfx::margins{});
		layout().set_margins(neogfx::margins{});
		item_added([this](item_index aItemIndex)
		{
			layout().add_item(aItemIndex, std::make_shared<menu_item_widget>(*this, item(aItemIndex)));
		}, this);
		item_removed([this](item_index aItemIndex)
		{
			if (layout().is_widget(aItemIndex))
			{
				i_widget& w = layout().get_widget(aItemIndex);
				w.parent().remove_widget(w);
			}
			layout().remove_item(aItemIndex);
		}, this);
		item_selected([this](i_menu_item& aMenuItem)
		{
			if (iOpenSubMenu.get() != 0)
			{
				if (aMenuItem.type() == i_menu_item::Action ||
					(aMenuItem.type() == i_menu_item::SubMenu && &iOpenSubMenu->menu() != &aMenuItem.sub_menu() && aMenuItem.sub_menu().item_count() > 0))
				{
					iOpenSubMenu->menu().close();
					if (aMenuItem.type() == i_menu_item::SubMenu)
						open_sub_menu.trigger(aMenuItem.sub_menu());
				}
			}
			update();
		}, this);
		open_sub_menu([this](i_menu& aSubMenu)
		{
			if (aSubMenu.item_count() > 0)
			{
				auto& itemWidget = layout().get_widget<menu_item_widget>(find_item(aSubMenu));
				close_sub_menu();
				iOpenSubMenu = std::make_unique<popup_menu>(*this, itemWidget.sub_menu_position(), aSubMenu);
				app::instance().keyboard().grab_keyboard(*this);
				iOpenSubMenu->menu().closed([this]()
				{
					if (iOpenSubMenu.get() != 0)
						iOpenSubMenu->close();
				}, this);
				iOpenSubMenu->closed([this]()
				{
					close_sub_menu();
				}, this);
			}
		}, this);
	}

	void menu_bar::close_sub_menu()
	{
		if (iOpenSubMenu.get() != 0)
		{
			iOpenSubMenu->menu().closed.unsubscribe(this);
			iOpenSubMenu.reset();
			app::instance().keyboard().ungrab_keyboard(*this);
		}
	}

}