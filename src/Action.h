#pragma once
#include <string>
#include <queue>
#include <atomic>
#include <mutex>
#include <map>
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
	Action(XPLMDataRef dat, int array_index, int d);
	Action(XPLMDataRef dat, float _delta, float _max, float _min);
	Action(XPLMCommandRef cmd, CommandType type);
	Action(std::string lua_str);
	~Action();
	void add_condition(std::string _condition);
	void set_condition_active(std::string _active_condition);
	void set_condition_inactive(std::string _active_condition);
	void activate();
private:
	int data = 0;
	int index = -1;
	float delta = 0;
	float max = 0;
	float min = 0;
	std::string lua_str = "";
	std::string condition = "";
	std::map<std::string,bool> active_conditions;
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


