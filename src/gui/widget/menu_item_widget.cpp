// menu_item_widget.cpp
/*
neogfx C++ GUI Library
Copyright (c) 2015-present, Leigh Johnston.  All Rights Reserved.

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
#include <neogfx/gui/widget/menu_item_widget.hpp>
#include <neogfx/gui/widget/i_menu.hpp>
#include <neogfx/gui/window/popup_menu.hpp>
#include <neogfx/app/app.hpp>

namespace neogfx
{
	menu_item_widget::menu_item_widget(i_menu& aMenu, i_menu_item& aMenuItem) :
		iMenu{ aMenu }, iMenuItem{ aMenuItem }, iLayout{ *this }, iIcon{ iLayout, texture{}, aspect_ratio::Keep }, iText{ iLayout }, iSpacer{ iLayout }, iShortcutText{ iLayout }
	{
		init();
	}

	menu_item_widget::menu_item_widget(i_widget& aParent, i_menu& aMenu, i_menu_item& aMenuItem) :
		widget{ aParent }, iMenu{ aMenu }, iMenuItem{ aMenuItem }, iLayout{ *this }, iIcon{ iLayout, texture{}, aspect_ratio::Keep }, iText{ iLayout }, iSpacer{ iLayout }, iShortcutText{ iLayout }
	{
		init();
	}

	menu_item_widget::menu_item_widget(i_layout& aLayout, i_menu& aMenu, i_menu_item& aMenuItem) :
		widget{ aLayout }, iMenu{ aMenu }, iMenuItem{ aMenuItem }, iLayout{ *this }, iIcon{ iLayout, texture{}, aspect_ratio::Keep }, iText{ iLayout }, iSpacer{ iLayout }, iShortcutText{ iLayout }
	{
		init();
	}

	menu_item_widget::~menu_item_widget()
	{
		app::instance().remove_mnemonic(*this);
		iSubMenuOpener.reset();
	}

	i_menu& menu_item_widget::menu() const
	{
		return iMenu;
	}

	i_menu_item& menu_item_widget::menu_item() const
	{
		return iMenuItem;
	}

	neogfx::size_policy menu_item_widget::size_policy() const
	{
		if (widget::has_size_policy())
			return widget::size_policy();
		return neogfx::size_policy{ menu().type() == i_menu::type_e::Popup ? neogfx::size_policy::Expanding : neogfx::size_policy::Minimum, neogfx::size_policy::Minimum };
	}

	size menu_item_widget::minimum_size(const optional_size&) const
	{
		size result = widget::minimum_size();
		if (menu_item().type() == i_menu_item::Action && menu_item().action().is_separator())
			result.cy = units_converter(*this).from_device_units(dpi_scale(3.0));
		return result;
	}

	void menu_item_widget::paint_non_client(graphics_context& aGraphicsContext) const
	{
		if (menu().has_selected_item() && menu().selected_item() == (menu().find(menu_item())))
		{
			bool openSubMenu = (menu_item().type() == i_menu_item::SubMenu && menu_item().sub_menu().is_open());
			colour fillColour = background_colour().dark() ? colour::Black : colour::White;
			if (openSubMenu && menu().type() == i_menu::MenuBar)
			{
				if (fillColour.similar_intensity(app::instance().current_style().palette().colour(), 0.05))
					fillColour = app::instance().current_style().palette().selection_colour();
			}
			else if (fillColour.similar_intensity(background_colour(), 0.05))
				fillColour = fillColour.dark() ? fillColour.lighter(0x20) : fillColour.darker(0x20);
			fillColour.set_alpha(0xC0);
			aGraphicsContext.fill_rect(client_rect(), fillColour);
		}
	}

	void menu_item_widget::paint(graphics_context& aGraphicsContext) const
	{
		if (menu_item().type() != i_menu_item::Action || !menu_item().action().is_separator())
		{
			widget::paint(aGraphicsContext);
			if (menu_item().type() == i_menu_item::SubMenu && menu().type() == i_menu::Popup)
			{
				bool openSubMenu = (menu_item().type() == i_menu_item::SubMenu && menu_item().sub_menu().is_open());
				colour ink = openSubMenu ? app::instance().current_style().palette().selection_colour()
					: background_colour().light() ? background_colour().darker(0x80) : background_colour().lighter(0x80);
				if (iSubMenuArrow == boost::none || iSubMenuArrow->first != ink)
				{
					const char* sArrowImagePattern
					{
						"[6,9]"
						"{0,paper}"
						"{1,ink}"

						"000000"
						"010000"
						"011000"
						"011100"
						"011110"
						"011100"
						"011000"
						"010000"
						"000000"
					};
					const char* sArrowHighDpiImagePattern
					{
						"[12,18]"
						"{0,paper}"
						"{1,ink}"

						"000000000000"
						"001000000000"
						"001100000000"
						"001110000000"
						"001111000000"
						"001111100000"
						"001111110000"
						"001111111000"
						"001111111100"
						"001111111100"
						"001111111000"
						"001111110000"
						"001111100000"
						"001111000000"
						"001110000000"
						"001100000000"
						"001000000000"
						"000000000000"
					};
					iSubMenuArrow = std::make_pair(ink, 
						!high_dpi() ? 
							image{ "neogfx::menu_item_widget::sArrowImagePattern::" + ink.to_string(), sArrowImagePattern, { { "paper", colour{} },{ "ink", ink } } } : 
							image{ "neogfx::menu_item_widget::sArrowHighDpiImagePattern::" + ink.to_string(), sArrowHighDpiImagePattern,{ { "paper", colour{} },{ "ink", ink } }, 2.0 });
				}
				rect rect = client_rect(false);
				aGraphicsContext.draw_texture(
					point{ rect.right() - dpi_scale(iGap) + std::floor((dpi_scale(iGap) - iSubMenuArrow->second.extents().cx) / 2.0), std::floor((rect.height() - iSubMenuArrow->second.extents().cy) / 2.0) },
					iSubMenuArrow->second);
			}
		}
		else
		{
			scoped_units su(*this, aGraphicsContext, units::Pixels);
			rect line = client_rect(false);
			line.y += dpi_scale(1.0);
			line.cy = dpi_scale(1.0);
			line.x += dpi_scale(iGap);
			line.cx -= dpi_scale(iGap * 2.0);
			colour ink = colour::Black;
			if (ink.similar_intensity(background_colour(), 0.05))
				ink = ink.dark() ? ink.lighter(0x20) : ink.darker(0x20);
			ink.set_alpha(0x40);
			aGraphicsContext.fill_rect(line, ink);
		}
	}

	colour menu_item_widget::background_colour() const
	{
		return widget::has_background_colour() ?
			widget::background_colour() :
			parent().has_background_colour() ?
				parent().background_colour() :
				app::instance().current_style().palette().colour();
	}

	bool menu_item_widget::can_capture() const
	{
		return false;
	}

	void menu_item_widget::mouse_entered(const point& aPosition)
	{
		widget::mouse_entered(aPosition);
		update();
		if (menu_item().available())
			menu().select_item_at(menu().find(menu_item()), menu_item().type() == i_menu_item::SubMenu);
	}

	void menu_item_widget::mouse_left()
	{
		widget::mouse_left();
		update();
		if (menu().has_selected_item() && menu().selected_item() == (menu().find(menu_item())) &&
			(menu_item().type() == i_menu_item::Action || (!menu_item().sub_menu().is_open() && !iSubMenuOpener)))
			menu().clear_selection();
	}

	void menu_item_widget::mouse_button_pressed(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers)
	{
		widget::mouse_button_pressed(aButton, aPosition, aKeyModifiers);
		if (aButton == mouse_button::Left && menu_item().type() == i_menu_item::SubMenu)
			select_item();
	}

	void menu_item_widget::mouse_button_released(mouse_button aButton, const point& aPosition)
	{
		widget::mouse_button_released(aButton, aPosition);
		if (aButton == mouse_button::Left && menu_item().type() == i_menu_item::Action)
			select_item();
	}

	bool menu_item_widget::key_pressed(scan_code_e aScanCode, key_code_e, key_modifiers_e)
	{
		if (aScanCode == ScanCode_RETURN)
		{
			select_item(true);
			return true;
		}
		return false;
	}

	std::string menu_item_widget::mnemonic() const
	{
		return mnemonic_from_text(iText.text());
	}

	void menu_item_widget::mnemonic_execute()
	{
		select_item(true);
	}

	i_widget& menu_item_widget::mnemonic_widget()
	{
		return iText;
	}

	bool menu_item_widget::help_active() const
	{
		return menu_item().type() == i_menu_item::Action &&
			menu().has_selected_item() && menu().find(menu_item()) == menu().selected_item();
	}

	help_type menu_item_widget::help_type() const
	{
		return neogfx::help_type::Action;
	}

	std::string menu_item_widget::help_text() const
	{
		if (help_active())
			return menu_item().action().help_text();
		else
			return "";
	}

	point menu_item_widget::sub_menu_position() const
	{
		if (menu().type() == i_menu::MenuBar)
			return window_rect().bottom_left() + root().window_position();
		else
			return window_rect().top_right() + root().window_position();
	}

	void menu_item_widget::init()
	{
		set_margins(neogfx::margins{});
		iLayout.set_margins(dpi_scale(neogfx::margins{ iGap, dpi_scale(2.0), iGap * (menu().type() == i_menu::Popup ? 2.0 : 1.0), dpi_scale(2.0) }));
		iLayout.set_spacing(dpi_scale(size{ iGap, 0.0 }));
		if (menu().type() == i_menu::Popup)
			iIcon.set_fixed_size(dpi_scale(iIconSize));
		else
			iIcon.set_fixed_size(size{});
		iSpacer.set_minimum_size(size{ 0.0, 0.0 });
		auto text_updated = [this]()
		{
			auto m = mnemonic_from_text(iText.text());
			if (!m.empty())
				app::instance().add_mnemonic(*this);
			else
				app::instance().remove_mnemonic(*this);
		};
		iSink += iText.text_changed(text_updated);
		text_updated();
		if (menu_item().type() == i_menu_item::Action)
		{
			auto action_changed = [this]()
			{
				iIcon.set_image(menu_item().action().is_unchecked() ? menu_item().action().image() : menu_item().action().checked_image());
				if (iIcon.image().is_empty() && menu_item().action().is_checked())
				{
					const char* sTickPattern
					{
						"[16,8]"
						"{0,paper}"
						"{1,ink}"

						"0000000000100000"
						"0000000001100000"
						"0000000001000000"
						"0000000011000000"
						"0000010010000000"
						"0000011110000000"
						"0000001100000000"
						"0000001100000000"
					};
					const char* sTickHighDpiPattern
					{
						"[32,16]"
						"{0,paper}"
						"{1,ink}"

						"00000000000000000000110000000000"
						"00000000000000000000110000000000"
						"00000000000000000001110000000000"
						"00000000000000000011100000000000"
						"00000000000000000011000000000000"
						"00000000000000000011000000000000"
						"00000000000000000111000000000000"
						"00000000000000001110000000000000"
						"00000000001100001100000000000000"
						"00000000001100001100000000000000"
						"00000000001110011100000000000000"
						"00000000000111111000000000000000"
						"00000000000011110000000000000000"
						"00000000000011110000000000000000"
						"00000000000001100000000000000000"
						"00000000000001100000000000000000"
					};
					colour ink = app::instance().current_style().palette().text_colour();
					iIcon.set_image(!high_dpi() ?
						image{ "neogfx::menu_item_widget::sTickPattern::" + ink.to_string(), sTickPattern,{ { "paper", colour{} },{ "ink", ink } } } :
						image{ "neogfx::menu_item_widget::sTickHighDpiPattern::" + ink.to_string(), sTickHighDpiPattern,{ { "paper", colour{} },{ "ink", ink } }, 2.0 });
				}
				if (!iIcon.image().is_empty())
					iIcon.set_fixed_size(dpi_scale(iIconSize));
				else if (menu().type() == i_menu::MenuBar)
					iIcon.set_fixed_size(size{});
				iText.set_text(menu_item().action().menu_text());
				if (menu().type() != i_menu::MenuBar)
					iShortcutText.set_text(menu_item().action().shortcut() != boost::none ? menu_item().action().shortcut()->as_text() : std::string());
				iSpacer.set_minimum_size(dpi_scale(size{ menu_item().action().shortcut() != boost::none && menu().type() != i_menu::MenuBar ? iGap * 2.0 : 0.0, 0.0 }));
				enable(menu_item().action().is_enabled());
			};
			iSink += menu_item().action().changed(action_changed);
			iSink += menu_item().action().checked(action_changed);
			iSink += menu_item().action().unchecked(action_changed);
			iSink += menu_item().action().enabled(action_changed);
			iSink += menu_item().action().disabled(action_changed);
			action_changed();
		}
		else
		{
			iSink += menu_item().sub_menu().opened([this]() {update(); });
			iSink += menu_item().sub_menu().closed([this]() {update(); });
			auto menu_changed = [this]() 
			{ 
				iIcon.set_image(menu_item().sub_menu().image());
				iText.set_text(menu_item().sub_menu().title());
			};
			iSink += menu_item().sub_menu().menu_changed(menu_changed);
			menu_changed();
		}
		iSink += menu_item().selected([this]()
		{
			if (menu_item().type() == i_menu_item::Action)
				app::instance().help().activate(*this);
			else if (menu_item().type() == i_menu_item::SubMenu && menu_item().open_any_sub_menu() && menu().type() == i_menu::Popup)
			{
				if (!iSubMenuOpener)
				{
					iSubMenuOpener = std::make_unique<neolib::callback_timer>(app::instance(), [this](neolib::callback_timer&)
					{
						destroyed_flag destroyed{ *this };
						if (!menu_item().sub_menu().is_open())
							menu().open_sub_menu.trigger(menu_item().sub_menu());
						if (!destroyed)
							update();
						iSubMenuOpener.reset();
					}, 250);
				}
			}
		});
		iSink += menu_item().deselected([this]()
		{
			if (menu_item().type() == i_menu_item::Action)
				app::instance().help().deactivate(*this);
			iSubMenuOpener.reset();
		});
	}

	void menu_item_widget::select_item(bool aOpenAnySubMenu)
	{
		destroyed_flag destroyed{ *this };
		if (!menu_item().available())
			return;
		if (menu_item().type() == i_menu_item::Action)
		{
			menu().clear_selection();
			if (destroyed)
				return;
			menu_item().action().triggered.async_trigger();
			if (destroyed)
				return;
			if (menu_item().action().is_checkable())
				menu_item().action().toggle();
			if (destroyed)
				return;
			i_menu* menuToClose = &menu();
			while (menuToClose->has_parent())
				menuToClose = &menuToClose->parent();
			if (menuToClose->type() == i_menu::MenuBar)
				menuToClose->clear_selection();
			else if (menuToClose->type() == i_menu::Popup && menuToClose->is_open())
				menuToClose->close();
			if (destroyed)
				return;
		}
		else
		{
			if (!menu_item().sub_menu().is_open())
			{
				menu().select_item_at(menu().find(menu_item()), aOpenAnySubMenu);
				if (destroyed)
					return;
				menu().open_sub_menu.trigger(menu_item().sub_menu());
				if (destroyed)
					return;
				update();
			}
			else if (menu().type() == i_menu::MenuBar)
			{
				menu_item().sub_menu().close();
				if (destroyed)
					return;
				update();
			}
			else
			{
				auto& subMenu = menu_item().sub_menu();
				if (!subMenu.has_selected_item() && subMenu.has_available_items())
					subMenu.select_item_at(subMenu.first_available_item(), false);
			}
		}
	}
}