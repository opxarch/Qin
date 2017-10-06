/** @file
 * Qin - audio system.
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

/*******************************************************************************
*   Header Files                                                               *
*******************************************************************************/
#include <cstdlib>
#include <cstdio>
#include <cstring>

#include "util/misc.h"
#include "util/error.h"
#include "util/log.h"
#include "audiosys/audiosystem.h"

namespace audiosys {

/*******************************************************************************
*   Global Variables                                                           *
*******************************************************************************/

#if USES(DSOUND_AUDIO)
  extern IAudioOutput *audio_out_dsound;
#endif
extern IAudioOutput *audio_out_null;

IAudioOutput* audio_out_drivers[] =
{
#if USES(DSOUND_AUDIO)
  audio_out_dsound,
#endif
  audio_out_null,
  0
};


////////////////////////////////////////////////////////////////////////////////


IAudioOutput::IAudioOutput() {
    m_pts = 0;
    m_buffersize = -1;
    m_outburst =
    m_bps =
    m_format =
    m_channels =
    m_samplerate = 0;
}

IAudioOutput::~IAudioOutput() {
}

/*
 * void implements
 */
const char *IAudioOutput::getname() const {
  return "NULL";
}
const char *IAudioOutput::getshortname() const {
  return "NULL";
}
const char *IAudioOutput::getauthor() const {
  return "NULL";
}
const char *IAudioOutput::getcomment() const {
  return "NULL";
}



////////////////////////////////////////////////////////////////////////////////

/**
 * Print a list of valid devices.
 */
void
AudioSystem::dumpDeviceList(void)
{
  int i = 0;
  while (audio_out_drivers[i])
    {
      IAudioOutput *ia = audio_out_drivers[i++];
      printf("device: short_name[%s] name(%s)\n", ia->getshortname(), ia->getname());
    }
}

/*
 * Create and initiate a new audio device.
 * @param ao        Where to store the pointer to the new audio device interface.
 * @param name      The name of device you want to create.
 * @param rate      Sampling rate you want.
 * @param channels  The number of channels required.
 * @param format    Target format of audio.
 * @param flags     Extra parameters.
 * @return  status code.
 */
int
AudioSystem::initDevice(IAudioOutput **ao, const char *name, int rate, int channels, int format, float delay, int flags)
{
  int rc;
  int i = 0;
  const char *buf = 0;
  for(i=0;audio_out_drivers[i];i++)
    {
      IAudioOutput* audio_out = audio_out_drivers[i];

      buf = audio_out->getshortname();
      if (!strncmp(buf, name, strlen(buf)))
        {
          /* name matches, try it */
          rc = audio_out->init(rate,channels,format,delay,flags);
          if (V_SUCCESS(rc))
            {
              *ao = audio_out; /* succeeded! */
              return VINF_SUCCEEDED;
            }
          else
            LOG(ERR) << "failed on initiating '" << name << "'\n";
        }
    }

  /* now try the rest... */
  for(i=0;audio_out_drivers[i];i++)
    {
      IAudioOutput* audio_out=audio_out_drivers[i];

      rc = audio_out->init(rate,channels,format,delay,flags);
      if (V_SUCCESS(rc))
        {
          *ao = audio_out; /* succeeded! */
          return VINF_REPLACED;
        }
    }
  *ao = 0;
  return VERR_FAILED;
}

/**
 * Uninitlate the audio device.
 * @param dev       Pointer to the device.
 * @return status code.
 */
int
AudioSystem::uninitDevice(IAudioOutput *dev, int flags)
{
  return dev->uninit(flags);
}

/**
 * Resume the audio stream.
 * @param dev       Pointer to the device.
 * @return status code.
 */
void
AudioSystem::resume(IAudioOutput *dev)
{
  dev->resume();
}

/**
 * Pause the audio stream.
 * @param dev       Pointer to the device.
 * @return status code.
 */
void
AudioSystem::pause(IAudioOutput *dev)
{
  dev->pause();
}

/**
 * Get the delay of audio stream.
 * @param dev       Pointer to the device.
 * @return status code.
 */
int
AudioSystem::getDelay(IAudioOutput *dev)
{
  return dev->get_delay();
}

/**
 * Control the audio stream.
 * @param dev       Pointer to the device.
 * @param cmd       The command that is to be passed.
 * @return status code.
 */
int
AudioSystem::control(IAudioOutput *dev, AudioCTRL cmd, void *arg, ...)
{
  return dev->control(cmd, arg);
}

int
AudioSystem::write(IAudioOutput *dev, char *data, int length, int flags)
{
  return dev->write(data, length, flags);
}

} // namespace audiosys
