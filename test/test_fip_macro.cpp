/*
 * Copyright 2026 Norbert Takacs
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <string>
#include "XPLMDefs.h"
#include "XPLMPlanes.h"
#include "core/ConfigParser.h"
#include "fip/FIPScreen.h"

#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

void test_set_aircraft_path_and_filename(char* file_name, char* path);

namespace test
{
	// The FIP macro feature expands `@use` + `[@page]`/`@parameter` bindings into ordinary
	// [page]/[layer] sections. A successful parse already proves the macro file was found and
	// its bmp assets resolved against the macro folder (add_layer_to_page fails the parse
	// otherwise); the assertions below check the expanded pages/layers landed as expected.
	TEST_CLASS(test_fip_macro)
	{
	public:
		TEST_METHOD(TestMacroExpansion)
		{
			test_set_aircraft_path_and_filename(const_cast<char*>("generic.acf"), const_cast<char*>("./"));

			Configuration config;
			// plugin_path left default "" -> macros resolve under the test working dir
			// (build/test), where CMake copies test/macros/. Mirrors how the existing FIP
			// test resolves fip-fonts.bmp relative to the working dir.

			ConfigParser parser;
			int result = parser.parse_file("../../test/test-fip-macro-config.ini", config);
			Assert::AreEqual(0, result);

			Assert::AreEqual(1, (int)config.class_configs.size());
			FIPScreen* screen = config.class_configs[0].fip_screens["saitek_fip_screen"];
			Assert::IsTrue(screen != nullptr);

			// the macro defined two pages, in order: HSI (index 0) and STATIC (index 1)
			Assert::AreEqual(1, screen->get_last_page_index());
			Assert::AreEqual(std::string("HSI"), screen->get_page_name(0));
			Assert::AreEqual(std::string("STATIC"), screen->get_page_name(1));

			// HSI page: one image layer + one text layer -> last index 1
			Assert::AreEqual(1, screen->get_last_layer_index(0));
			// STATIC page: one image layer -> last index 0
			Assert::AreEqual(0, screen->get_last_layer_index(1));
		}

		TEST_METHOD(TestMacroUnboundParameterFails)
		{
			test_set_aircraft_path_and_filename(const_cast<char*>("generic.acf"), const_cast<char*>("./"));

			Configuration config;

			ConfigParser parser;
			// the 'baro' parameter of the HSI page is left unbound -> parsing must fail
			int result = parser.parse_file("../../test/test-fip-macro-unbound-config.ini", config);
			Assert::AreEqual(1, result);
		}
	};
}
