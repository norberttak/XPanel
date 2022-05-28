#include "pch.h"
#include "CppUnitTest.h"
#include "..\src\configparser.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;


namespace test
{
	TEST_CLASS(test_config)
	{
	private:
		std::vector<Configuration> config;
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
			Assert::AreEqual(1, (int)config.size());
			Assert::IsTrue(config[0].device_type == DeviceType::SAITEK_MULTI);
			Assert::AreEqual(0x12AB, (int)config[0].vid);
			Assert::AreEqual(0x34CD, (int)config[0].pid);
			Assert::AreEqual("tu154-saitket-multipanel.lua", config[0].script_file.c_str());
			Assert::AreEqual("generic.acf", config[0].aircraft_acf.c_str());
		}

		TEST_METHOD(TestButtonActions)
		{
			Assert::AreEqual(1, (int)config[0].push_actions.count("AP"));
			Assert::AreEqual(1, (int)config[0].release_actions.count("AP"));

			Assert::AreEqual(1, (int)config[0].push_actions.count("NAV"));
			Assert::AreEqual(1, (int)config[0].release_actions.count("NAV"));

			Assert::AreEqual(0, (int)config[0].push_actions.count("HDG"));
			Assert::AreEqual(1, (int)config[0].release_actions.count("HDG"));
		}
	};
}
