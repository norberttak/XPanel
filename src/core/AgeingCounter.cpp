/*
 * Copyright 2022 Norbert Takacs
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "AgeingCounter.h"
#include "Logger.h"

AgeingCounter::AgeingCounter()
{
	//
}

uint64_t AgeingCounter::get_current_time()
{
	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

void AgeingCounter::event_happend(int nr_of_event)
{
	AgeingEvent event;
	event.time_of_event = get_current_time();
	event.number_of_event = nr_of_event;

	guard.lock();
	list.push_back(event);
	guard.unlock();
}

int AgeingCounter::get_nr_of_events_not_older_than(uint64_t max_age)
{
	int count = 0;
	uint64_t current_time = get_current_time();

	guard.lock();
	for (auto& ev : list)
	{
		if (ev.time_of_event + max_age >= current_time) {
			count += ev.number_of_event;
		}
	}
	guard.unlock();

	return count;
}

void AgeingCounter::clear_old_events(uint64_t max_age)
{
	uint64_t current_time = get_current_time();

	guard.lock();
	auto it = list.cbegin();
	while (it != list.cend())
	{
		if (it->time_of_event + max_age < current_time)
			it = list.erase(it);
		else
			++it;
	}
	guard.unlock();
}