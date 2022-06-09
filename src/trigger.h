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
	Trigger(XPLMDataRef data, int val, TriggerType _trig_action);
	TriggerType get_and_clear_stored_action();
	void evaluate_and_store_action();
private:
	XPLMDataRef data_ref;
	XPLMDataTypeID data_ref_type;
	double trigger_value;
	int last_dataref_value;
	TriggerType trigger_action;
	TriggerType stored_action;	
	std::mutex guard;
};