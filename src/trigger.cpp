#include <limits>
#include "trigger.h"
#include "logger.h"

Trigger::Trigger()
{
	data_ref = NULL;
	data_ref_type = xplmType_Unknown;
	trigger_value = 0;
	trigger_action = TriggerType::NO_CHANGE;
	stored_action = TriggerType::NO_CHANGE;
	last_dataref_value = -1;
}

Trigger::Trigger(XPLMDataRef data, int val, TriggerType _trig_action)
{
	data_ref = data;
	data_ref_type = XPLMGetDataRefTypes(data_ref);
	trigger_value = val;
	trigger_action = _trig_action;
	stored_action = TriggerType::NO_CHANGE;
	last_dataref_value = -1;
}

/* call this fumction only from an XPlane flight loop !*/
void Trigger::evaluate_and_store_action()
{
	double act_value;
	if (data_ref_type == xplmType_Int)
		act_value = (double)XPLMGetDatai(data_ref);
	else if (data_ref_type == xplmType_Float)
		act_value = (double)XPLMGetDataf(data_ref);
	else
		act_value = XPLMGetDatad(data_ref);

	guard.lock();
	if (abs(act_value - trigger_value) <= 0.001 && abs(last_dataref_value - act_value) >= 0.01)
	{
		stored_action = trigger_action;
	}
	guard.unlock();

	last_dataref_value = act_value;
}

TriggerType Trigger::get_and_clear_stored_action()
{
	guard.lock();
	TriggerType _stored = stored_action;
	stored_action = TriggerType::NO_CHANGE;
	guard.unlock();
	return _stored;
}