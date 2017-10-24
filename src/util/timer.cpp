/** @file
 * Util - Timer system.
 */

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

#include "util/timer.h"
#include "util/misc.h"
#include "util/error.h"

#if OS(WIN32)
# include <windows.h>
# include <mmsystem.h>
#elif OS(LINUX)
# include <unistd.h>
# ifdef __BEOS__
#  define usleep(t) snooze(t)
# endif
# include <stdlib.h>
# include <time.h>
# include <sys/time.h>
#else
# error port me!
#endif

const char *
getTimerName()
{
#if OS(WIN32)
  return "Windows native";
#elif OS(LINUX)
# ifdef HAVE_NANOSLEEP
  return "nanosleep()";
# else
  return "usleep()";
# endif
#else
  return "unknown";
#endif
}

/*
 * Get the current time in microseconds
 */
unsigned int getTimer()
{
#if OS(WIN32)
  return timeGetTime() * 1000;
#elif OS(LINUX)
  struct timeval tv;
//  float s;
  gettimeofday(&tv,NULL);
//  s=tv.tv_usec;s*=0.000001;s+=tv.tv_sec;
  return (tv.tv_sec*1000000+tv.tv_usec);
#else
  return 0;
#endif
}

/*
 * Returns current time in milliseconds
 */
unsigned int
getTimerMS()
{
#if OS(WIN32)
  return timeGetTime() ;
#elif OS(LINUX)
  struct timeval tv;
//  float s;
  gettimeofday(&tv,NULL);
//  s=tv.tv_usec;s*=0.000001;s+=tv.tv_sec;
  return (tv.tv_sec*1000+tv.tv_usec/1000);
#else
  return 0;
#endif
}

int
usecSleep(int usec_delay)
{
  // Sleep(0) won't sleep for one clocktick as the unix usleep
  // instead it will only make the thread ready
  // it may take some time until it actually starts to run again
#if OS(WIN32)
  if(usec_delay<1000)usec_delay=1000;
  Sleep( usec_delay/1000);
  return VINF_SUCCEEDED;
#elif OS(LINUX)
# ifdef HAVE_NANOSLEEP
  struct timespec ts;
  ts.tv_sec  =  usec_delay / 1000000;
  ts.tv_nsec = (usec_delay % 1000000) * 1000;
  return nanosleep(&ts, NULL) ? VINF_SUCCEEDED : VERR_FAILED;
# else
  return usleep(usec_delay) ? VINF_SUCCEEDED : VERR_FAILED;
# endif
#else
  return VERR_FAILED;
#endif
}

