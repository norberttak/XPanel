#include "Action.h"
#include "XPLMUtilities.h"
#include "XPLMProcessing.h"

Action::Action()
{
	data = 0;
	lua_str = "";
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

void Action::activate()
{
	if (dataref != NULL)
	{
		XPLMSetDatai(dataref, data);
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