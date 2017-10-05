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

#ifndef MIDI_EVENT_H_
#define MIDI_EVENT_H_

#include "util/types.h"

#include "midi/note.h"

namespace midi
{


enum EventTypes
{
	// messages
	NoteOff = 0x80,
	NoteOn = 0x90,
	KeyPressure = 0xA0,
	ControlChange = 0xB0,
	ProgramChange = 0xC0,
	ChannelPressure = 0xD0,
	PitchBend = 0xE0,
	// system exclusive
	SysEx= 0xF0,
	// system common - never in midi files
	TimeCode= 0xF1,
	SongPosition = 0xF2,
	SongSelect = 0xF3,
	TuneRequest = 0xF6,
	EOX= 0xF7,
	// system real-time - never in midi files
	Sync = 0xF8,
	Tick = 0xF9,
	Start = 0xFA,
	Continue = 0xFB,
	Stop = 0xFC,
	ActiveSensing = 0xFE,
	SystemReset = 0xFF,
	// meta event - for midi files only
	MetaEvent = 0xFF
} ;

enum MetaEventTypes
{
	MetaInvalid = 0x00,
	Copyright = 0x02,
	TrackName = 0x03,
	InstName = 0x04,
	Lyric = 0x05,
	Marker = 0x06,
	CuePoint = 0x07,
	PortNumber = 0x21,
	EOT = 0x2f,
	SetTempo = 0x51,
	SMPTEOffset = 0x54,
	TimeSignature = 0x58,
	KeySignature = 0x59,
	SequencerEvent = 0x7f,
	MetaCustom = 0x80,
	NotePanning
} ;
typedef MetaEventTypes MetaEventType;


enum StandardControllers
{
	ControllerBankSelect = 0,
	ControllerModulationWheel = 1,
	ControllerBreathController = 2,
	ControllerFootController = 4,
	ControllerPortamentoTime = 5,
	ControllerDataEntry = 6,
	ControllerMainVolume = 7,
	ControllerBalance = 8,
	ControllerPan = 10,
	ControllerEffectControl1 = 12,
	ControllerEffectControl2 = 13,
	ControllerSustain = 64,
	ControllerPortamento = 65,
	ControllerSostenuto = 66,
	ControllerSoftPedal = 67,
	ControllerLegatoFootswitch = 68,
	ControllerRegisteredParameterNumberLSB = 100,
	ControllerRegisteredParameterNumberMSB = 101,
	// Channel Mode Messages are controllers too...
	ControllerAllSoundOff = 120,
	ControllerResetAllControllers = 121,
	ControllerLocalControl = 122,
	ControllerAllNotesOff = 123,
	ControllerOmniOn = 124,
	ControllerOmniOff = 125,
	ControllerMonoOn = 126,
	ControllerPolyOn = 127,

};

enum ControllerRegisteredParameterNumbers
{
	PitchBendSensitivityRPN = 0x0000,
	ChannelFineTuningRPN = 0x0001,
	ChannelCoarseTuningRPN = 0x0002,
	TuningProgramChangeRPN = 0x0003,
	TuningBankSelectRPN = 0x0004,
	ModulationDepthRangeRPN = 0x0005,
	NullFunctionNumberRPN = 0x7F7F
};

const int ChannelCount = 16;
const int ControllerCount = 128;
const int ProgramCount = 128;
const int MaxVelocity = 127;
const int DefaultVelocity = MaxVelocity / 2;
const int MaxControllerValue = 127;
const int MaxKey = 127;

const int MaxPanning = 127;
const int MinPanning = -128;

const int MinPitchBend = 0;
const int MaxPitchBend = 16383;



enum Keys
{
    Key_C = 0,
    Key_CIS = 1, Key_DES = 1,
    Key_D = 2,
    Key_DIS = 3, Key_ES = 3,
    Key_E = 4, Key_FES = 4,
    Key_F = 5,
    Key_FIS = 6, Key_GES = 6,
    Key_G = 7,
    Key_GIS = 8, Key_AS = 8,
    Key_A = 9,
    Key_AIS = 10, Key_B = 10,
    Key_H = 11
} ;

enum Octaves
{
    Octave_0,
    Octave_1,
    Octave_2,
    Octave_3,
    Octave_4, DefaultOctave = Octave_4,
    Octave_5,
    Octave_6,
    Octave_7,
    Octave_8,
    NumOctaves
} ;


const int WhiteKeysPerOctave = 7;
const int BlackKeysPerOctave = 5;
const int KeysPerOctave = WhiteKeysPerOctave + BlackKeysPerOctave;
const int NumKeys = NumOctaves * KeysPerOctave;
const int DefaultKey = DefaultOctave*KeysPerOctave + Key_A;

const float MaxDetuning = 4 * 12.0f;



class Event
{
public:
  Event( EventTypes type = ActiveSensing,
              int8_t channel = 0,
              int16_t param1 = 0,
              int16_t param2 = 0) :
      m_type( type ),
      m_metaEvent( MetaInvalid ),
      m_channel( channel ),
      m_sysExData( NULL )
  {
      m_data.m_param[0] = param1;
      m_data.m_param[1] = param2;
  }

