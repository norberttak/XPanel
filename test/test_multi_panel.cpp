#include "XPLMDefs.h"
#include "XPLMGraphics.h"
#include "XPLMPlugin.h"
#include "XPLMPlanes.h"
#include "XPLMDisplay.h"

#include "CppUnitTest.h"
#include "SaitekMultiPanel.h"
#include "configparser.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

void test_hid_set_read_data(unsigned char* data, size_t length);
int test_get_dataref_value(const char* datarefstr);
int test_hid_get_vid();
int test_hid_get_pid();
std::string test_get_last_command();
void test_hid_get_write_data(unsigned char* data, size_t length);
void test_flight_loop(std::vector<Configuration> config);

namespace test
{
	TEST_CLASS(test_multi_panel)
	{
	private:
		std::vector<Configuration> config;
		Configparser* p;
		SaitekMultiPanel* device;
		std::thread* t;
	public:
		TEST_METHOD_INITIALIZE(TestMultiPanelInit)
		{
			p = new Configparser();
			int result = p->parse_file("../../test/test-valid-config.ini", config);
			Assert::AreEqual(0, result);
			device = new SaitekMultiPanel(config[0]);
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
			test_flight_loop(config);
			Assert::AreEqual(1, test_get_dataref_value("/sim/hello/AP"));

			buffer[0] = 0;
			test_hid_set_read_data(buffer, sizeof(buffer));
			std::this_thread::sleep_for(150ms);
			test_flight_loop(config);
			Assert::AreEqual(0, test_get_dataref_value("/sim/hello/AP"));
		}

		//Test a button with command begin-end action
		TEST_METHOD(TestNAVButton)
		{
			unsigned char buffer[4] = { 0,0x02,0,0 };
			test_hid_set_read_data(buffer, sizeof(buffer));
			std::this_thread::sleep_for(150ms);
			test_flight_loop(config);
			std::string last_cmd = test_get_last_command();
			Assert::AreEqual("/sim/cmd/NAV_BEGIN", last_cmd.c_str());

			memset(buffer, 0, sizeof(buffer));
			test_hid_set_read_data(buffer, sizeof(buffer));
			std::this_thread::sleep_for(150ms);
			test_flight_loop(config);
			last_cmd = test_get_last_command();
			Assert::AreEqual("/sim/cmd/NAV_END", last_cmd.c_str());
		}

		TEST_METHOD(TestTrimWheelUp)
		{
			int val_before = test_get_dataref_value("sim/custom/switchers/console/absu_pitch_wheel");
			unsigned char buffer[4] = { 0,0,0x08,0};
			test_hid_set_read_data(buffer, sizeof(buffer));
			std::this_thread::sleep_for(150ms);
			test_flight_loop(config);
			int val_after = test_get_dataref_value("sim/custom/switchers/console/absu_pitch_wheel");

			Assert::IsTrue((val_after - val_before) == 1);

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
			test_flight_loop(config);
			std::string last_cmd = test_get_last_command();
			Assert::AreNotEqual("/sim/cmd/HDG_ONCE", last_cmd.c_str());

			memset(buffer, 0, sizeof(buffer));
			test_hid_set_read_data(buffer, sizeof(buffer));
			std::this_thread::sleep_for(150ms);
			test_flight_loop(config);
			last_cmd = test_get_last_command();
			Assert::AreEqual("/sim/cmd/HDG_ONCE", last_cmd.c_str());
		}

		TEST_METHOD(TestAltButtonLight)
		{
			XPLMDataRef dataref = XPLMFindDataRef("sim/custom/lights/button/absu_stab_h");
			// ALT button light set to LIT
			XPLMSetDatai(dataref, 1);
			test_flight_loop(config); // evaluate triggers
			std::this_thread::sleep_for(150ms);
			unsigned char write_buffer[13];
			test_hid_get_write_data(write_buffer, sizeof(write_buffer));
			Assert::AreEqual((int)write_buffer[11], 0x10);

			// ALT button light set to UNLIT
			XPLMSetDatai(dataref, 0);
			test_flight_loop(config); // evaluate triggers
			std::this_thread::sleep_for(150ms);
			test_hid_get_write_data(write_buffer, sizeof(write_buffer));
			Assert::AreEqual((int)write_buffer[11], 0x00);
		}

		TEST_METHOD(TestHdgButtonLight)
		{
			XPLMDataRef dataref = XPLMFindDataRef("sim/custom/lights/button/absu_zk");
			// HDG button light set to LIT
			XPLMSetDatai(dataref, 1);
			test_flight_loop(config); // evaluate triggers
			std::this_thread::sleep_for(150ms);
			unsigned char write_buffer[13];
			test_hid_get_write_data(write_buffer, sizeof(write_buffer));
			Assert::AreEqual((int)write_buffer[11], 0x02);

			// HDG button light set to UNLIT
			XPLMSetDatai(dataref, 0);
			test_flight_loop(config); // evaluate triggers
			std::this_thread::sleep_for(150ms);
			test_hid_get_write_data(write_buffer, sizeof(write_buffer));
			Assert::AreEqual((int)write_buffer[11] ,0x00);
		}

		TEST_METHOD_CLEANUP(TestMultiPanelCleanup)
		{
			device->stop(0);
			t->join();
		}
	};
}
