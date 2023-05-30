/*
 * Copyright 2022 Norbert Takacs
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <filesystem>
#include "LuaHelper.h"
#include "FIPScreen.h"
#include "Logger.h"

FIPScreen::FIPScreen() :GenericScreen()
{
	pages.clear();
}

FIPScreen::~FIPScreen()
{
	for (const auto &it_page : pages)
		delete it_page.second;

	pages.clear();
}

void FIPScreen::evaluate_and_store_screen_action()
{
	for (auto &action : screen_action_queue)
	{
		int action_value = 0;
		std::string action_value_str = "";

		if (action->data_ref != NULL)
		{
			if (action->data_ref_type == xplmType_Data)
				action_value_str = read_dataref_str(action->data_ref);
			else 
			{
				if (action->data_ref_index > -1)
					action_value = action->scale_factor * read_data_ref_int(action->data_ref, action->data_ref_type, action->data_ref_index);
				else
					action_value = action->scale_factor * read_data_ref_int(action->data_ref, action->data_ref_type);
				if (action->type == SC_SET_TEXT)
					action_value_str = std::to_string(action_value);
			}
		}
		else if (action->lua_str != "")
		{
			if (action->type == SC_SET_TEXT)
			{
				LuaHelper::get_instace()->do_string("return " + action->lua_str, action_value_str);
			}
			else
			{
				double ret_value = 0;
				LuaHelper::get_instace()->do_string("return " + action->lua_str, ret_value);
				action_value = (int)ret_value;
			}
		}
		else if (action->constant_val != 0)
		{
			action_value = action->constant_val;
		}
		else
		{
			//Logger(logERROR) << "FIP Screen: unknown value for screen action" << std::endl;
			//return;
		}

		if ((action->scale_factor == 0 || abs(action->value_old - action_value) < (1/action->scale_factor)) && (action->value_str_old == action_value_str))
			continue;

		action->value_old = action_value;
		action->value_str_old = action_value_str;

		//Logger(logTRACE) << "FIP screen: value changed (page " << action->page_index << " layer " << action->layer_index << "): " << action_value << std::endl;

		guard.lock();
		switch (action->type) {
		case SC_ROTATION:
			rotate_layer(action->page_index, action->layer_index, action_value);
			break;
		case SC_TRANSLATION_X:
			translate_layer_x(action->page_index, action->layer_index, action_value);
			break;
		case SC_TRANSLATION_Y:
			translate_layer_y(action->page_index, action->layer_index, action_value);
			break;
		case SC_SET_TEXT:
			set_text(action->page_index, action->layer_index, action_value_str);
			break;
		default:
			Logger(logERROR) << "FIP screen: unknown action type" << std::endl;
		}
		guard.unlock();
	}
}

void FIPScreen::add_page(int page_index, std::string _debug_name, bool set_active)
{
	Logger(logTRACE) << "FIP screen: add page. index=" << page_index << ", active=" << set_active << std::endl;

	FIPPage* fip_page = new FIPPage(SCREEN_WIDTH, SCREEN_HEIGHT, BIT_PER_PIXEL, _debug_name);
	pages[page_index] = fip_page;
}

void FIPScreen::add_page(std::string _debug_name, bool set_active)
{
	int	next_page_index = (int)pages.size();

	add_page(next_page_index, _debug_name, set_active);
}

int FIPScreen::get_last_page_index()
{
	return (int)pages.size() - 1;
}

std::string FIPScreen::get_page_name(int page_index)
{
	if (pages.count(page_index) == 0)
	{
		Logger(logERROR) << "FIPScreen: invalid page index: " << page_index << std::endl;
		return "";
	}
	return pages[page_index]->get_name();
}

int FIPScreen::get_last_layer_index(int page_index)
{
	return pages[page_index]->get_last_layer_index();
}

int FIPScreen::add_layer_to_page(int page_index, std::string bmp_file_name, int ref_x, int ref_y, int base_rot)
{
	Logger(logTRACE) << "FIP screen: add layer to page " << page_index << " file name:" << bmp_file_name << " ref_x=" << ref_x << " ref_y" << ref_y << std::endl;

	if (!std::filesystem::exists(bmp_file_name))
	{
		Logger(logERROR) << "FIP Screen: bmp file doesn't exist: " << bmp_file_name << std::endl;
		return -1;
	}
	int layer_index = pages[page_index]->add_layer_from_bmp_file(bmp_file_name, ref_x, ref_y, base_rot);
	Logger(logTRACE) << "FIP screen: new layer index=" << layer_index << std::endl;
	return layer_index;
}

int FIPScreen::add_text_layer_to_page(int page_index, std::string font_file_name, int base_rot)
{
	Logger(logTRACE) << "FIP screen: add text layer to page " << page_index << "font file name:" << font_file_name <<  std::endl;

	if (!std::filesystem::exists(font_file_name))
	{
		Logger(logERROR) << "FIP Screen: font bmp file doesn't exist: " << font_file_name << std::endl;
		return -1;
	}
	int layer_index = pages[page_index]->add_text_layer(font_file_name, base_rot);
	Logger(logTRACE) << "FIP screen: new text layer index=" << layer_index << std::endl;
	return layer_index;
}

void FIPScreen::set_text(int page_index, int layer_index, std::string text)
{
	pages[page_index]->set_text(layer_index, text);
}

void FIPScreen::set_mask(int page_index, int layer_index, MaskWindow& mask)
{
	pages[page_index]->set_mask(layer_index, mask);
}

void FIPScreen::rotate_layer(int page_index, int layer_index, int angle)
{
	pages[page_index]->rotate_layer(layer_index, angle);
}

void FIPScreen::translate_layer_x(int page_index, int layer_index, int offset_x)
{
	pages[page_index]->translate_layer_x(layer_index, offset_x);
}

void FIPScreen::translate_layer_y(int page_index, int layer_index, int offset_y)
{
	pages[page_index]->translate_layer_y(layer_index, offset_y);
}

void FIPScreen::render_page(int page_index, void** byte_buffer)
{
	if (pages.count(page_index) == 0)
		return;

	guard.lock();
	pages[page_index]->render_page();
	*byte_buffer = pages[page_index]->get_raw_buffer();
	guard.unlock();
}