  Event( EventTypes type, const char* sysExData, int dataLen ) :
      m_type( type ),
      m_metaEvent( MetaInvalid ),
      m_channel( 0 ),
      m_sysExData( sysExData )
  {
      m_data.m_sysExDataLen = dataLen;
  }

  Event( const Event& other ) :
      m_type( other.m_type ),
      m_metaEvent( other.m_metaEvent ),
      m_channel( other.m_channel ),
      m_data( other.m_data ),
      m_sysExData( other.m_sysExData )
  {
  }

  inline EventTypes type() const
  {
    return m_type;
  }

  inline void setType( EventTypes type )
  {
    m_type = type;
  }

  inline MetaEventType metaEvent() const
  {
    return m_metaEvent;
  }

  inline void setMetaEvent( MetaEventType metaEvent )
  {
    m_metaEvent = metaEvent;
  }

  inline int8_t channel() const
  {
    return m_channel;
  }

  inline void setChannel( int8_t channel )
  {
    m_channel = channel;
  }

  inline int16_t param( int i ) const
  {
    return m_data.m_param[i];
  }

  inline void setParam( int i, uint16_t value )
  {
    m_data.m_param[i] = value;
  }

  inline int16_t key() const
  {
    return param( 0 );
  }

  inline void setKey( int16_t key )
  {
    m_data.m_param[0] = key;
  }

  inline uint8_t velocity() const
  {
    return m_data.m_param[1] & 0x7F;
  }

  inline void setVelocity( int16_t velocity )
  {
    m_data.m_param[1] = velocity;
  }
/*
  panning_t panning() const
  {
      return (panning_t) ( PanningLeft +
          ( (float)( midiPanning() - MidiMinPanning ) ) /
          ( (float)( MidiMaxPanning - MidiMinPanning ) ) *
          ( (float)( PanningRight - PanningLeft ) ) );
  }

  int16_t midiPanning() const
  {
      return m_data.m_param[1];
  }

  volume_t volume( int midiBaseVelocity ) const
  {
    return (volume_t)( velocity() * DefaultVolume / midiBaseVelocity );
  }
*/

  inline uint8_t controllerNumber() const
  {
    return param( 0 ) & 0x7F;
  }

  inline void setControllerNumber( uint8_t num )
  {
    setParam( 0, num );
  }

  inline uint8_t controllerValue() const
  {
    return param( 1 );
  }

  void setControllerValue( uint8_t value )
  {
    setParam( 1, value );
  }

  inline uint8_t program() const
  {
    return param( 0 );
  }

  inline uint8_t channelPressure() const
  {
    return param( 0 );
  }

  inline int16_t pitchBend() const
  {
    return param( 0 );
  }

  void setPitchBend( uint16_t pitchBend )
  {
    setParam( 0, pitchBend );
  }


private:
  EventTypes        m_type;      // MIDI event type
  MetaEventType     m_metaEvent;  // Meta event (mostly unused)
  int8_t            m_channel;    // MIDI channel
  union
  {
    int16_t m_param[2]; // first/second parameter (key/velocity)
    uint8_t m_bytes[4];     // raw bytes
    int32_t m_sysExDataLen; // len of m_sysExData
  } m_data;

  const char* m_sysExData;

} ;

#if 0
/***************************************************
  *****          Midi event object             *****
  ***************************************************/
class Event {
public:
  Event()
    : note(NOTE_INVALID),
      velocity(0) {}

  Event &operator=(const Event &src)
  {
    this->note = src.note;
    this->velocity = src.velocity;
    return *this;
  }

public:
  /** MIDI note */
  Note  note;
  /** Velocity value */
  Velocity velocity;
};

#endif

} // namespace midi

#endif //!defined(MIDI_EVENT_H_)
