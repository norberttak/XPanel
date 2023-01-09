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
#include "ArduinoHomeCockpit.h"
#include "LuaHelper.h"
#include "ConfigParser.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

void test_hid_set_read_data(unsigned char* data, size_t length);
int test_get_dataref_value(const char* datarefstr);
int test_hid_get_vid();
int test_hid_get_pid();
std::string test_get_last_command();
void test_hid_get_write_data(unsigned char* data, size_t length);
void test_flight_loop(std::vector<DeviceConfiguration> &config);

namespace test
{
	TEST_CLASS(test_generic_display)
	{
	private:
		Configuration config;
		Configparser* p;
		ArduinoHomeCockpit* device;
		std::thread* t;
	public:
		TEST_METHOD_INITIALIZE(TestGenericDisplayInit)
		{
			p = new Configparser();
			int result = p->parse_file("../../test/test-arduino-home.ini", config);
			Assert::AreEqual(0, result);

			LuaHelper::get_instace()->init();
			LuaHelper::get_instace()->load_script_file("../../test/" + config.script_file);

			device = new ArduinoHomeCockpit(config.device_configs[0]);
			device->connect();
			device->start();
			t = new std::thread(&ArduinoHomeCockpit::thread_func, (ArduinoHomeCockpit*)device);
		}
		TEST_METHOD(TestAltimeterGauge)
		{
			XPLMDataRef dataref = XPLMFindDataRef("sim/altimeter_gauge");
			XPLMSetDatai(dataref, 0xFABA);

			test_flight_loop(config.device_configs);
			std::this_thread::sleep_for(150ms);
			
			unsigned char write_buffer[9];
			test_hid_get_write_data(write_buffer, sizeof(write_buffer));
			Assert::AreEqual(0xBA, (int)write_buffer[5]);
			Assert::AreEqual(0xFA, (int)write_buffer[6]);
		}

		TEST_METHOD(TestVariometerGauge)
		{
			XPLMDataRef dataref = XPLMFindDataRef("sim/test/variometer");
			XPLMSetDatai(dataref, -2000); // -2000 --> 0x87D0 (MSB set to 1 as number is negative)

			test_flight_loop(config.device_configs);
			std::this_thread::sleep_for(150ms);

			unsigned char write_buffer[9];
			test_hid_get_write_data(write_buffer, sizeof(write_buffer));
			Assert::AreEqual(0xD0, (int)write_buffer[3]);
			Assert::AreEqual(0x87, (int)write_buffer[4]);
		}

		TEST_METHOD(TestConstGauge)
		{
			test_flight_loop(config.device_configs);
			std::this_thread::sleep_for(150ms);

			unsigned char write_buffer[9];
			test_hid_get_write_data(write_buffer, sizeof(write_buffer));
			Assert::AreEqual(0x08, (int)write_buffer[7]);
		}

		TEST_METHOD_CLEANUP(TestGenericDisplayCleanup)
		{
			device->stop(0);
			t->join();
		}
	};
}
