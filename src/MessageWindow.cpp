/*
 * Copyright 2022 Norbert Takacs
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */


#include "XPLMGraphics.h"
#include "MessageWindow.h"

int widget_handler(XPWidgetMessage inMessage, XPWidgetID inWidget, intptr_t inParam1, intptr_t inParam2)
{
	int exists=0;
	MessageWindow* message_window = (MessageWindow*)(XPGetWidgetProperty(inWidget, xpProperty_Object, &exists));

	if (!message_window || exists == 0)
		return 0;

	if (inMessage == xpMessage_CloseButtonPushed)
	{
		message_window->hide();
		return 1;
	}

	return 0;
}

void MessageWindow::draw_message_caption_widgets()
{
	// clear any previous captions
	for (XPWidgetID wid : message_caption_widgets)
	{
		XPDestroyWidget(wid, 1);
	}

	// print log lines:
	int vertical_offset = 50;
	for (std::string msg : get_messages())
	{
		XPWidgetID widget_id = XPCreateWidget(x+10, y - vertical_offset, x + 100, y - vertical_offset - 150, 1, msg.c_str(), 0, message_window_widget, xpWidgetClass_Caption);
		message_caption_widgets.push_back(widget_id);
		vertical_offset += 20;
	}
}

XPWidgetID MessageWindow::add_button(XPWidgetID home_widget, int _x, int _y, const char* title)
{
	// Draw button(s)
	XPWidgetID button_id = XPCreateWidget(_x, _y, _x + 100, _y - 100, 1, title, 0, home_widget, xpWidgetClass_Button);
	XPSetWidgetProperty(button_id, xpProperty_ButtonType, xpPushButton);

	return button_id;
}

std::list<std::string>& MessageWindow::get_messages()
{
	return messages;
}

void MessageWindow::clear_messages()
{
	messages.clear();
	draw_message_caption_widgets();
}

MessageWindow::MessageWindow(std::string _title)
{
	x = 50;
	y = 712;
	width = 974;
	height = 662;

	int x2 = x + width;
	int y2 = y - height;

	message_window_widget = XPCreateWidget(x, y, x2, y2, 0, _title.c_str(), 1, NULL, xpWidgetClass_MainWindow);
	XPSetWidgetProperty(message_window_widget, xpProperty_Object, (intptr_t)(this));
	XPSetWidgetProperty(message_window_widget, xpProperty_MainWindowHasCloseBoxes, 1);

	add_button(message_window_widget, x+10, y-15, "Clear");
	add_button(message_window_widget, x+300, y-15, "Close");

	XPAddWidgetCallback(message_window_widget, widget_handler);
}

void MessageWindow::add_messages(std::list<std::string>& _messages)
{
	messages.merge(_messages);
	draw_message_caption_widgets();
}

void MessageWindow::show()
{
	if (!message_window_widget)
		return;

	XPShowWidget(message_window_widget);
}

void MessageWindow::hide()
{
	if (!message_window_widget)
		return;

	XPHideWidget(message_window_widget);
}

MessageWindow::~MessageWindow()
{
	XPDestroyWidget(message_window_widget, 1);
	message_window_widget = NULL;
}
