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

#include "CppUnitTest.h"
#include "TRC1000.h"
#include "TRC1000Audio.h"
#include "LuaHelper.h"
#include "ConfigParser.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

void test_hid_set_read_data(unsigned char* data, size_t length);
void test_hid_get_write_data(unsigned char* data, size_t length);
int test_hid_get_vid();
int test_hid_get_pid();
std::string test_get_last_command();
bool test_is_command_in_queue(std::string cmd_str);
int test_get_command_queue_size();
void test_clear_command_queue();
void test_flight_loop(std::vector<DeviceConfiguration> config);


namespace test
{
	TEST_CLASS(test_trc1000_audio_panel)
	{
	private:
		Configuration config;
		Configparser* p;
		TRC1000Audio* device;
		std::thread* t;
	public:
		TEST_METHOD_INITIALIZE(TestTrc1000AudioPanelInit)
		{
			p = new Configparser();
			int result = p->parse_file("../../test/test-trc1000-audio.ini", config);
			Assert::AreEqual(0, result);

			LuaHelper::get_instace()->init();
			LuaHelper::get_instace()->load_script_file("../../test/" + config.script_file);

			device = new TRC1000Audio(config.device_configs[0]);
			device->connect();
			device->start();
			t = new std::thread(&TRC1000Audio::thread_func, (TRC1000Audio*)device);
			LuaHelper::get_instace()->register_hid_device(device);
		}

		TEST_METHOD(Test_VID_PID)
		{
			Assert::AreEqual(0xd59, test_hid_get_vid());
			Assert::AreEqual(0x2b0, test_hid_get_pid());
		}

		TEST_METHOD(TestCom2Button)
		{
			unsigned char buffer[8] = { 0xC2, 0,0,0,0,0,0,0 };
			test_hid_set_read_data(buffer, sizeof(buffer));
			std::this_thread::sleep_for(150ms);

			buffer[3] = 0x01;
			test_hid_set_read_data(buffer, sizeof(buffer));
			std::this_thread::sleep_for(150ms);

			test_flight_loop(config.device_configs);
			Assert::IsTrue(test_is_command_in_queue("sim/GPS/g1000n1_flc_ONCE"));
		}

		TEST_METHOD_CLEANUP(TestTrc1000PFDPanelCleanup)
		{
			device->stop(0);
			t->join();
		}

	};
}