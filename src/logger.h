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
			XPLMDebugString(("XPanel: "+str()).c_str());
		}
	}
};