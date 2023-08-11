/*
 * Copyright 2022 Norbert Takacs
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once
#include <list>
#include <string>
#include "XPLMDisplay.h"
#include "XPLMGraphics.h"
#include "XPLMPlugin.h"
#include "XPLMPlanes.h"
#include "XPLMProcessing.h"
#include "XPWidgets.h"
#include "XPWidgetDefs.h"
#include "XPStandardWidgets.h"

#define OBJECT_POINTER_ID 1101

class MessageWindow
{
private:
	int x, y;
	int width, height;

	XPLMWindowID window_id;
	XPLMWindowID clear_button_id;
	XPLMWindowID clear_button_caption_id;
	XPLMWindowID text_box_id;
	std::string title;
	std::list<std::string> messages;
	bool wrap_lines;
	int char_height;
	int char_width;
	XPLMFontID font;
	void print_log_message_lines();
public:
	MessageWindow(std::string _title);
	~MessageWindow();
	int	on_window_event(XPWidgetMessage widget_message, XPWidgetID  widget_id, intptr_t  in_param1, intptr_t  in_param2);
	void set_font(XPLMFontID _font);
	void append_line(std::string& _text);
	void append_multi_lines(std::list<std::string>& _lines);
	void clear_text();
	void show();
	void hide();
};
