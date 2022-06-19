#include "Action.h"
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

Action::Action(std::string lua_str)
{
	lua_str = lua_str;
}

Action::~Action()
{
	if (dataref != NULL)
		XPLMUnregisterDataAccessor(dataref);
	dataref = NULL;
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
			val += delta;
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
		//
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
	action_queue.push(action);
	guard.unlock();
}

void ActionQueue::activate_actions_in_queue()
{
	guard.lock();

	while (!action_queue.empty())
	{
		action_queue.front()->activate();
		action_queue.pop();		
	}

	guard.unlock();
}

void ActionQueue::clear_all_actions()
{
	guard.lock();
	while (action_queue.size())
		action_queue.pop();
	guard.unlock();
}