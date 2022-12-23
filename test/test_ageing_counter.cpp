/*
 * Copyright 2022 Norbert Takacs
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include <iostream>
#include <thread>
#include <utility>
#include <cstdlib>
#include <chrono>

#include "CppUnitTest.h"
#include "AgeingCounter.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace std::chrono_literals;


namespace test
{
	TEST_CLASS(test_ageing_counter)
	{
	private:
		AgeingCounter ageing_counter;
	public:
		TEST_METHOD_INITIALIZE(TestConfigParserInit)
		{
			//
		}

		TEST_METHOD(TestSingleEvent_wait0ms)
		{
			ageing_counter.event_happend(1);
			//wait 0ms
			Assert::AreEqual(1, ageing_counter.get_nr_of_events_not_older_than(10));
		}

		TEST_METHOD(TestSingleEvent_wait15ms)
		{
			ageing_counter.event_happend(1);
			std::this_thread::sleep_for(15ms);
			//due to 15ms delay the event not counted
			Assert::AreEqual(0, ageing_counter.get_nr_of_events_not_older_than(10));
		}

		TEST_METHOD(TestDoubleEvent_wait0ms)
		{
			ageing_counter.event_happend(2);
			//wait 0ms
			Assert::AreEqual(2, ageing_counter.get_nr_of_events_not_older_than(10));
		}

		TEST_METHOD(TestMultiEvent)
		{
			ageing_counter.event_happend(6);// this event shouldn't appear in query
			std::this_thread::sleep_for(20ms);
			ageing_counter.event_happend(1); 

			Assert::AreEqual(1, ageing_counter.get_nr_of_events_not_older_than(15));
		}

		TEST_METHOD(TestDoubleEvent_clear)
		{
			ageing_counter.event_happend(2);
			std::this_thread::sleep_for(15ms);
			ageing_counter.clear_old_events(10);

			Assert::AreEqual(0, ageing_counter.get_nr_of_events_not_older_than(10));
		}
	};
}
