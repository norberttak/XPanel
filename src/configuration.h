#pragma once
#include <string>
#include <vector>
#include <map>
#include "Action.h"

typedef enum {
	UNKNOWN_DEVICE_TYPE,
	SAITEK_MULTI,
	SAITEK_RADIO,
	SAITEK_SWITCH,
	HOME_COCKPIT
} DeviceType;

class Configuration
{
public:
	~Configuration();
	Configuration& operator=(const Configuration& other);
	DeviceType device_type = UNKNOWN_DEVICE_TYPE;
	unsigned int vid = 0;
	unsigned int pid = 0;
	std::string version = "";
	std::string aircraft_acf = "";
	std::string script_file = "";
	std::map<std::string, Action> push_actions;
	std::map<std::string, Action> release_actions;
};

