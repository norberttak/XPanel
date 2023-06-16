/*
 * Copyright 2022 Norbert Takacs
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once
#include <string>
#include <queue>
#include <atomic>
#include <mutex>
#include <map>
#include <list>
#include "AgeingCounter.h"
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
	Action(XPLMDataRef dat, float d);
	Action(XPLMDataRef dat, int array_index, int d);
	Action(XPLMDataRef dat, int array_index, float d);
	Action(XPLMDataRef dat, float _delta, float _max, float _min);
	Action(XPLMCommandRef cmd, CommandType type);
	Action(std::string _lua_str);
	~Action();
	void add_condition(std::string _condition);
	void set_condition_active(std::string _active_condition);
	void set_condition_inactive(std::string _active_condition);
	void set_dynamic_speed_params(float _tick_per_sec_mid, int _multi_mid, float tick_per_sec_high, int _multi_high);
	void get_dynamic_speed_params(float* tick_per_sec_mid, int* _multi_mid, float* tick_per_sec_high, int* _multi_high);
	void speed_up(int multi);
	int get_speed_multi();
	int get_hash();
	void activate();
private:
	int data = 0;
	float data_f = 0;
	int index = -1;
	float delta = 0;
	XPLMDataTypeID dataref_type;
	//Dynamic speed params:
	float tick_per_sec_mid=0;
	float tick_per_sec_high=0;
	int multi_low = 1;
	int multi_high = 1;
	int multi = 1;
	//Limits for dataref value:
	float max = 0;
	float min = 0;
	std::string lua_str = "";
	std::string condition = "";
	std::map<std::string, bool> active_conditions;
	XPLMDataRef dataref = NULL;
	CommandType command_type = CommandType::NONE;
	XPLMCommandRef commandref = NULL;
};

class ActionQueue
{
private:
	std::list<Action*> action_queue;
	static ActionQueue* current;
	std::mutex guard;
	ActionQueue();
	//std::map<int, uint64_t> action_happend_last_time;
	std::map<int, AgeingCounter*> action_ageing_counters;
public:
	static ActionQueue* get_instance();
	void push(Action* act);
	void activate_actions_in_queue();
	void clear_all_actions();
};
