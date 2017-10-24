/*
 *  Qin is Copyright (C) 2016, The 1st Middle School in
 *  Yongsheng Lijiang, Yunnan Province, ZIP 674200 China
 *
 *  This project is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public License(GPL)
 *  as published by the Free Software Foundation; either version 2.1
 *  of the License, or (at your option) any later version.
 *
 *  This project is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 */

#ifndef V_LOG_H_
#define V_LOG_H_

#include "util/misc.h"

/*
 * Logging
 */

enum LogLevel {
  INFO = 0,
  WARNING,
  ERR,
  VERBOSE,
  DEBUG0,
  DEBUG1,
  DEBUG2,
};


#if ENABLE(LOG)
# define logLevelThreshold (DEBUG2)
#else
# define logLevelThreshold (ERR)
#endif

class logstream{
public:
  logstream(LogLevel level)
  {
    m_level = level;
  }

 template <class T>
  std::ostream & operator<<(T value) const
  {
    if (m_level <= logLevelThreshold)
      {
#if 0
        return std::cout.operator <<(value);
#endif
        return std::cout << value;
      }
    return std::cout;
  }

 ~logstream()
 {
   std::cout.flush(); /* flush the stream */
 }

private:
  LogLevel m_level;
};

# define LOG(level) logstream(level)


#endif //!defined(V_LOG_H_)
