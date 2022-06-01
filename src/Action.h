#pragma once
#include <string>
#include <queue>
#include <atomic>
#include <mutex>
#include "XPLMDataAccess.h"
#include "XPLMMenus.h"

typedef enum 
{
	BEGIN,
	END,
	ONCE,
	NONE
} CommandType;

class Action
{
public:
	Action();
	Action(XPLMDataRef dat, int d);
	Action(XPLMCommandRef cmd, CommandType type);
	Action(std::string lua_str);
	//XPLMCommandPhase command_phase;
	~Action();
	void activate();
private:
	int data = 0;
	std::string lua_str = "";
	XPLMDataRef dataref = NULL;
	CommandType command_type = CommandType::NONE;
	XPLMCommandRef commandref = NULL;
};

class ActionQueue
{
private:
	std::queue<Action*> action_queue;
	static ActionQueue* current;
	std::mutex guard;
	ActionQueue();
public:
	static ActionQueue* get_instance();
	void push(Action* act);
	void activate_actions_in_queue();
};


