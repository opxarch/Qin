/** @file
 * Qin - MIDI message.
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
#include "util/error.h"

#include "midi/event.h"
#include "midi/message.h"


namespace midi
{

////////////////////////////////////////////////////////////////////////////////

int
formatMidiMessage(std::string &out, const Event &event)
{
  char buff[1024];

  switch (event.type())
  {
    case PitchBend:
      sprintf(buff, "pitch bend: channel = %04d, value = %05d.",
              event.channel(),
              event.pitchBend()
              );
      break;

    case SongPosition:
      sprintf(buff, "song position: param0 = %04x.",
              event.param( 0 )
              );
      break;

    case ProgramChange:
      sprintf(buff, "program change: param0 = %04x.",
              event.param( 0 )
              );
      break;

    case SongSelect:
      sprintf(buff, "song select: param0 = %04x.",
              event.param( 0 )
              );
      break;

    case NoteOff:
      sprintf(buff, "note off: key = %04d, velocity = %04d.",
              event.key(),
              event.velocity()
              );
      break;
    case NoteOn:
      sprintf(buff, "note on: key = %04d, velocity = %04d.",
              event.key(),
              event.velocity()
              );
      break;

    case ControlChange:
      sprintf(buff, "control change: param0 = %04x.",
              event.param( 0 )
              );
      break;

    default:
      out = "unknown event";
  }

  out.assign(buff);
  return VINF_SUCCEEDED;
}

} // namespace midi
