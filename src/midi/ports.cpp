/** @file
 * Qin - MIDI ports.
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

#include "mididev/mididev.h"
#include "midi/ports.h"


namespace midi
{

////////////////////////////////////////////////////////////////////////////////

Ports::Ports() :
    m_writePos(0),
    m_readPos(0),
    m_queueCount(0)
{
}

/**
 * Query is the queue empty?
 * @return true if empty.
 * @return false in the contrary.
 */
bool
Ports::queueEmpty()
{
  return (m_writePos == m_readPos) && (m_queueCount == 0);
}

/**
 * Query is the queue full?
 * @return true if empty.
 * @return false in the contrary.
 */
bool
Ports::queueFull()
{
  return (m_writePos == m_readPos) && (m_queueCount == _MAX_MIDI_EVENT_QUEUE_SIZE);
}

/**
 * Pop a event from the queue.
 * @param out       Where to store the event.
 * @return status code.
 */
int
Ports::queuePop(midi::Event &out)
{
  if ((m_writePos == m_readPos) && (m_queueCount == 0))
    {
      return VERR_QUEUE_EMPTY;
    }
  else
    {
      out = m_eventQueue[m_readPos];
      m_readPos = (m_readPos + 1) % _MAX_MIDI_EVENT_QUEUE_SIZE;
      m_queueCount--;
    }
  return VINF_SUCCEEDED;
}

/**
 * Process the MIDI event.
 * @param event     The source event
 * @return status code.
 */
int
Ports::processMidiEvent(midi::Event event)
{
  if ((m_writePos == m_readPos) && (m_queueCount == _MAX_MIDI_EVENT_QUEUE_SIZE))
    {
      return VERR_QUEUE_FULL;
    }
  else
    {
      /*
       * push it to the back of queue
       */
      m_eventQueue[m_writePos] = event;
      m_writePos = (m_writePos + 1) % _MAX_MIDI_EVENT_QUEUE_SIZE;
      m_queueCount++;
    }
  return VINF_SUCCEEDED;
}

} // namespace midi
