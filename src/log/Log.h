#pragma once

#include <ostream>
#include <string>
#include <ctime>

enum class LogLevel : std::int8_t {INFO = 0, DEBUG = 1, WARNING = 2, ERROR = 3};
const char* levelName[] = {" INFO", "DEBUG", " WARN", "ERROR"}; 

class Log
{
public:
  static char timeBuffer[20];

  static char* getTime();
};

char Log::timeBuffer[20];

inline char* Log::getTime()
{
  time_t current;
  struct tm* timeinfo;

  time(&current);
  timeinfo = localtime(&current);

  // format time into mm/dd/yy hh:MM::ss
  strftime(timeBuffer, 20, "%x %X", timeinfo);

  return timeBuffer;
}

// setl(set level) manipulator 
class setl
{
public:
  explicit setl(LogLevel _level) : level(_level) {}
private:
  LogLevel level;

  template <class charT, class Traits>
  friend std::basic_ostream<charT, Traits>& operator<<
  (std::basic_ostream<charT, Traits>& os, const setl& l)
  {
    os << "- " << Log::getTime() << " [" << levelName[static_cast<int>(l.level)] << "] ";
    os.flush();

    return os;
  }
};
