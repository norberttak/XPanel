/*
 * Copyright 2022 Norbert Takacs
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "Action.h"
#include "LuaHelper.h"
#include "XPLMUtilities.h"
#include "XPLMProcessing.h"
#include "Logger.h"

Action::Action()
{
	data = 0;
	delta = 0;
	lua_str = "";
	condition = "";
	active_conditions.clear();
	dataref = NULL;
	dataref_type = xplmType_Unknown;
}

Action::Action(XPLMCommandRef cmd, CommandType type)
{
	commandref = cmd;
	command_type = type;
	dataref_type = xplmType_Unknown;
}

Action::Action(XPLMDataRef dat, int d)
{
	dataref = dat;
	data = d;
	index = -1; // dataref is not an array
	dataref_type = xplmType_Int;
}

Action::Action(XPLMDataRef dat, float d)
{
	dataref = dat;
	data_f = d;
	index = -1; // dataref is not an array
	dataref_type = xplmType_Float;
}

Action::Action(XPLMDataRef dat, double d)
{
	dataref = dat;
	data_f = (float)d;
	index = -1; // dataref is not an array
	dataref_type = xplmType_Double;
}

Action::Action(XPLMDataRef dat, int array_index, int d)
{
	dataref = dat;
	data = d;
	index = array_index;
	dataref_type = xplmType_IntArray;
}

Action::Action(XPLMDataRef dat, int array_index, float d)
{
	dataref = dat;
	data_f = d;
	index = array_index;
	dataref_type = xplmType_FloatArray;
}

Action::Action(XPLMDataRef dat, float _delta, float _max, float _min)
{
	dataref = dat;
	delta = _delta;
	max = _max;
	min = _min;
	if (min > max)
		Logger(TLogLevel::logWARNING) << "Action: defined min limit is greater than max " << _min << " > " << _max << std::endl;

	dataref_type = XPLMGetDataRefTypes(dataref);
}

Action::Action(std::string _lua_str)
{
	lua_str = _lua_str;
}

Action::~Action()
{
	if (dataref != NULL)
		XPLMUnregisterDataAccessor(dataref);
	dataref = NULL;
}

void Action::speed_up(int _multi)
{
	multi = _multi;
}

int Action::get_speed_multi()
{
	return multi;
}

void Action::add_condition(std::string _condition)
{
	condition = _condition;
	Logger(TLogLevel::logDEBUG) << "Action: add condition: " << _condition << std::endl;
}

void Action::set_condition_active(std::string _active_condition)
{
	active_conditions[_active_condition] = true;
	Logger(TLogLevel::logDEBUG) << "Action: set_condition_active " << _active_condition << std::endl;
}

void Action::set_condition_inactive(std::string _active_condition)
{
	active_conditions[_active_condition] = false;
	Logger(TLogLevel::logDEBUG) << "Action: set_condition_inactive " << _active_condition << std::endl;
}

int Action::get_hash()
{
	std::size_t h0 = std::hash<int>{}(data);
	std::size_t h1 = std::hash<float>{}(delta);
	std::size_t h2 = std::hash<std::string>{}(condition);
	std::size_t h3 = std::hash<std::string>{}(lua_str);
	std::size_t h4 = std::hash<void*>{}(dataref);
	std::size_t h5 = std::hash<void*>{}(commandref);

	return h0 ^ (h1 << 1) ^ (h2 << 2) ^ (h3 << 3) ^ (h4 << 4) ^ (h5 << 5);
}

void Action::set_dynamic_speed_params(float _tick_per_sec_mid, int _multi_low, float _tick_per_sec_high, int _multi_high)
{
	tick_per_sec_mid = _tick_per_sec_mid;
	tick_per_sec_high = _tick_per_sec_high;
	multi_low = _multi_low;
	multi_high = _multi_high;
}

void Action::get_dynamic_speed_params(float* _tick_per_sec_mid, int* _multi_low, float* _tick_per_sec_high, int* _multi_high)
{
	*_tick_per_sec_mid = tick_per_sec_mid;
	*_tick_per_sec_high = tick_per_sec_high;
	*_multi_low = multi_low;
	*_multi_high = multi_high;
}

void Action::activate()
{
	if (condition != "" && (active_conditions.count(condition) == 0 || active_conditions[condition] == false) )
	{
		Logger(TLogLevel::logTRACE) << "Action: skip because condition (" << condition << ") is not active" << std::endl;
		return;
	}

	if (dataref != NULL)
	{
		if (abs(delta) > 0.0001)
		{
			double val;

			switch (dataref_type) {
			case xplmType_Int:
				val = (double)XPLMGetDatai(dataref);
				break;
			case xplmType_Double:
				val = XPLMGetDatad(dataref);
				break;
			case xplmType_Float:
				val = (double)XPLMGetDataf(dataref);
				break;
			case xplmType_IntArray:
				XPLMGetDatavi(dataref, &data, index, 1);
				val = (double)data;
				break;
			case xplmType_FloatArray:
				XPLMGetDatavf(dataref, &data_f, index, 1);
				val = (double)data_f;
				break;
			default:
				Logger(TLogLevel::logWARNING) << "action: unknown dataref type. set value = 0" << std::endl;
				break;
			}

			Logger(TLogLevel::logTRACE) << "action: change float value " << val << " by " << delta << std::endl;

			val += delta;
			// when reach the max or min we need to wrap from the other side:
			if (val > max) val = min;
			if (val < min) val = max;

			Logger(TLogLevel::logTRACE) << "action: new value:" << val << " max:" << max << " min:" << min << std::endl;

			switch (dataref_type) {
			case xplmType_Int:
				XPLMSetDatai(dataref, (int)val);
				break;
			case xplmType_Double:
				XPLMSetDatad(dataref, val);
				break;
			case xplmType_Float:
				XPLMSetDataf(dataref, (float)val);
				break;
			case xplmType_IntArray:
				data = (int)val;
				XPLMSetDatavi(dataref, &data, index, 1);
				break;
			case xplmType_FloatArray:
				data_f = (float)val;
				XPLMSetDatavf(dataref, &data_f, index, 1);
				break;
			default:
				Logger(TLogLevel::logWARNING) << "Dataref change set. Invalid dataref type" << std::endl;
			}
		}
		// set dataref to a value
		else
		{
			switch (dataref_type) {
			case xplmType_Int:
				XPLMSetDatai(dataref, data);
				break;
			case xplmType_Double:
				XPLMSetDatad(dataref, (double)data_f);
				break;
			case xplmType_Float:
				XPLMSetDataf(dataref, data_f);
				break;
			case xplmType_IntArray:
				XPLMSetDatavi(dataref, &data, index, 1);
				break;
			case xplmType_FloatArray:
				XPLMSetDatavf(dataref, &data_f, index, 1);
				break;
			default:
				Logger(TLogLevel::logWARNING) << "Dataref set. Invalid dataref type" << std::endl;
			}
		}
	}
	//execute xplane command
	if (commandref != NULL)
	{
		switch (command_type) {
		case CommandType::BEGIN:
			XPLMCommandBegin(commandref);
			break;

		case CommandType::END:
			XPLMCommandEnd(commandref);
			break;

		case CommandType::ONCE:
			XPLMCommandOnce(commandref);
			break;

		default:
			break;
		}
	}
	//do a lua script
	if (!lua_str.empty())
	{
		Logger(TLogLevel::logTRACE) << "activate lua action: " << lua_str << std::endl;;
		LuaHelper::get_instace()->do_string(lua_str);
	}
}

// ---------------------- ActionQueue ------------------
// ---------- a thread safe queue implementation -------
ActionQueue* ActionQueue::current = NULL;

ActionQueue::ActionQueue()
{
	//
}

ActionQueue* ActionQueue::get_instance()
{
	if (current == NULL)
		current = new ActionQueue();

	return current;
}

void ActionQueue::push(Action* action)
{
	float tick_per_sec_mid, tick_per_sec_high;
	int multi_mid, multi_high;
	action->get_dynamic_speed_params(&tick_per_sec_mid, &multi_mid, &tick_per_sec_high, &multi_high);
	
	guard.lock();
	if (tick_per_sec_mid != 0 || tick_per_sec_high != 0)
	{		
		if (action_ageing_counters.count(action->get_hash()) == 0) {
			action_ageing_counters[action->get_hash()] = new AgeingCounter();
		}
		AgeingCounter* counter = action_ageing_counters[action->get_hash()];
		counter->event_happend(1);

		const uint64_t time_base_in_ms = 500;

		int count = counter->get_nr_of_events_not_older_than(time_base_in_ms);

		if (count > tick_per_sec_high / 2)
			action->speed_up(multi_high);
		else if (count > tick_per_sec_mid / 2)
			action->speed_up(multi_mid);
		else
			action->speed_up(1);

		counter->clear_old_events(2 * time_base_in_ms);
	}

	action_queue.push_back(action);
	guard.unlock();
}

void ActionQueue::activate_actions_in_queue()
{
	guard.lock();

	while (!action_queue.empty())
	{
		int speed_multi = action_queue.front()->get_speed_multi();

		for (int i = 0; i < speed_multi; i++) 
			action_queue.front()->activate();

		action_queue.pop_front();
	}

	guard.unlock();
}

void ActionQueue::clear_all_actions()
{
	guard.lock();
	action_queue.clear();
	for (auto it = action_ageing_counters.begin(); it != action_ageing_counters.end(); ++it)
	{
		delete(it->second);
	}
	action_ageing_counters.clear();
	guard.unlock();
}
