#include "XPLMDefs.h"
#include "XPLMGraphics.h"
#include "XPLMPlugin.h"
#include "XPLMPlanes.h"
#include "XPLMDisplay.h"

#include "CppUnitTest.h"
#include "configparser.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;


namespace test
{
	TEST_CLASS(test_config)
	{
	private:
		Configuration config;
		Configparser* p;
	public:
		TEST_METHOD_INITIALIZE(TestConfigParserInit)
		{
			p = new Configparser();
			int result = p->parse_file("../../test/test-valid-config.ini", config);
			Assert::AreEqual(0, result);
		}

		TEST_METHOD(TestDeviceConfigParams)
		{
			Assert::AreEqual(1, (int)config.device_configs.size());
			Assert::IsTrue(config.device_configs[0].device_type == DeviceType::SAITEK_MULTI);
			Assert::AreEqual(0x12AB, (int)config.device_configs[0].vid);
			Assert::AreEqual(0x34CD, (int)config.device_configs[0].pid);
			Assert::AreEqual("test-script.lua", config.script_file.c_str());
			Assert::AreEqual("generic.acf", config.aircraft_acf.c_str());
		}

		TEST_METHOD(TestButtonActions)
		{
			Assert::AreEqual(3, (int)config.device_configs[0].push_actions["AP"].size());
			Assert::AreEqual(3, (int)config.device_configs[0].release_actions["AP"].size());

			Assert::AreEqual(1, (int)config.device_configs[0].push_actions["NAV"].size());
			Assert::AreEqual(1, (int)config.device_configs[0].release_actions["NAV"].size());

			Assert::AreEqual(0, (int)config.device_configs[0].push_actions.count("HDG"));
			Assert::AreEqual(1, (int)config.device_configs[0].release_actions["HDG"].size());
		}

		TEST_METHOD(TestDatarefArray)
		{
			Assert::AreEqual(1, (int)config.device_configs[0].push_actions["TEST"].size());
			Assert::AreEqual(0, (int)config.device_configs[0].release_actions.count("TEST"));
		}	
	};
}
