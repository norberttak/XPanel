#pragma once
#include <sstream>
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
public:
	static void set_log_level(TLogLevel level)
	{
		current_log_level = level;
	}

	Logger(TLogLevel level):std::ostringstream()
	{
		last_message_log_level = level;
	}

	~Logger()
	{
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
			XPLMDebugString(("XPanel "+log_level_str+str()).c_str());
		}
	}
};