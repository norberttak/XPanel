#include "logger.h"

TLogLevel Logger::current_log_level = TLogLevel::logINFO;
std::list<std::string> Logger::saved_errors_and_warnings;
std::mutex Logger::guard;