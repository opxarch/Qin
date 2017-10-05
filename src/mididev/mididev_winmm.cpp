/** @file
 * Qin - MIDI device for windows mmsystem.
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
#include <map>

#include "util/misc.h"
#include "util/error.h"
#include "util/log.h"
#include "midi/event.h"
#include "mididev/mididev.h"

#include <windows.h>

namespace mididev
{

////////////////////////////////////////////////////////////////////////////////

class MidiControllerWinMMImpl : public IMidiController
{
private:

  std::map<HMIDIIN, std::string> m_inputDevices;

private:

  static void WINAPI CALLBACK inputCallback( HMIDIIN hm, UINT msg,
        DWORD_PTR inst,
        DWORD_PTR param1,
        DWORD_PTR param2 )
  {
    if( msg == MIM_DATA )
      {
          ( (MidiControllerWinMMImpl *) inst )->handleInputEvent( hm, param1 );
      }
  }

public:
  MidiControllerWinMMImpl()
  {
  }

  /**
   * Setup the midi controller
   * @param flags unused.
   * @return status code.
   */
  int init(int flags)
  {
    int count = 0;

    /*
     * Open all the input devices
     */
    m_inputDevices.clear();
    for( unsigned int i = 0; i < midiInGetNumDevs(); ++i )
      {
        MIDIINCAPS c;
        midiInGetDevCaps( i, &c, sizeof( c ) );
        HMIDIIN hm = 0;
        MMRESULT res = midiInOpen( &hm, i, (DWORD_PTR) &inputCallback,
                        (DWORD_PTR) this,
                          CALLBACK_FUNCTION );
        if( res == MMSYSERR_NOERROR )
          {
            m_inputDevices[hm] = c.szPname;
            midiInStart( hm );
            count++;
          }
      }

    return count ? VINF_SUCCEEDED : VERR_FAILED;
  }

  /**
   * close the midi controller
   * @param flags unused.
   * @return status code.
   */
  int uninit(int flags)
  {
    std::map<HMIDIIN, std::string>::iterator i;
    for (i = m_inputDevices.begin(); i != m_inputDevices.end(); ++i)
      {
        midiInStop( ( *i ).first );
        midiInClose( ( *i ).first );
      }

    m_inputDevices.clear();
    return VINF_SUCCEEDED;
  }

  /**
   * Get the count of controllers
   * @return the result
   */
  int
  getInputNum()
  {
    return m_inputDevices.size();
  }

  /**
   * Get the name of one controller.
   * @param idx     The index of target, beginning with 0.
   * @param buff    Where to store the string.
   * @return status code.
   */
  int
  getInputName(unsigned idx, std::string &buff)
  {
    if (idx >= m_inputDevices.size())
      {
        return VERR_OUT_OF_RANGE;
      }
    else
      {
        unsigned n = 0;
        std::map<HMIDIIN, std::string>::iterator i;
        for (i = m_inputDevices.begin(); i != m_inputDevices.end(); ++i)
          {
            if (n++ == idx)
              {
                buff = ( *i ).second;
                break;
              }
          }
      }
    return VINF_SUCCEEDED;
  }

protected:
  void handleInputEvent( HMIDIIN hm, DWORD ev )
  {
    const int cmd = ev & 0xff;
    if( cmd == midi::ActiveSensing )
      {
        return;
      }
    const int par1 = ( ev >> 8 ) & 0xff;
    const int par2 = ev >> 16;
    const midi::EventTypes cmdtype = static_cast<midi::EventTypes>( cmd & 0xf0 );
    const int chan = cmd & 0x0f;

    const std::string &d = m_inputDevices[hm];
    if( d.empty() )
      {
        return;
      }

    std::vector<IMIDIPort*>::iterator it;
    for( it = m_inputPorts.begin(); it != m_inputPorts.end(); ++it )
      {
        switch( cmdtype )
        {
          case midi::NoteOn:
          case midi::NoteOff:
          case midi::KeyPressure:
            ( *it )->processMidiEvent( midi::Event( cmdtype, chan, par1 - midi::KeysPerOctave, par2 & 0xff ) );
            break;

          case midi::ControlChange:
          case midi::ProgramChange:
          case midi::ChannelPressure:
            ( *it )->processMidiEvent( midi::Event( cmdtype, chan, par1, par2 & 0xff ) );
            break;

          case midi::PitchBend:
            ( *it )->processMidiEvent( midi::Event( cmdtype, chan, par1 + par2*128, 0 ) );
            break;

          default:
            LOG(WARNING) << "MidiWinMM: unhandled input event \n";
            break;
        }
      }
  }

} instance;

/*
 * Export this instance
 */
IMidiController *g_mididev_winmm = static_cast<IMidiController *>(&instance);


} // namespace mididev
