#pragma once
#include <sstream>
#include <list>
#include <mutex>
#include <ctime>
#include "XPLMUtilities.h"

enum TLogLevel
{
	logERROR, logWARNING, logINFO, logDEBUG, logTRACE
};

class Logger: public std::ostringstream
{
private:
	TLogLevel last_message_log_level;
	static TLogLevel current_log_level;
	static std::list<std::string> saved_errors_and_warnings;
	static std::mutex guard;
public:
	static void set_log_level(TLogLevel level)
	{
		current_log_level = level;
	}

	static int number_of_stored_messages()
	{	
		guard.lock();
		int number_of_messages = saved_errors_and_warnings.size();
		guard.unlock();

		return number_of_messages;
	}

	static std::list<std::string> get_and_clear_stored_messages()
	{	
		guard.lock();
		std::list<std::string> messages = saved_errors_and_warnings;
		saved_errors_and_warnings.clear();
		guard.unlock();

		return messages;
	}

	Logger(TLogLevel level):std::ostringstream()
	{
		last_message_log_level = level;
	}

	~Logger()
	{		
		auto time_since_epoch = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

		if (current_log_level >= last_message_log_level)
		{
			std::string log_level_str = "";
			switch (last_message_log_level) {
			case TLogLevel::logDEBUG:
				log_level_str = "[DEBUG]:";
				break;
			case TLogLevel::logERROR:
				log_level_str = "[ERROR]:";
				break;
			case TLogLevel::logINFO:
				log_level_str = "[INFO]:";
				break;
			case TLogLevel::logTRACE:
				log_level_str = "[TRACE]:";
				break;
			case TLogLevel::logWARNING:
				log_level_str = "[WARNING]:";
				break;
			default:
				log_level_str = "[UNKNOWN]:";
			} 
			XPLMDebugString(("XPanel ["+ std::to_string(time_since_epoch & 0xffff) + "] " + log_level_str + str()).c_str());

			if (last_message_log_level == TLogLevel::logERROR || last_message_log_level == TLogLevel::logWARNING)
			{
				guard.lock();
				Logger::saved_errors_and_warnings.push_back(log_level_str + str());
				guard.unlock();
			}
		}
	}
};