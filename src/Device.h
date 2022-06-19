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
	Configuration config;
public:
	std::thread* thread_handle = NULL;
	Device(Configuration &_config);
	virtual void stop(int time_out)=0;
	virtual void thread_func(void)=0;
	virtual int connect() = 0;
	virtual void release() = 0;
	virtual void start() = 0;
};