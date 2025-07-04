/*
 * Copyright 2022 Norbert Takacs
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "MessageWindow.h"
#include "core/Logger.h"

 // C event handler for widgets. This will send the event to a MessageWindow instance specified by custom property OBJECT_POINTER_ID
int	window_event_handler(XPWidgetMessage widget_message, XPWidgetID  widget_id, intptr_t  in_param1, intptr_t  in_param2)
{
	if (widget_message == xpMessage_CloseButtonPushed)
	{
		XPHideWidget(widget_id);
		return 1;
	}

	intptr_t message_window_obj_ptr = XPGetWidgetProperty(widget_id, OBJECT_POINTER_ID, NULL);
	MessageWindow* window = reinterpret_cast<MessageWindow*>(message_window_obj_ptr);
	if (window == NULL)
		return 0;

	int result = 0;
	try {
		result = window->on_window_event(widget_message, widget_id, in_param1, in_param2);
	}
	catch (std::exception& e)
	{
		Logger(TLogLevel::logTRACE) << "MessageWindow: exception happend in C handler: " << e.what() << std::endl;
		result = 0;
	}

	return result;
}

MessageWindow::MessageWindow(std::string _title)
{
	// it's a fixed size 600x600 window. initial position (top left corner) is x=50, y=50
	width = 600;
	height = 600;
	x = 50;
	y = height + 50;

	wrap_lines = true; // wrap long lines

	window_id = XPCreateWidget(x, y, x + width, y - height, 1, _title.c_str(), 1, NULL, xpWidgetClass_MainWindow);
	Logger(TLogLevel::logTRACE) << "MessageWindow window_id:" << window_id << std::endl;
	XPSetWidgetProperty(window_id, xpProperty_MainWindowHasCloseBoxes, 1);
	XPSetWidgetProperty(window_id, xpProperty_MainWindowType, xpMainWindowStyle_Translucent);
	XPSetWidgetProperty(window_id, OBJECT_POINTER_ID, reinterpret_cast<intptr_t>(this));
	XPAddWidgetCallback(window_id, window_event_handler);

	clear_button_id = XPCreateWidget(x + 15, y - 15, x + 5 + 90, y - 5 - 60, 1, "", 0, window_id, xpWidgetClass_Button);
	Logger(TLogLevel::logTRACE) << "MessageWindow clear_button_id:" << clear_button_id << std::endl;
	XPSetWidgetProperty(clear_button_id, xpProperty_ButtonType, xpPushButton);
	XPSetWidgetProperty(clear_button_id, xpProperty_ButtonBehavior, xpButtonBehaviorPushButton);
	XPSetWidgetProperty(clear_button_id, xpProperty_ButtonState, 0);
	XPSetWidgetProperty(clear_button_id, OBJECT_POINTER_ID, reinterpret_cast<intptr_t>(this));
	XPAddWidgetCallback(clear_button_id, window_event_handler);

	clear_button_caption_id = XPCreateWidget(x + 15, y - 30, x + 35 + 100, y - 5 - 40, 1, "Clear logs", 0, window_id, xpWidgetClass_Caption);
	Logger(TLogLevel::logTRACE) << "MessageWindow clear_button_caption_id:" << clear_button_caption_id << std::endl;
	XPSetWidgetProperty(clear_button_caption_id, xpProperty_CaptionLit, 1);
	XPSetWidgetProperty(clear_button_caption_id, OBJECT_POINTER_ID, reinterpret_cast<intptr_t>(this));

	set_font(xplmFont_Proportional);

	text_box_id = XPCreateCustomWidget(x + 15, y - 50, x + 500, y - 50 - 500, 1, "", 0, window_id, window_event_handler);
	Logger(TLogLevel::logTRACE) << "MessageWindow text_box_id:" << text_box_id << std::endl;
	XPSetWidgetProperty(text_box_id, OBJECT_POINTER_ID, reinterpret_cast<intptr_t>(this));
}

void MessageWindow::set_font(XPLMFontID _font)
{
	if (_font != xplmFont_Proportional && _font != xplmFont_Basic)
		return;

	font = _font;
	XPLMGetFontDimensions(font, &char_width, &char_height, NULL);
	char_height += 5; // line spacing

	Logger(TLogLevel::logTRACE) << "MessageWindow: font height:" << char_height << " font width:" << char_width << std::endl;
}

void MessageWindow::append_line(std::string& _text)
{
	messages.push_back(_text);
}

void MessageWindow::append_multi_lines(std::list<std::string>& _lines)
{
	messages.merge(_lines);
}

void MessageWindow::clear_text()
{
	Logger(TLogLevel::logTRACE) << "MessageWindow clear text" << std::endl;
	messages.clear();
}

void MessageWindow::print_log_message_lines()
{
	int left, top, right, bottom;
	XPGetWidgetGeometry(text_box_id, &left, &top, &right, &bottom);

	float white[] = { 1.0, 1.0, 1.0 };
	int line_index = 0;
	const int line_x_pos = left + 5;
	const int line_y_pos = top - 15;

	for (std::string& msg_line : messages)
	{
		if (line_index * char_height > height)
			break;

		if (wrap_lines == true)
		{
			int remaining_line_width = char_width * msg_line.size();
			int offset = 0;
			while (remaining_line_width >= width)
			{
				XPLMDrawString(white, line_x_pos, line_y_pos - (line_index * char_height), (char*)msg_line.substr(offset, (int)(width / char_width)).c_str(), NULL, font);
				line_index++;
				offset += (int)(width / char_width);
				remaining_line_width -= width;
			}
			if (remaining_line_width > 0)
			{
				XPLMDrawString(white, line_x_pos, line_y_pos - (line_index * char_height), (char*)msg_line.substr(offset, remaining_line_width).c_str(), NULL, font);
				line_index++;
			}
		}
		else
		{
			XPLMDrawString(white, line_x_pos, line_y_pos - (line_index * char_height), (char*)msg_line.c_str(), NULL, font);
			line_index++;
		}
	}
}

int	MessageWindow::on_window_event(XPWidgetMessage widget_message, XPWidgetID  widget_id, intptr_t  in_param1, intptr_t  in_param2)
{
	(void)in_param1;
	(void)in_param2;
	if (widget_id == clear_button_id && widget_message == xpMsg_PushButtonPressed)
	{
		messages.clear();
		return 1;
	}

	if (widget_id == text_box_id)
	{
		if (widget_message == xpMsg_Draw || widget_message == xpMsg_Paint)
		{
			Logger(TLogLevel::logTRACE) << "MessageWindow repaint" << std::endl;
			print_log_message_lines();
			return 1;
		}
	}

	return 0;
}

void MessageWindow::show()
{
	Logger(TLogLevel::logTRACE) << "MessageWindow show" << std::endl;
	XPShowWidget(window_id);
}

void MessageWindow::hide()
{
	Logger(TLogLevel::logTRACE) << "MessageWindow hide" << std::endl;
	XPHideWidget(window_id);
}

MessageWindow::~MessageWindow()
{
	messages.clear();
	XPDestroyWidget(window_id, 1);
}