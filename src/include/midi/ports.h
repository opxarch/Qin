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

#ifndef MIDI_PORTS_H_
#define MIDI_PORTS_H_

#include "util/types.h"

#include "mididev/mididev.h"

#define _MAX_MIDI_EVENT_QUEUE_SIZE 64

namespace midi
{

/***************************************************
  *****         MIDI controller device         *****
  ***************************************************/
class Ports : public mididev::IMIDIPort {
public:
  Ports();

  bool queueEmpty();
  bool queueFull();
  int queuePop(midi::Event &out);

  int processMidiEvent(midi::Event event);

private:
  midi::Event   m_eventQueue[_MAX_MIDI_EVENT_QUEUE_SIZE];
  int           m_writePos;
  int           m_readPos;
  int           m_queueCount;
};

} // namespace midi

#endif //!defined(MIDI_PORTS_H_)
