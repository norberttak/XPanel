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
#include "XPStandardWidgets.h"
#include "XPWidgets.h"

class MessageWindow
{
private:
	XPWidgetID message_window_widget = NULL;
	std::string title;
	std::list<std::string> messages;
	std::list<XPWidgetID> message_caption_widgets;
	int x;
	int y;
	int width;
	int height;
	XPWidgetID add_button(XPWidgetID home_widget, int _x, int _y, const char* title);
	void draw_message_caption_widgets();
public:
	MessageWindow(std::string _title);
	~MessageWindow();
	std::list<std::string>& get_messages();
	void clear_messages();
	void add_messages(std::list<std::string>& _messages);
	void show();
	void hide();
};
