/** @file
 * Qin - midi note.
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

#include "util/string.h"

#include "midi/note.h"

////////////////////////////////////////////////////////////////////////////////

namespace midi {

struct noteDesc
{
  const char *symbol;
  Note index;
};

static noteDesc notes[] =
{
  {"d1",  NOTE_D1},
  {"e1",  NOTE_E1},
  {"f#1", NOTE_FS1},
  {"a1",  NOTE_A1},
  {"b1",  NOTE_B1},
  {"d2",  NOTE_D2},
  {"e2",  NOTE_E2},
  {"f#2", NOTE_FS2},
  {"a2",  NOTE_A2},
  {"b2",  NOTE_B2},
  {"d3",  NOTE_D3},
  {"e3",  NOTE_E3},
  {"f#3", NOTE_FS3},
  {"a3",  NOTE_A3},
  {"b3",  NOTE_B3},
  {"d4",  NOTE_D4},
  {"e4",  NOTE_E4},
  {"f#4", NOTE_FS4},
  {"a4",  NOTE_A4},
  {"b4",  NOTE_B4},
  {"d5",  NOTE_D5},
  {0, NOTE_INVALID},
};

/*
 * Convert a string to note index.
 * @param src               Source string.
 * @return NOTE_INVALID     The source string is invalid.
 * @return note index if succeeded.
 */
Note
stringToNote(const char *src)
{
  for (int i=0; notes[i].symbol; i++)
    {
      /*
       * FIXME! add the whole word matching.
       */
      if (v_strcasecmp(src, notes[i].symbol) == 0)
        {
          return notes[i].index;
        }
    }
  return NOTE_INVALID;
}

} // namespace midi
