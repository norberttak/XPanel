/*
 * Copyright 2022 Norbert Takacs
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "Action.h"
#include "lua_helper.h"
#include "XPLMUtilities.h"
#include "XPLMProcessing.h"
#include "logger.h"

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
	dataref_type = XPLMGetDataRefTypes(dataref);
}

Action::Action(XPLMDataRef dat, float d)
{
	dataref = dat;
	data_f = d;
	index = -1; // dataref is not an array
	dataref_type = XPLMGetDataRefTypes(dataref);
}

Action::Action(XPLMDataRef dat, int array_index, int d)
{
	dataref = dat;
	data = d;
	index = array_index;
	dataref_type = XPLMGetDataRefTypes(dataref);
}

Action::Action(XPLMDataRef dat, int array_index, float d)
{
	dataref = dat;
	data_f = d;
	index = array_index;
	dataref_type = XPLMGetDataRefTypes(dataref);
}

Action::Action(XPLMDataRef dat, float _delta, float _max, float _min)
{
	dataref = dat;
	delta = _delta;
	max = _max;
	min = _min;
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

void Action::set_dynamic_speed_params(float _time_low, int _multi_low, float _time_high, int _multi_high)
{
	time_low = _time_low;
	time_high = _time_high;
	multi_low = _multi_low;
	multi_high = _multi_high;
}

void Action::get_dynamic_speed_params(float* _time_low, int* _multi_low, float* _time_high, int* _multi_high)
{
	*_time_low = time_low;
	*_time_high = time_high;
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
		// set array type dataref
		if (index != -1)
		{
			if (dataref_type == xplmType_IntArray)
				XPLMSetDatavi(dataref, &data, index, 1);
			else if (dataref_type == xplmType_FloatArray)
				XPLMSetDatavf(dataref, &data_f, index, 1);
		}
		// inc/dec dataref value
		else if (abs(delta) > 0.0001)
		{
			double val = XPLMGetDataf(dataref);
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
			default:
				break;
			}

			Logger(TLogLevel::logTRACE) << "action: change float value " << val << " by " << delta * multi << std::endl;

			val += delta * multi;
			if (val > max) val = max;
			if (val < min) val = min;

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
			default:
				Logger(TLogLevel::logWARNING) << "Dataref change. Invalid dataref type" << std::endl;
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
		}
	}
	//do a lua script
	if (!lua_str.empty())
	{
		Logger(TLogLevel::logTRACE) << "activate lua action: " << lua_str << std::endl;
		if (LuaHelper::get_instace()->do_string(lua_str) == EXIT_FAILURE)
			Logger(TLogLevel::logERROR) << "Error while execute lua string: " << lua_str << std::endl;
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
	uint64_t current_time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

	uint64_t time_diff = current_time_ms - action_happend_last_time[action->get_hash()];

	float time_low, time_high;
	int multi_low, multi_high;
	action->get_dynamic_speed_params(&time_low, &multi_low, &time_high, &multi_high);

	uint64_t time_low_64 = (uint64_t)(time_low * 1000);
	uint64_t time_high_64 = (uint64_t)(time_high * 1000);

	if (time_diff < time_high_64)
		action->speed_up(multi_high);
	else if (time_diff < time_low_64 && time_diff >= time_high_64)
		action->speed_up(multi_low);
	else
		action->speed_up(1);

	action_happend_last_time[action->get_hash()] = current_time_ms;

	guard.lock();
	action_queue.push_back(action);
	guard.unlock();
}

void ActionQueue::activate_actions_in_queue()
{
	guard.lock();

	while (!action_queue.empty())
	{
		action_queue.front()->activate();
		action_queue.pop_front();
	}

	guard.unlock();
}

void ActionQueue::clear_all_actions()
{
	guard.lock();
	action_queue.clear();
	guard.unlock();
}
