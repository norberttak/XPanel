#include "XPLMDefs.h"
#include "XPLMGraphics.h"
#include "XPLMPlugin.h"
#include "XPLMPlanes.h"
#include "XPLMDisplay.h"

#include "CppUnitTest.h"
#include "SaitekRadioPanel.h"
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
void test_hid_mock_init();

namespace test
{
	TEST_CLASS(test_radio_panel)
	{
	private:
		Configuration config;
		Configparser* p;
		SaitekRadioPanel* device;
		std::thread* t;
		std::string nav1_freq_dataref_str = "sim/cockpit2/radios/actuators/nav1_frequency_hz";
		std::string nav2_freq_dataref_str = "sim/cockpit2/radios/actuators/nav2_frequency_hz";
		std::string com1_freq_dataref_str = "sim/cockpit2/radios/actuators/com1_frequency_hz";
		std::string com2_freq_dataref_str = "sim/cockpit/radios/com1_stdby_freq_hz";

		XPLMDataRef nav1_freq_dataref;
		XPLMDataRef nav2_freq_dataref;
		XPLMDataRef com1_freq_dataref;
		XPLMDataRef com2_freq_dataref;

	public:
		TEST_METHOD_INITIALIZE(TestMultiPanelInit)
		{
			test_hid_mock_init();
			
			p = new Configparser();
			int result = p->parse_file("../../test/test-radio-panel-config.ini", config);
			Assert::AreEqual(0, result);

			LuaHelper::get_instace()->init();
			LuaHelper::get_instace()->load_script_file("../../test/" + config.script_file);

			device = new SaitekRadioPanel(config.device_configs[0]);
			device->connect();
			device->start();
			t = new std::thread(&SaitekRadioPanel::thread_func, (SaitekRadioPanel*)device);

			nav1_freq_dataref = XPLMFindDataRef(nav1_freq_dataref_str.c_str());
			nav2_freq_dataref = XPLMFindDataRef(nav2_freq_dataref_str.c_str());
			com1_freq_dataref = XPLMFindDataRef(com1_freq_dataref_str.c_str());
			com2_freq_dataref = XPLMFindDataRef(com2_freq_dataref_str.c_str());

			XPLMSetDatai(nav1_freq_dataref, 11111);
			XPLMSetDatai(nav2_freq_dataref, 22222);
			XPLMSetDatai(com1_freq_dataref, 33333);
			XPLMSetDatai(com2_freq_dataref, 44444);
		}

		TEST_METHOD(Test_VID_PID)
		{
			Assert::AreEqual(0x6A3, test_hid_get_vid());
			Assert::AreEqual(0xd05, test_hid_get_pid());
		}

		TEST_METHOD(Test_Display)
		{
			// set rotation switch to SW_NAV_1 position
			unsigned char buffer[4] = { 0x04,0,0,0 };
			test_hid_set_read_data(buffer, sizeof(buffer));
			std::this_thread::sleep_for(150ms);

			test_flight_loop(config.device_configs);
			std::this_thread::sleep_for(150ms);

			unsigned char write_buffer[23];
			test_hid_get_write_data(write_buffer, sizeof(write_buffer));
			Assert::AreEqual(1, (int)write_buffer[1]);
			Assert::AreEqual(1, (int)write_buffer[2]);
			Assert::AreEqual(1, (int)write_buffer[3]);
			Assert::AreEqual(1, (int)write_buffer[4]);
			Assert::AreEqual(1, (int)write_buffer[5]);
		}

		TEST_METHOD(Test_KNOB_UP_BIG_PLUS)
		{
			// set rotation switch to SW_NAV_1 position
			unsigned char buffer[4] = { 0x04,0,0,0 };
			test_hid_set_read_data(buffer, sizeof(buffer));
			std::this_thread::sleep_for(150ms);

			test_flight_loop(config.device_configs);
			std::this_thread::sleep_for(150ms);

			buffer[2] |= 0x04; // set KNOB_UP_BIG_PLUS to 1
			test_hid_set_read_data(buffer, sizeof(buffer));
			std::this_thread::sleep_for(150ms);
			test_flight_loop(config.device_configs);
			Assert::AreEqual(11111 + 100, test_get_dataref_value(nav1_freq_dataref_str.c_str())); // nav1 freq shal be increased by +100
			Assert::AreEqual(22222, test_get_dataref_value(nav2_freq_dataref_str.c_str())); // nav2 freq shall not change
			Assert::AreEqual(33333, test_get_dataref_value(com1_freq_dataref_str.c_str())); // com1 freq shall not change
			Assert::AreEqual(44444, test_get_dataref_value(com2_freq_dataref_str.c_str())); // com2 freq shall not change

			buffer[2] &= (~0x04); // set KNOB_UP_BIG_PLUS to 0
			test_hid_set_read_data(buffer, sizeof(buffer));
			std::this_thread::sleep_for(150ms);
			test_flight_loop(config.device_configs);
			Assert::AreEqual(11111 + 100, test_get_dataref_value(nav1_freq_dataref_str.c_str()));
		}

		TEST_METHOD_CLEANUP(TestMultiPanelCleanup)
		{
			device->stop(0);
			t->join();
		}
	};
}
