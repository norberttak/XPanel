#pragma once
#include <string>
#include <queue>
#include <atomic>
#include <mutex>
#include "XPLMDataAccess.h"
#include "XPLMMenus.h"

typedef enum {
	NO_CHANGE,
	LIT,
	UNLIT,
	BLINK
} TriggerType;

class Trigger
{
public:
	Trigger();
	Trigger(XPLMDataRef data, double val, TriggerType _trig_action);
	Trigger(std::string _lua_str, double val, TriggerType _trigger_action);
	TriggerType get_and_clear_stored_action();
	void evaluate_and_store_action();
private:
	XPLMDataRef data_ref;
	XPLMDataTypeID data_ref_type;
	std::string lua_str;
	double trigger_value;
	double last_value;
	TriggerType trigger_action;
	TriggerType stored_action;
	std::mutex guard;
};