/*
 * Copyright 2022 Norbert Takacs
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include <list>
#include <atomic>
#include <mutex>

struct AgeingEvent
{
	uint64_t time_of_event;
	int	number_of_event;
};

class AgeingCounter
{
public:
	AgeingCounter();
	int get_nr_of_events_not_older_than(uint64_t max_age);
	void event_happend(int nr_of_event);
	void clear_old_events(uint64_t max_age);
private:
	uint64_t get_current_time();
	std::list<AgeingEvent> list;
	std::mutex guard;
};