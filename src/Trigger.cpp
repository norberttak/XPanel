/*
 * Copyright 2022 Norbert Takacs
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <limits>
#include "Trigger.h"
#include "LuaHelper.h"
#include "Logger.h"

Trigger::Trigger()
{
	data_ref = NULL;
	data_ref_type = xplmType_Unknown;
	trigger_value = 0;
	trigger_action = TriggerType::NO_CHANGE;
	stored_action = TriggerType::NO_CHANGE;
	last_value = -1;
}

Trigger::Trigger(XPLMDataRef data, double val, TriggerType _trigger_action)
{
	data_ref = data;
	data_ref_type = XPLMGetDataRefTypes(data_ref);
	trigger_value = val;
	trigger_action = _trigger_action;
	stored_action = TriggerType::NO_CHANGE;
	last_value = -1;
}

Trigger::Trigger(std::string _lua_str, double val, TriggerType _trigger_action)
{
	lua_str = _lua_str;
	trigger_value = val;
	trigger_action = _trigger_action;
	stored_action = TriggerType::NO_CHANGE;
	last_value = -1;
}

/* call this function only from an XPlane flight loop !*/
void Trigger::evaluate_and_store_action()
{
	double act_value;
	if (lua_str.empty())
	{
		if (data_ref_type == xplmType_Int)
			act_value = (double)XPLMGetDatai(data_ref);
		else if (data_ref_type == xplmType_Float)
			act_value = (double)XPLMGetDataf(data_ref);
		else
			act_value = XPLMGetDatad(data_ref);
	}
	else
	{
		act_value = LuaHelper::get_instace()->do_string("return " + lua_str);
	}

	guard.lock();

	if (abs(act_value - trigger_value) <= 0.001 && abs(last_value - act_value) >= 0.01)
		stored_action = trigger_action;

	guard.unlock();

	last_value = act_value;
}

TriggerType Trigger::get_and_clear_stored_action()
{
	guard.lock();
	TriggerType _stored = stored_action;
	stored_action = TriggerType::NO_CHANGE;
	guard.unlock();
	return _stored;
}
