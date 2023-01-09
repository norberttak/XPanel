/*
 * Copyright 2022 Norbert Takacs
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "XPLMDefs.h"
#include "XPLMPlugin.h"
#include "XPLMPlanes.h"
#include "XPLMDisplay.h"
#include "LuaHelper.h"
#include "CppUnitTest.h"
#include "FIPDevice.h"
#include "Configparser.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
void test_set_aircraft_path_and_filename(char* file_name, char* path);
int test_fip_get_led_state(int led_index);
void test_fip_set_button_states(uint16_t _button_states);
void test_fip_set_current_page(int page);
void test_flight_loop(std::vector<DeviceConfiguration> &config);
void test_fip_get_image(unsigned char* buffer, size_t buffer_size);

namespace test
{
	TEST_CLASS(TestFIP)
	{
	private:
		Configuration config;
		Configparser* p;
		FIPDevice* fip_device;
		std::thread* t;
		std::string airspeed_dataref_str = "sim/cockpit2/gauges/indicators/airspeed_kts_pilot";
		XPLMDataRef airspeed_dataref;
	public:
		TEST_METHOD_INITIALIZE(TestFIPInit)
		{
			test_set_aircraft_path_and_filename("test.acf", "./");

			p = new Configparser();
			int result = p->parse_file("../../test/test-fip-screen-config.ini", config);
			Assert::AreEqual(0, result);

			LuaHelper::get_instace()->init();
			LuaHelper::get_instace()->load_script_file("../../test/" + config.script_file);

			fip_device = new FIPDevice(config.device_configs[0]);
			fip_device->connect();
			fip_device->start();
			t = new std::thread(&FIPDevice::thread_func, (FIPDevice*)fip_device);

			airspeed_dataref = XPLMFindDataRef(airspeed_dataref_str.c_str());
			XPLMSetDatai(airspeed_dataref, 0);

			test_fip_set_current_page(0);
		}

		TEST_METHOD(TestFIPButtonState)
		{
			test_fip_set_button_states(4);
			Assert::AreEqual(4, (int)fip_device->get_button_state());

			test_fip_set_button_states(0);
			Assert::AreEqual(0, (int)fip_device->get_button_state());
		}

		TEST_METHOD(TestFIPAirSpeedChange)
		{
			XPLMSetDatai(airspeed_dataref, 0);
			test_flight_loop(config.device_configs);
			std::this_thread::sleep_for(150ms);
			unsigned char fip_image_buffer[240 * 320 * 3];
			test_fip_get_image(fip_image_buffer, 240 * 320 * 3);
			Assert::AreEqual(0, (int)fip_image_buffer[0]);

			XPLMSetDatai(airspeed_dataref, 150);
			test_flight_loop(config.device_configs);
			std::this_thread::sleep_for(150ms);
			test_fip_get_image(fip_image_buffer, 240 * 320 * 3);
			Assert::AreEqual(0, (int)fip_image_buffer[0]);
		}

		TEST_METHOD(TestFIPBMPPadding)
		{
			/* Page 2 contains one layer with the BMP file bmp_test_padding.bmp
			   This file contains a 5x3 pixel where the middle column is black, other
			   pixels are white. Due to the BMP specification every row contains 1 additional
			   byte as padding in each row. Those padding bytes shall be handled by RawBMP class.
			   The 5x3 BMP layer will be put to 0,0 position */
			test_fip_set_current_page(2);
			test_flight_loop(config.device_configs);
			std::this_thread::sleep_for(150ms);

			const int row_count = 240; //height
			const int col_count = 320; //width
			const int bytes_per_pixel = 3;

			unsigned char fip_buffer[col_count * row_count * bytes_per_pixel];
			test_fip_get_image(&fip_buffer[0], col_count * row_count * bytes_per_pixel);

			//col 0 --> shall be white (RGB 255,255,255)
			Assert::AreEqual(255, (int)fip_buffer[0]);
			Assert::AreEqual(255, (int)fip_buffer[1]);
			Assert::AreEqual(255, (int)fip_buffer[2]);

			//col 1 --> shall be white (RGB 255,255,255)
			Assert::AreEqual(255, (int)fip_buffer[3]);
			Assert::AreEqual(255, (int)fip_buffer[4]);
			Assert::AreEqual(255, (int)fip_buffer[5]);

			//col 2 --> shall be black (RGB 0,0,0)
			Assert::AreEqual(0, (int)fip_buffer[6]);
			Assert::AreEqual(0, (int)fip_buffer[7]);
			Assert::AreEqual(0, (int)fip_buffer[8]);

			//col 3 --> shall be white (RGB 255,255,255)
			Assert::AreEqual(255, (int)fip_buffer[9]);
			Assert::AreEqual(255, (int)fip_buffer[10]);
			Assert::AreEqual(255, (int)fip_buffer[11]);

			//col 4 --> shall be white (RGB 255,255,255)
			Assert::AreEqual(255, (int)fip_buffer[12]);
			Assert::AreEqual(255, (int)fip_buffer[13]);
			Assert::AreEqual(255, (int)fip_buffer[14]);
		}

		TEST_METHOD_CLEANUP(TestFIPCleanup)
		{
			fip_device->stop(0);
			t->join();
		}
	};
}