#pragma once
#include <string>
#include <map>
#include <list>
#include <thread>
#include "Action.h"
#include "configuration.h"
#include <thread>
#include <chrono>
using namespace std::literals;

class Device
{
protected:
	Configuration config;
public:
	Device(Configuration &_config);
	virtual void stop(int time_out)=0;
	virtual void thread_func(void)=0;
};