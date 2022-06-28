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
}

Action::Action(XPLMCommandRef cmd, CommandType type)
{
	commandref = cmd;
	command_type = type;
}

Action::Action(XPLMDataRef dat, int d)
{
	dataref = dat;
	data = d;
	index = -1; // dataref is not an array
}

Action::Action(XPLMDataRef dat, int array_index, int d)
{
	dataref = dat;
	data = d;
	index = array_index;
}

Action::Action(XPLMDataRef dat, float _delta, float _max, float _min)
{
	dataref = dat;
	delta = _delta;
	max = _max;
	min = _min;
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

	return h0 ^ (h1 << 1) ^ (h2<<2) ^ (h3<<3) ^ (h4<<4) ^ (h5<<5);
}

void Action::activate()
{
	if (active_conditions.count(condition) != 0 && active_conditions[condition] == false)
	{
		Logger(TLogLevel::logTRACE) << "Action: skip because condition (" << condition << ") is not active" << std::endl;
		return;
	}

	if (dataref != NULL)
	{
		if (index != -1)
		{
			XPLMSetDatavi(dataref, &data, index, 1);
		}
		else if (abs(delta) > 0.0001)
		{
			float val = XPLMGetDataf(dataref);
			Logger(TLogLevel::logTRACE) << "action: change float value " << val << " by " << delta << std::endl;
			val += delta * multi;
			if (val > max)
				val = max;
			if (val < min)
				val = min;
			XPLMSetDataf(dataref, val);
		}
		else
		{
			XPLMSetDatai(dataref, data);
		}
	}

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
	guard.lock();
	action_queue.push_back(action);
	guard.unlock();
}

void ActionQueue::activate_actions_in_queue()
{
	guard.lock();	

	uint64_t current_time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	
	while (!action_queue.empty())
	{
		Action* act = action_queue.front();

		uint64_t time_diff = current_time_ms - action_happend_last_time[act->get_hash()];

		if (time_diff < 0.25)
			act->speed_up(4);
		else if (time_diff < 0.5 && time_diff >= 0.25)
			act->speed_up(2);
		else
			act->speed_up(1);
		
		action_happend_last_time[act->get_hash()] = current_time_ms;

		act->activate();
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