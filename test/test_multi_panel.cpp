#include "XPLMDefs.h"
#include "XPLMGraphics.h"
#include "XPLMPlugin.h"
#include "XPLMPlanes.h"
#include "XPLMDisplay.h"

#include "CppUnitTest.h"
#include "SaitekMultiPanel.h"
#include "lua_helper.h"
#include "configparser.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

void test_hid_set_read_data(unsigned char* data, size_t length);
int test_get_dataref_value(const char* datarefstr);
int test_hid_get_vid();
int test_hid_get_pid();
std::string test_get_last_command();
void test_hid_get_write_data(unsigned char* data, size_t length);
void test_flight_loop(std::vector<DeviceConfiguration> config);

namespace test
{
	TEST_CLASS(test_multi_panel)
	{
	private:
		Configuration config;
		Configparser* p;
		SaitekMultiPanel* device;
		std::thread* t;
	public:
		TEST_METHOD_INITIALIZE(TestMultiPanelInit)
		{
			p = new Configparser();
			int result = p->parse_file("../../test/test-valid-config.ini", config);
			Assert::AreEqual(0, result);

			LuaHelper::get_instace()->init();			
			LuaHelper::get_instace()->load_script_file("../../test/" + config.script_file);
			
			device = new SaitekMultiPanel(config.device_configs[0]);
			device->connect();
			device->start();
			t = new std::thread(&SaitekMultiPanel::thread_func, (SaitekMultiPanel*)device);
		}

		TEST_METHOD(Test_VID_PID)
		{
			Assert::AreEqual(0x12AB, test_hid_get_vid());
			Assert::AreEqual(0x34CD, test_hid_get_pid());
		}

		//Test a button with DataRef action
		TEST_METHOD(TestAPMasterButton)
		{
			unsigned char buffer[4] = { 0x80,0,0,0 };
			test_hid_set_read_data(buffer, sizeof(buffer));
			std::this_thread::sleep_for(150ms);
			test_flight_loop(config.device_configs);
			Assert::AreEqual(1, test_get_dataref_value("/sim/hello/AP"));
			// this command is called by the lua script:
			Assert::AreEqual("/sim/test/lua/button_AP_BEGIN", test_get_last_command().c_str());

			buffer[0] = 0;
			test_hid_set_read_data(buffer, sizeof(buffer));
			std::this_thread::sleep_for(150ms);
			test_flight_loop(config.device_configs);
			Assert::AreEqual(0, test_get_dataref_value("/sim/hello/AP"));
			// this command is called by the lua script:
			Assert::AreEqual("/sim/test/lua/button_AP_END", test_get_last_command().c_str());
		}

		//Test a button with command begin-end action
		TEST_METHOD(TestNAVButton)
		{
			unsigned char buffer[4] = { 0,0x02,0,0 };
			test_hid_set_read_data(buffer, sizeof(buffer));
			std::this_thread::sleep_for(150ms);
			test_flight_loop(config.device_configs);
			std::string last_cmd = test_get_last_command();
			Assert::AreEqual("/sim/cmd/NAV_BEGIN", last_cmd.c_str());

			memset(buffer, 0, sizeof(buffer));
			test_hid_set_read_data(buffer, sizeof(buffer));
			std::this_thread::sleep_for(150ms);
			test_flight_loop(config.device_configs);
			last_cmd = test_get_last_command();
			Assert::AreEqual("/sim/cmd/NAV_END", last_cmd.c_str());
		}

		TEST_METHOD(TestTrimWheelUp)
		{
			int val_before = test_get_dataref_value("sim/custom/switchers/console/absu_pitch_wheel");
			unsigned char buffer[4] = { 0,0,0x08,0 };
			test_hid_set_read_data(buffer, sizeof(buffer));
			std::this_thread::sleep_for(150ms);
			test_flight_loop(config.device_configs);
			int val_after = test_get_dataref_value("sim/custom/switchers/console/absu_pitch_wheel");

			Assert::AreEqual(1, (val_after - val_before));

			memset(buffer, 0, sizeof(buffer));
			test_hid_set_read_data(buffer, sizeof(buffer));
			std::this_thread::sleep_for(150ms);
		}
		//Test a button with command once action
		TEST_METHOD(TestHDGButton)
		{
			unsigned char buffer[4] = { 0,0x01,0,0 };
			test_hid_set_read_data(buffer, sizeof(buffer));
			std::this_thread::sleep_for(150ms);
			test_flight_loop(config.device_configs);
			std::string last_cmd = test_get_last_command();
			Assert::AreNotEqual("/sim/cmd/HDG_ONCE", last_cmd.c_str());

			memset(buffer, 0, sizeof(buffer));
			test_hid_set_read_data(buffer, sizeof(buffer));
			std::this_thread::sleep_for(150ms);
			test_flight_loop(config.device_configs);
			last_cmd = test_get_last_command();
			Assert::AreEqual("/sim/cmd/HDG_ONCE", last_cmd.c_str());
		}

		TEST_METHOD(TestAltButtonLight)
		{
			XPLMDataRef dataref = XPLMFindDataRef("sim/custom/lights/button/absu_stab_h");
			// ALT button light set to LIT
			XPLMSetDatai(dataref, 1);
			test_flight_loop(config.device_configs); // evaluate triggers
			std::this_thread::sleep_for(150ms);
			unsigned char write_buffer[13];
			test_hid_get_write_data(write_buffer, sizeof(write_buffer));
			Assert::AreEqual(0x10, (int)write_buffer[11] & 0x10);

			// ALT button light set to UNLIT
			XPLMSetDatai(dataref, 0);
			test_flight_loop(config.device_configs); // evaluate triggers
			std::this_thread::sleep_for(150ms);
			test_hid_get_write_data(write_buffer, sizeof(write_buffer));
			Assert::AreEqual(0x00, (int)write_buffer[11] & 0x10);
		}

		TEST_METHOD(TestHdgButtonLight)
		{
			XPLMDataRef dataref = XPLMFindDataRef("sim/custom/lights/button/absu_zk");
			// HDG button light set to LIT
			XPLMSetDatai(dataref, 1);
			test_flight_loop(config.device_configs); // evaluate triggers
			std::this_thread::sleep_for(150ms);
			unsigned char write_buffer[13];
			test_hid_get_write_data(write_buffer, sizeof(write_buffer));
			Assert::AreEqual(0x02, (int)write_buffer[11] & 0x02);

			// HDG button light set to UNLIT
			XPLMSetDatai(dataref, 0);
			test_flight_loop(config.device_configs); // evaluate triggers
			std::this_thread::sleep_for(150ms);
			test_hid_get_write_data(write_buffer, sizeof(write_buffer));
			Assert::AreEqual(0x00, (int)write_buffer[11] & 0x02);
		}

		TEST_METHOD(TestApButtonLight)
		{
			// in the test this LED has a lua handler ('get_led_status()') which return with a constant 1 value
			test_flight_loop(config.device_configs); // evaluate triggers
			std::this_thread::sleep_for(150ms);
			unsigned char write_buffer[13];
			test_hid_get_write_data(write_buffer, sizeof(write_buffer));
			Assert::AreEqual(0x01, (int)write_buffer[11] & 0x01);
		}

		TEST_METHOD(TestMultiDisplayConfig)
		{
			Assert::AreEqual(2, (int)config.device_configs[0].multi_displays.size());
		}

		TEST_METHOD(TestMultiDisplay)
		{
			XPLMDataRef dataref = XPLMFindDataRef("sim/custom/gauges/compas/pkp_helper_course_L");
			XPLMSetDatai(dataref, 12345);

			// set rotation switch to ALT_SW position
			unsigned char buffer[4] = { 0x01,0,0,0 };
			test_hid_set_read_data(buffer, sizeof(buffer));
			std::this_thread::sleep_for(150ms);

			test_flight_loop(config.device_configs);
			std::this_thread::sleep_for(150ms);

			unsigned char write_buffer[13];
			test_hid_get_write_data(write_buffer, sizeof(write_buffer));
			Assert::AreEqual(1, (int)write_buffer[1]);
			Assert::AreEqual(2, (int)write_buffer[2]);
			Assert::AreEqual(3, (int)write_buffer[3]);
			Assert::AreEqual(4, (int)write_buffer[4]);
			Assert::AreEqual(5, (int)write_buffer[5]);

			XPLMSetDatai(dataref, 0);
			test_flight_loop(config.device_configs);
			std::this_thread::sleep_for(150ms);

			test_hid_get_write_data(write_buffer, sizeof(write_buffer));
			Assert::AreEqual(0, (int)write_buffer[1]);
			Assert::AreEqual(0, (int)write_buffer[2]);
			Assert::AreEqual(0, (int)write_buffer[3]);
			Assert::AreEqual(0, (int)write_buffer[4]);
			Assert::AreEqual(0, (int)write_buffer[5]);
		}

		TEST_METHOD(TestRotationKnobHdg)
		{
			std::string dataref_str = "sim/custom/gauges/compas/pkp_helper_course_L";

			// set rotation switch to SW_HDG position
			unsigned char buffer[4] = { 0x08,0,0,0 };
			test_hid_set_read_data(buffer, sizeof(buffer));
			std::this_thread::sleep_for(150ms);

			buffer[0] |= 0x20; // set KNOB_PLUS to 1
			test_hid_set_read_data(buffer, sizeof(buffer));
			std::this_thread::sleep_for(150ms);
			test_flight_loop(config.device_configs);
			Assert::AreEqual(1, test_get_dataref_value(dataref_str.c_str()));

			buffer[0] &= (~0x20); // set KNOB_PLUS to 0
			test_hid_set_read_data(buffer, sizeof(buffer));
			std::this_thread::sleep_for(150ms);
			test_flight_loop(config.device_configs);
			Assert::AreEqual(1, test_get_dataref_value(dataref_str.c_str()));

			buffer[0] |= 0x40; // set KNOB_MINUS to 1
			test_hid_set_read_data(buffer, sizeof(buffer));
			std::this_thread::sleep_for(150ms);
			test_flight_loop(config.device_configs);
			Assert::AreEqual(0, test_get_dataref_value(dataref_str.c_str()));

			buffer[0] &= (~0x40); // set KNOB_MINUS to 0
			test_hid_set_read_data(buffer, sizeof(buffer));
			std::this_thread::sleep_for(250ms);
			test_flight_loop(config.device_configs);
			Assert::AreEqual(0, test_get_dataref_value(dataref_str.c_str()));
		}

		TEST_METHOD_CLEANUP(TestMultiPanelCleanup)
		{
			device->stop(0);
			t->join();
		}
	};
}
