#pragma once
#include <string>
#include <vector>
#include <list>
#include <map>
#include "Action.h"
#include "trigger.h"
#include "multi_purpose_display.h"

class DeviceConfiguration;

/*------ Plugin level configuration options ------------*/
class Configuration
{
public:
	~Configuration();
	Configuration& operator=(const Configuration& other);
	void clear();

	std::string version = "";
	std::string aircraft_acf = "";
	std::string script_file = "";

	std::vector<DeviceConfiguration> device_configs;
};

/* ----------- Device specific configuration options ------------------*/
typedef enum {
	UNKNOWN_DEVICE_TYPE,
	SAITEK_MULTI,
	SAITEK_RADIO,
	SAITEK_SWITCH,
	HOME_COCKPIT
} DeviceType;

class DeviceConfiguration
{
public:
	~DeviceConfiguration();
	DeviceConfiguration& operator=(const DeviceConfiguration& other);
	DeviceType device_type = UNKNOWN_DEVICE_TYPE;
	unsigned int vid = 0;
	unsigned int pid = 0;
	std::map<std::string, std::list<Action*>> push_actions;
	std::map<std::string, std::list<Action*>> release_actions;
	std::map<std::string, std::list<Trigger*>> light_triggers;
	std::map<std::string, MultiPurposeDisplay*> multi_displays;
};

