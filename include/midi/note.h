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

#ifndef MIDI_NOTE_H_
#define MIDI_NOTE_H_

#include "util/types.h"

namespace midi {

/*
 * Notes
 */
enum Note
{
  NOTE_INVALID = 0,
  NOTE_D1,
  NOTE_E1,
  NOTE_FS1,
  NOTE_A1,
  NOTE_B1,
  NOTE_D2,
  NOTE_E2,
  NOTE_FS2,
  NOTE_A2,
  NOTE_B2,
  NOTE_D3,
  NOTE_E3,
  NOTE_FS3,
  NOTE_A3,
  NOTE_B3,
  NOTE_D4,
  NOTE_E4,
  NOTE_FS4,
  NOTE_A4,
  NOTE_B4,
  NOTE_D5,
  _MAX_NOTE_NUM
};

typedef uint8_t Velocity; /* we have just used the range of 0~128 */

Note stringToNote(const char *src);

/*
 * The maximum number of polyphony.
 */
#define _MAX_POLYPHONY_NUM (16)

} // namespace midi

#endif //!defined(MIDI_NOTE_H_)
