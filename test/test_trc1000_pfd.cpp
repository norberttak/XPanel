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
#include "TRC1000PFD.h"
#include "LuaHelper.h"
#include "ConfigParser.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

void test_hid_set_read_data(unsigned char* data, size_t length);
int test_get_dataref_value(const char* datarefstr);
int test_hid_get_vid();
int test_hid_get_pid();
std::string test_get_last_command();
bool test_is_command_in_queue(std::string cmd_str);
int test_get_command_queue_size();
void test_clear_command_queue();
void test_hid_get_write_data(unsigned char* data, size_t length);
void test_flight_loop(std::vector<DeviceConfiguration> &config);


namespace test
{
	TEST_CLASS(test_trc1000_pfd_panel)
	{
	private:
		Configuration config;
		Configparser* p;
		TRC1000PFD* device;
		std::thread* t;
	public:
		TEST_METHOD_INITIALIZE(TestTrc1000PFDPanelInit)
		{
			p = new Configparser();
			int result = p->parse_file("../../test/test-trc1000-pfd.ini", config);
			Assert::AreEqual(0, result);

			LuaHelper::get_instace()->init();
			LuaHelper::get_instace()->load_script_file("../../test/" + config.script_file);

			device = new TRC1000PFD(config.device_configs[0]);
			device->connect();
			device->start();
			t = new std::thread(&TRC1000PFD::thread_func, (TRC1000PFD*)device);
			LuaHelper::get_instace()->register_hid_device(device);
		}

		TEST_METHOD(Test_VID_PID)
		{
			Assert::AreEqual(0xd59, test_hid_get_vid());
			Assert::AreEqual(0x2ae, test_hid_get_pid());
		}

		TEST_METHOD(TestNAVButton)
		{
			unsigned char buffer[8] = { 0xC1, 0,0,0,0,0,0,0 };
			test_hid_set_read_data(buffer, sizeof(buffer));
			std::this_thread::sleep_for(150ms);

			buffer[1] = 0x40;
			test_hid_set_read_data(buffer, sizeof(buffer));
			std::this_thread::sleep_for(150ms);

			test_flight_loop(config.device_configs);
			Assert::IsTrue(test_is_command_in_queue("sim/GPS/g1000n1_nav_ONCE"));
		}

		TEST_METHOD(TestEncoderIncrement)
		{
			unsigned char buffer[8] = { 0xC2, 127,0,0,0,0,0,0 };
			test_hid_set_read_data(buffer, sizeof(buffer));
			std::this_thread::sleep_for(150ms);

			buffer[1] = 129; // rotate encoder NAV_INNER by + 2
			test_hid_set_read_data(buffer, sizeof(buffer));

			std::this_thread::sleep_for(150ms);
			test_flight_loop(config.device_configs);
			Assert::IsTrue(test_is_command_in_queue("sim/GPS/g1000n1_nav_inner_up_ONCE"));
		}

		TEST_METHOD(TestEncoderIncrement2)
		{
			unsigned char buffer[8] = { 0xC2, 255,0,0,0,0,0,0 };
			test_hid_set_read_data(buffer, sizeof(buffer));
			std::this_thread::sleep_for(150ms);

			buffer[1] = 2; // rotate encoder NAV_INNER by + 2
			test_hid_set_read_data(buffer, sizeof(buffer));

			std::this_thread::sleep_for(150ms);
			test_flight_loop(config.device_configs);
			Assert::IsTrue(test_is_command_in_queue("sim/GPS/g1000n1_nav_inner_up_ONCE"));
		}

		TEST_METHOD(TestEncoderIncrement3)
		{
			unsigned char buffer[8] = { 0xC2, 0,0,0,0,0,0,0 };
			test_hid_set_read_data(buffer, sizeof(buffer));
			std::this_thread::sleep_for(150ms);

			buffer[1] = 1;
			buffer[2] = 255;
			test_hid_set_read_data(buffer, sizeof(buffer));

			std::this_thread::sleep_for(150ms);
			test_flight_loop(config.device_configs);
			std::this_thread::sleep_for(150ms);
			Assert::IsFalse(test_is_command_in_queue("sim/GPS/g1000n1_nav_inner_up_ONCE"));
		}

		TEST_METHOD(TestEncoderDecrement)
		{
			unsigned char buffer[8] = { 0xC2, 2,0,0,0,0,0,0 };
			test_hid_set_read_data(buffer, sizeof(buffer));
			std::this_thread::sleep_for(150ms);

			buffer[1] = 0; // rotate encoder NAV_INNER by - 2
			test_hid_set_read_data(buffer, sizeof(buffer));
			std::this_thread::sleep_for(150ms);

			test_flight_loop(config.device_configs);
			Assert::IsTrue(test_is_command_in_queue("sim/GPS/g1000n1_nav_inner_down_ONCE"));
		}

		TEST_METHOD(TestEncoderRegisterOverflow_255_0)
		{
			unsigned char buffer[8] = { 0xC2, 255,0,0,0,0,0,0 };
			test_hid_set_read_data(buffer, sizeof(buffer));
			std::this_thread::sleep_for(150ms);

			buffer[1] = 1; // rotate encoder NAV_INNER by +2
			test_hid_set_read_data(buffer, sizeof(buffer));
			std::this_thread::sleep_for(150ms);

			test_clear_command_queue();
			test_flight_loop(config.device_configs);

			Assert::AreEqual(1, test_get_command_queue_size());
		}

		TEST_METHOD(TestEncoderRegisterOverflow_0_255)
		{
			unsigned char buffer[8] = { 0xC2, 1,0,0,0,0,0,0 };
			test_hid_set_read_data(buffer, sizeof(buffer));
			std::this_thread::sleep_for(150ms);

			buffer[1] = 255; // rotate encoder NAV_INNER by -1
			test_hid_set_read_data(buffer, sizeof(buffer));
			std::this_thread::sleep_for(150ms);

			test_flight_loop(config.device_configs);

			Assert::AreEqual(1, test_get_command_queue_size());
		}

		TEST_METHOD_CLEANUP(TestTrc1000PFDPanelCleanup)
		{
			device->stop(0);
			t->join();
		}

	};
}