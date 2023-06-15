/*
 * Copyright 2022 Norbert Takacs
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "XPLMDefs.h"
#include "XPLMGraphics.h"
#include "XPLMPlugin.h"
#include "XPLMPlanes.h"
#include "XPLMDisplay.h"
#include "core/LuaHelper.h"

#include "CppUnitTest.h"
#include "core/ConfigParser.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

std::string test_get_last_command();
int test_get_dataref_value(const char* datarefstr);
void test_set_dataref_value(const char* datarefstr, int value);

namespace test
{
	TEST_CLASS(test_lua)
	{
	private:
		//
	public:
		TEST_METHOD_INITIALIZE(TestLuaInit)
		{
			LuaHelper::get_instace()->init();
		}

		TEST_METHOD(TestLuaDoString)
		{
			LuaHelper::get_instace()->do_string("command_once(\"/sim/test\")");
			Assert::AreEqual("/sim/test_ONCE", test_get_last_command().c_str());

			LuaHelper::get_instace()->do_string("command_begin(\"/sim/test\")");
			Assert::AreEqual("/sim/test_BEGIN", test_get_last_command().c_str());

			LuaHelper::get_instace()->do_string("command_end(\"/sim/test\")");
			Assert::AreEqual("/sim/test_END", test_get_last_command().c_str());

			LuaHelper::get_instace()->do_string("set_dataref(\"/sim/test2\",123)");
			Assert::AreEqual(123, test_get_dataref_value("/sim/test2"));

			test_set_dataref_value("/sim/test3", 321);
			LuaHelper::get_instace()->do_string("get_dataref(\"/sim/test3\")");
		}

		TEST_METHOD(TestLuaLoadScript)
		{
			int res = LuaHelper::get_instace()->load_script_file("../../test/test-script.lua");
			
			Assert::AreEqual(EXIT_SUCCESS, res);

			//the script file sets the dataref "/sim/test/lua_script_test1" to value 567
			Assert::AreEqual(567, test_get_dataref_value("/sim/test/lua_script_test1"));
		}

		TEST_METHOD(TestLuaFlightLoop)
		{
			LuaHelper::get_instace()->load_script_file("../../test/test-script.lua");

			LuaHelper::get_instace()->do_flight_loop();
			Assert::AreEqual(12345, test_get_dataref_value("/sim/test/lua_flight_loop"));
		}

		TEST_METHOD_CLEANUP(TestLuaCleanup)
		{
			LuaHelper::get_instace()->close();
		}
	};
}
