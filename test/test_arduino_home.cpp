#include "XPLMDefs.h"
#include "XPLMGraphics.h"
#include "XPLMPlugin.h"
#include "XPLMPlanes.h"
#include "XPLMDisplay.h"
#include "XPLMPlugin.h"

#include "CppUnitTest.h"
#include "ArduinoHomeCockpit.h"
#include "configparser.h"
#include "logger.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

void test_hid_set_read_data(unsigned char* data, size_t length);
int test_get_dataref_value(const char* datarefstr);
int test_hid_get_vid();
int test_hid_get_pid();
std::string test_get_last_command();
int test_get_command_queue_size();

namespace test
{
	TEST_CLASS(test_arduino_home_panel)
	{
	private:
		Configuration config;
		Configparser* p;
		ArduinoHomeCockpit* device;
		std::thread* t;
	public:
		TEST_METHOD_INITIALIZE(TestArduinoHomeCockpitInit)
		{
			if (p == NULL) 
			{
				p = new Configparser();
				int result = p->parse_file("../../test/test-arduino-home.ini", config);
				Assert::AreEqual(0, result);
				device = new ArduinoHomeCockpit(config.device_configs[0]);
				device->connect();
				device->start();
				t = new std::thread(&ArduinoHomeCockpit::thread_func, (ArduinoHomeCockpit*)device);
			}
		}

		TEST_METHOD(Test_VID_PID)
		{
			Assert::AreEqual(0x2341, test_hid_get_vid());
			Assert::AreEqual(0x8036, test_hid_get_pid());
		}

		//Test a button with DataRef action
		TEST_METHOD(TestStrobeLightButton)
		{
			unsigned char buffer[9] = { 0,1,0,0,0,0,0,0,0 };
			test_hid_set_read_data(buffer, sizeof(buffer));
			std::this_thread::sleep_for(150ms);
			ActionQueue::get_instance()->activate_actions_in_queue();
			Assert::AreEqual(-1, test_get_dataref_value("sim/cockpit/electrical/strobe_lights_on"));

			memset(buffer, 0, sizeof(buffer));
			test_hid_set_read_data(buffer, sizeof(buffer));
			std::this_thread::sleep_for(150ms);
			ActionQueue::get_instance()->activate_actions_in_queue();
			Assert::AreEqual(0, test_get_dataref_value("sim/cockpit/electrical/strobe_lights_on"));
		}

		//Test a button with CommandRef action
		TEST_METHOD(TestStarterButton)
		{
			unsigned char buffer[9] = { 0,0x40,0,0,0,0,0,0,0 };
			test_hid_set_read_data(buffer, sizeof(buffer));
			std::this_thread::sleep_for(150ms);
			ActionQueue::get_instance()->activate_actions_in_queue();
			Assert::AreEqual(1, test_get_command_queue_size());			
			Assert::AreEqual(std::string("sim/ignition/engage_starter_1_BEGIN"), test_get_last_command());

			memset(buffer, 0, sizeof(buffer));;
			test_hid_set_read_data(buffer, sizeof(buffer));
			std::this_thread::sleep_for(150ms);
			ActionQueue::get_instance()->activate_actions_in_queue();
			Assert::AreEqual(1, test_get_command_queue_size());			
			Assert::AreEqual(std::string("sim/ignition/engage_starter_1_END"), test_get_last_command());
		}

		TEST_METHOD_CLEANUP(TestArduinoHomeCockpitCleanup)
		{
			device->stop(0);
			t->join();
		}
	};
}