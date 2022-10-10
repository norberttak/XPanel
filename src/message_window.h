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

class MessageWindow
{
private:
	XPLMWindowID windowID;
	XPLMWindowID g_window;
	std::string title;
	std::list<std::string> messages;
public:
	MessageWindow(std::string _title, std::list<std::string> _messages, bool show);
	~MessageWindow();
	std::list<std::string>& get_messages();
	void show();
	void hide();
};
