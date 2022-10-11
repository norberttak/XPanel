/*
 * Copyright 2022 Norbert Takacs
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once
#include <string>
#include <map>
#include <list>
#include <thread>
#include "Action.h"
#include "configuration.h"
#include <queue>
#include "Action.h"
#include <thread>
#include <chrono>
using namespace std::literals;

class Device
{
protected:
	DeviceConfiguration config;
public:
	std::thread* thread_handle = NULL;
	Device(DeviceConfiguration &_config);
	virtual void stop(int time_out)=0;
	virtual void thread_func(void)=0;
	virtual int connect() = 0;
	virtual void release() = 0;
	virtual void start() = 0;
};
