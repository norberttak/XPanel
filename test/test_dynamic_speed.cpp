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
#include "saitek-multi/SaitekMultiPanel.h"
#include "core/LuaHelper.h"
#include "core/ConfigParser.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

void test_hid_set_read_data(unsigned char* data, size_t length);
int test_get_dataref_value(const char* datarefstr);
void test_set_dataref_value(const char* datarefstr, int value);
int test_hid_get_vid();
int test_hid_get_pid();
std::string test_get_last_command();
void test_hid_get_write_data(unsigned char* data, size_t length);
void test_flight_loop(std::vector<DeviceConfiguration> &config);
void test_hid_mock_init();

namespace test
{
	void rotate_knob_plus(int rot_nr, std::chrono::milliseconds delay_ms)
	{
		unsigned char buffer[4] = { 0x08,0,0,0 };

		for (int i = 0; i < rot_nr; i++)
		{
			buffer[0] |= 0x20; // set KNOB_PLUS to 1
			test_hid_set_read_data(buffer, sizeof(buffer));
			std::this_thread::sleep_for(delay_ms);

			buffer[0] &= (~0x20); // set KNOB_PLUS to 0
			test_hid_set_read_data(buffer, sizeof(buffer));
			std::this_thread::sleep_for(delay_ms);
		}
	}

	TEST_CLASS(test_dynamic_speed)
	{
	private:
		Configuration config;
		Configparser* p;
		SaitekMultiPanel* device;
		std::thread* t;
		std::string dataref_str = "test/dynamic_speed_test";
	public:
		TEST_METHOD_INITIALIZE(TestDynamicSpeedInit)
		{
			test_hid_mock_init();

			p = new Configparser();
			int result = p->parse_file("../../test/test-dynamic-speed.ini", config);
			Assert::AreEqual(0, result);

			LuaHelper::get_instace()->init();

			device = new SaitekMultiPanel(config.device_configs[0]);
			device->connect();
			device->start();
			t = new std::thread(&SaitekMultiPanel::thread_func, (SaitekMultiPanel*)device);
			XPLMSetDatai(XPLMFindDataRef(dataref_str.c_str()), 0);

			// set rotation switch to SW_HDG position
			unsigned char buffer[4] = { 0x08,0,0,0 };
			test_hid_set_read_data(buffer, sizeof(buffer));			
			test_set_dataref_value(dataref_str.c_str(), 0);
			std::this_thread::sleep_for(150ms);
		}

		TEST_METHOD(TestRotationKnobHigh)
		{
			rotate_knob_plus(6, 50ms);

			test_flight_loop(config.device_configs);

			// it shall run on x4 speed
			Assert::AreEqual(6 * 4, test_get_dataref_value(dataref_str.c_str()));
		}

		TEST_METHOD(TestRotationKnobMid)
		{
			rotate_knob_plus(2, 200ms);

			test_flight_loop(config.device_configs);

			// it shall run on x2 speed
			Assert::AreEqual(2*2, test_get_dataref_value(dataref_str.c_str()));
		}

		TEST_METHOD(TestRotationKnobNormal)
		{
			rotate_knob_plus(2, 500ms); // two rotations on the knob with normal speed

			test_flight_loop(config.device_configs);

			// // it shall run on x1 speed
			Assert::AreEqual(2, test_get_dataref_value(dataref_str.c_str()));
		}

		TEST_METHOD_CLEANUP(TestDynamicSpeedCleanup)
		{
			device->stop(0);
			t->join();
		}
	};
}
