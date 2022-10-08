/*
 * Copyright 2022 Norbert Takacs
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "message_window.h"

int dummy_mouse_handler(
	XPLMWindowID,
	int,
	int,
	XPLMMouseStatus,
	void*)
{
	return 0;
}

int dummy_wheel_handler(
	XPLMWindowID,
	int,
	int,
	int,
	int,
	void*)
{
	return 0;
}

void dummy_key_handler(
	XPLMWindowID,
	char,
	XPLMKeyFlags,
	char,
	void*,
	int)
{

}

XPLMCursorStatus dummy_cursor_status_handler(
	XPLMWindowID,
	int,
	int,
	void*)
{
	return xplm_CursorDefault;
}

void draw_message_display_window(XPLMWindowID in_window_id, void* in_refcon)
{
	MessageWindow* msg_window = (MessageWindow*)in_refcon;

	// Mandatory: We *must* set the OpenGL state before drawing
	// (we can't make any assumptions about it)
	XPLMSetGraphicsState(
		0 /* no fog */,
		0 /* 0 texture units */,
		0 /* no lighting */,
		0 /* no alpha testing */,
		1 /* do alpha blend */,
		1 /* do depth testing */,
		0 /* no depth writing */
	);

	int l, t, r, b;
	XPLMGetWindowGeometry(in_window_id, &l, &t, &r, &b);

	float col_white[] = { 1.0, 1.0, 1.0 }; // red, green, blue

	int vertical_offset = 30;
	for (std::string msg : msg_window->get_messages())
	{
		XPLMDrawString(col_white, l + 10, t - vertical_offset, (char*)msg.c_str(), NULL, xplmFont_Proportional);
		vertical_offset -= 20;
	}
}

std::list<std::string>& MessageWindow::get_messages()
{
	return messages;
}

MessageWindow::MessageWindow(std::string _title, std::list<std::string> _messages, bool show)
{
	messages = _messages;

	// code based on this example:
	// https://developer.x-plane.com/code-sample/hello-world-sdk-3/
	XPLMCreateWindow_t params;
	params.structSize = sizeof(params);
	params.visible = show ? 1 : 0;
	params.drawWindowFunc = draw_message_display_window;

	params.handleMouseClickFunc = dummy_mouse_handler;
	params.handleRightClickFunc = dummy_mouse_handler;
	params.handleMouseWheelFunc = dummy_wheel_handler;
	params.handleKeyFunc = dummy_key_handler;
	params.handleCursorFunc = dummy_cursor_status_handler;
	params.refcon = (void*)this;
	params.layer = xplm_WindowLayerFloatingWindows;

	params.decorateAsFloatingWindow = xplm_WindowDecorationRoundRectangle;

	int left, bottom, right, top;
	XPLMGetScreenBoundsGlobal(&left, &top, &right, &bottom);
	params.left = left + 50;
	params.bottom = bottom + 150;
	params.right = params.left + 800;
	params.top = params.bottom + 300;

	g_window = XPLMCreateWindowEx(&params);

	// Position the window as a "free" floating window, which the user can drag around
	XPLMSetWindowPositioningMode(g_window, xplm_WindowPositionFree, -1);
	// Limit resizing our window: maintain a minimum width/height of 100 boxels and a max width/height of 300 boxels
	XPLMSetWindowResizingLimits(g_window, 200, 200, 1200, 800);

	XPLMSetWindowTitle(g_window, _title.c_str());
}

MessageWindow::~MessageWindow()
{
	XPLMDestroyWindow(g_window);
	g_window = NULL;
}
