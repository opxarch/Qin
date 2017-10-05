/** @file
 * Qin - MIDI device.
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

#include "util/misc.h"
#include "util/error.h"
#include "util/log.h"
#include "mididev/mididev.h"

namespace mididev {

/*******************************************************************************
*   Global Variables                                                           *
*******************************************************************************/

#if USES(MIDI_DEVICE_WINMM)
  extern IMidiController* g_mididev_winmm;
#endif

IMidiController* midi_controllers[] =
{
#if USES(MIDI_DEVICE_WINMM)
  g_mididev_winmm,
#endif
  0
};

////////////////////////////////////////////////////////////////////////////////

MidiDev::MidiDev() :
    m_currentDevice(0)
{
}

/**
 * Open the MIDI devices.
 * @return status code.
 */
int
MidiDev::openDevices()
{
  int rc = VERR_FAILED;
  IMidiController **devs = midi_controllers;

  for (int i = 0; devs[i]; i++)
    {
      rc = devs[i]->init(0);
      if (V_SUCCESS(rc))
        {
          m_currentDevice = devs[i];
          break;
        }
    }
  return rc;
}

/*
 * Close the MIDI devices.
 * #return status code.
 */
void
MidiDev::closeDevices()
{
  if (m_currentDevice)
    {
      m_currentDevice->uninit(0);
      m_currentDevice = 0;
    }
}

} // namespace mididev
