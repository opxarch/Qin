/** @file
 * Qin - MIDI mapping.
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

#include "util/error.h"

#include "midi/note.h"
#include "midi/event.h"
#include "midi/mapping.h"


namespace midi
{

/**
 * Static MIDI mapping table
 */
#define MAPPING_NUM (21)

struct mapping
{
  int16_t midiKey;
  Note qinNote;
}
mappingTable[MAPPING_NUM] = {
    {36, NOTE_D1},
    {38, NOTE_E1},
    {40, NOTE_FS1},
    {41, NOTE_A1},
    {43, NOTE_B1},
    {45, NOTE_D2},
    {47, NOTE_E2},
    {48, NOTE_FS2},
    {50, NOTE_A2},
    {52, NOTE_B2},
    {53, NOTE_D3},
    {55, NOTE_E3},
    {57, NOTE_FS3},
    {59, NOTE_A3},
    {60, NOTE_B3},
    {62, NOTE_D4},
    {64, NOTE_E4},
    {65, NOTE_FS4},
    {67, NOTE_A4},
    {69, NOTE_B4},
    {71, NOTE_D5}
};

////////////////////////////////////////////////////////////////////////////////

/**
 * Mapping between MIDI key and Qin note.
 * @param key       MIDI key value.
 * @return Qin note index.
 */
Note
mapNote(int16_t key)
{
  for (int i = 0; i < MAPPING_NUM; i++)
    {
      if (mappingTable[i].midiKey == key)
        {
          return mappingTable[i].qinNote;
        }
    }
  // not matched
  return NOTE_INVALID;
}


/**
 * Mapping between MIDI key and Qin note.
 * @param key       Qin note index.
 * @return MIDI key value.
 */
int16_t
mapKey(Note note)
{
  for (int i = 0; i < MAPPING_NUM; i++)
    {
      if (mappingTable[i].qinNote == note)
        {
          return mappingTable[i].midiKey;
        }
    }
  // not matched
  return NOTE_INVALID;
}

} // namespace midi
