/*
 * Copyright 2026 Norbert Takacs
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
using namespace std::literals;

void test_hid_get_write_data(unsigned char* data, size_t length);
void test_flight_loop(Device* dev);

namespace test
{
	// Regression test for https://github.com/norberttak/XPanel/issues/124:
	// a plain [display:] section (GenericDisplay, as opposed to [multi_display:])
	// never refreshed on a Saitek Multi Panel because nr_of_bytes was never
	// initialized for GenericDisplay objects on this device class - only
	// multi_displays received a set_nr_bytes() call in the SaitekMultiPanel
	// constructor. With nr_of_bytes left uninitialized, get_decimal_components()/
	// get_binary_components() never actually wrote into the display's slice of
	// the write buffer, even though the underlying dataref kept changing.
	TEST_CLASS(test_multi_panel_generic_display)
	{
	private:
		Configuration config;
		ConfigParser* p;
		SaitekMultiPanel* device;
		std::thread* t;
	public:
		TEST_METHOD_INITIALIZE(TestMultiPanelGenericDisplayInit)
		{
			p = new ConfigParser();
			int result = p->parse_file("../../test/test-multi-panel-plain-display.ini", config);
			Assert::AreEqual(0, result);

			LuaHelper::get_instance()->init();
			LuaHelper::get_instance()->load_script_file("../../test/" + config.script_file);

			device = new SaitekMultiPanel(config.class_configs[0]);
			device->connect();
			device->start();
			t = new std::thread(&SaitekMultiPanel::thread_func, (SaitekMultiPanel*)device);
			LuaHelper::get_instance()->register_hid_device(device);
		}

		TEST_METHOD(TestPlainDisplayRefreshesOnMultiPanel)
		{
			XPLMDataRef dataref = XPLMFindDataRef("sim/custom/gauges/compas/pkp_helper_course_L");
			XPLMSetDatai(dataref, 42);

			test_flight_loop(device);
			std::this_thread::sleep_for(150ms);

			unsigned char write_buffer[13];
			test_hid_get_write_data(write_buffer, sizeof(write_buffer));

			// GenericDisplay defaults to binary encoding (use_bcd=false) unless
			// bcd="yes" is set on the [display:] section, so 42 is written as
			// raw little-endian bytes into the MULTI_DISPLAY_UP slot (reg_index=1).
			Assert::AreEqual(42, (int)write_buffer[1]);
			Assert::AreEqual(0, (int)write_buffer[2]);
			Assert::AreEqual(0, (int)write_buffer[3]);
			Assert::AreEqual(0, (int)write_buffer[4]);
			Assert::AreEqual(0, (int)write_buffer[5]);
		}

		TEST_METHOD_CLEANUP(TestMultiPanelGenericDisplayCleanup)
		{
			device->stop(0);
			t->join();
		}
	};
}
