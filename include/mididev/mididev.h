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

#ifndef MIDIDEV_H_
#define MIDIDEV_H_

#include <vector>
#include <string>

#include "midi/event.h"

namespace mididev {

/***************************************************
  *****        Interface of MIDI ports         *****
  ***************************************************/
class IMIDIPort {
public:
  virtual ~IMIDIPort() {}

  virtual int processMidiEvent(midi::Event event)=0;
};

/***************************************************
  *****        Interface of MIDI device        *****
  ***************************************************/

class IMidiController {
public:
    IMidiController() {}
    virtual ~IMidiController() {}

    virtual int init(int flags)=0;
    virtual int uninit(int flags)=0;
    virtual int getInputNum()=0;
    virtual int getInputName(unsigned idx, std::string &buff)=0;

    virtual void addInputPort(IMIDIPort *src)
    {
      m_inputPorts.push_back(src);
    }

public:
    std::vector<IMIDIPort*> m_inputPorts;
};


/***************************************************
  *****         MIDI controller device         *****
  ***************************************************/
class MidiDev {
public:
  MidiDev();

  int openDevices();
  void closeDevices();

  IMidiController *current()
  {
    return m_currentDevice;
  }

  void dumpDeviceList(void);

private:
  IMidiController *m_currentDevice;
};

} // namespace mididev

#endif //!defined(MIDIDEV_H_)
