/*
 * Copyright 2023 Norbert Takacs
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <filesystem>
#include "XPLMDefs.h"
#include "XPLMGraphics.h"
#include "XPLMPlugin.h"
#include "XPLMDisplay.h"
#include "XPWidgetDefs.h"

#include "CppUnitTest.h"
#include "core/ConfigParser.h"
#include "log-message-window/MessageWindow.h"

int test_invoke_widget_callback(XPWidgetMessage message, XPWidgetID widget_id, intptr_t param1, intptr_t param2);

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace test
{
	TEST_CLASS(TestLogWindow)
	{
	private:

	public:
		TEST_METHOD_INITIALIZE(TestLogWindowInit)
		{

		}

		TEST_METHOD(TestLogWindowCallback)
		{
			MessageWindow message_window("test window");
			message_window.append_line(std::string("test message 1"));
			message_window.append_line(std::string("test message 2"));
			message_window.show();

			Assert::AreEqual(1, test_invoke_widget_callback(xpMsg_Draw, NULL, 0, 0), L"invoke all callback functions: failed", 0);
		}

		TEST_METHOD_CLEANUP(TestLogWindowCleanup)
		{

		}
	};
}