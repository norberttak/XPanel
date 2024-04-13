/*
 * Copyright 2022 Norbert Takacs
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include <string>
#include <map>
#include "core/Configuration.h"
#include "core/GenericScreen.h"
#include "FIPPage.h"

class FIPScreen : public GenericScreen
{
private:
	std::map<int, FIPPage*> pages;
	const int SCREEN_WIDTH = 320;
	const int SCREEN_HEIGHT = 240;
	const int BIT_PER_PIXEL = 24;
	std::mutex guard;
public:
	FIPScreen();
	FIPScreen(FIPScreen* other);
	~FIPScreen();

	void evaluate_and_store_screen_action();

	void add_page(int page_index, std::string _debug_name, bool set_active);
	void add_page(std::string _debug_name, bool set_active);
	int get_last_page_index();
	int add_layer_to_page(int page_index, std::string bmp_file_name, int ref_x, int ref_y, int base_rot);
	int add_text_layer_to_page(int page_index, std::string font_file_name, int base_rot);
	int get_last_layer_index(int page_index);
	std::string get_page_name(int page_index);
	void set_text(int page_index, int layer_index, std::string text);
    void set_mask(int page_index, int layer_index, MaskWindow& mask);
	void rotate_layer(int page_index, int layer_index, int angle);
	void translate_layer_x(int page_index, int layer_index, int offset_x);
	void translate_layer_y(int page_index, int layer_index, int offset_y);
	void render_page(int page_index, void **byte_buffer);
};
