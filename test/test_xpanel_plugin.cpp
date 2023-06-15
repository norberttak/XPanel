/*
 * Copyright 2022 Norbert Takacs
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <filesystem>
#include "XPLMDefs.h"
#include "XPLMGraphics.h"
#include "XPLMPlugin.h"
#include "XPLMPlanes.h"
#include "XPLMDisplay.h"

#include "CppUnitTest.h"
#include "saitek-multi/SaitekMultiPanel.h"
#include "core/ConfigParser.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

void test_set_aircraft_path_and_filename(char* file_name, char* path);
void test_call_registered_flight_loop();
void test_call_menu_handler(XPLMMenuID inMenuID, void* menuRef, void* itemRef);
PLUGIN_API int XPluginStart(char* outName, char* outSig, char* outDesc);
PLUGIN_API void XPluginReceiveMessage(XPLMPluginID inFrom, int inMsg, void* inParam);
PLUGIN_API void XPluginStop(void);

char outName[16];
char outSig[16];
char outDesc[64];

namespace test
{
	TEST_CLASS(test_plugin)
	{
	private:
		std::vector<Configuration> config;
		Configparser* p;
		SaitekMultiPanel* device;
		std::thread* t;
	public:
		TEST_METHOD_INITIALIZE(TestPluginInit)
		{	
			std::filesystem::path aircraft_path = std::filesystem::absolute("../../test/mock_airplane.acf");
			
			test_set_aircraft_path_and_filename("mock_airplane.acf", (char*)aircraft_path.string().c_str());

			XPluginStart(outName, outSig, outDesc);
			XPluginReceiveMessage(XPLM_PLUGIN_XPLANE, XPLM_MSG_AIRPORT_LOADED, NULL);
		}

		TEST_METHOD(TestPluginCheckPluginName)
		{
			Assert::AreEqual("XPanel ver " PLUGIN_VERSION, outName);
		}

		TEST_METHOD(TestPluginReload)
		{
			test_call_registered_flight_loop();
			std::this_thread::sleep_for(150ms);

			int MENU_ITEM_RELOAD = 0;
			test_call_menu_handler(0, NULL, &MENU_ITEM_RELOAD);
			Assert::AreEqual("XPanel ver " PLUGIN_VERSION, outName);
			
			test_call_registered_flight_loop();
			std::this_thread::sleep_for(150ms);
		}

		TEST_METHOD_CLEANUP(TestPluginCleanup)
		{
			XPluginStop();
		}
	};
}