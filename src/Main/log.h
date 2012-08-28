#pragma once

#include <ostream>
#include <string>
#include <ctime>

enum loglevel {ERROR, WARNING, DEBUG};

class log
{
public:
  static const loglevel error = ERROR;
  static const loglevel warning = WARNING;
  static const loglevel debug = DEBUG;
  
  static const char* levelName[];
  static char timeBuffer[20];
  
  static char* getTime();
};

const char* log::levelName[] = {"Error", "Warning", "Debug"}; 
char log::timeBuffer[20];

inline char* log::getTime()
{
  time_t current;
  struct tm* timeinfo;

  time(&current);
  timeinfo = localtime(&current);

  strftime(timeBuffer, 20, "%x %X", timeinfo);

  return timeBuffer;
}

// setl(set level) manipulator 
class setl
{
public:
  explicit setl(loglevel _level) : level(_level) {}
private:
  loglevel level;

  template <class charT, class Traits>
  friend std::basic_ostream<charT, Traits>& operator<<
  (std::basic_ostream<charT, Traits>& os, const setl& l)
  {
    os << "- " << log::getTime();
    os << " " << log::levelName[l.level] << ": ";
    os.flush();
  }
};
