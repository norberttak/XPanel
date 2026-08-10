/*
 * Copyright 2026 Norbert Takacs
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <filesystem>
#include <string>
#include "XPLMDefs.h"
#include "XPLMGraphics.h"
#include "XPLMPlugin.h"
#include "XPLMPlanes.h"
#include "XPLMDisplay.h"

#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

void test_set_aircraft_path_and_filename(char* file_name, char* path);
void test_call_registered_flight_loop();
void test_set_name_unavailable(const std::string& name);
void test_set_name_available(const std::string& name);
PLUGIN_API int XPluginStart(char* outName, char* outSig, char* outDesc);
PLUGIN_API void XPluginReceiveMessage(XPLMPluginID inFrom, int inMsg, void* inParam);
PLUGIN_API void XPluginStop(void);

extern bool plugin_already_initialized;

namespace test
{
	// wait_for_available must not block the main thread: init_and_start_xpanel_plugin()
	// registers wait_for_available_callback and returns immediately while the dataref/
	// commandref is still missing, then finishes init (asynchronously, via repeated flight
	// loop ticks) once it appears or the timeout elapses.
	TEST_CLASS(test_wait_for_available)
	{
	public:
		char out_name[16];
		char out_sig[16];
		char out_desc[64];
		const char* pending_name = "sim/test/pending_dataref";

		TEST_METHOD_INITIALIZE(TestWaitForAvailableInit)
		{
			std::filesystem::path aircraft_path = std::filesystem::absolute("../../test/wait-for-available/wait_for_available_test.acf");

			test_set_aircraft_path_and_filename(const_cast<char*>("wait_for_available_test.acf"), const_cast<char*>(aircraft_path.string().c_str()));
			test_set_name_unavailable(pending_name);

			XPluginStart(out_name, out_sig, out_desc);
		}

		TEST_METHOD(TestWaitForAvailableResolvesWithoutBlocking)
		{
			XPluginReceiveMessage(XPLM_PLUGIN_XPLANE, XPLM_MSG_AIRPORT_LOADED, NULL);
			Assert::IsFalse(plugin_already_initialized);

			// still unavailable: several ticks must not complete init
			for (int i = 0; i < 3; i++)
				test_call_registered_flight_loop();
			Assert::IsFalse(plugin_already_initialized);

			test_set_name_available(pending_name);
			test_call_registered_flight_loop();
			Assert::IsTrue(plugin_already_initialized);
		}

		TEST_METHOD_CLEANUP(TestWaitForAvailableCleanup)
		{
			test_set_name_available(pending_name);
			XPluginStop();
		}
	};
}
