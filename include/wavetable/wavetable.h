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

#ifndef WAVETABLE_H_
#define WAVETABLE_H_

#include <string>
#include <cstdio>

#include "midi/note.h"  // request: midi::Note ; midi::_MAX_NOTE_NUM ; _MAX_POLYPHONY_NUM
#include "midi/event.h" // request: midi::Event
#include "util/list.h"
#include "util/types.h"


namespace wavetable
{

/*
 * Bank name (Only for Qin2)
 */
enum SampleBank {
  BANK_INVALID = 0,
  BANK_SN
};

/***************************************************
  *****          Wave Sample object            *****
  ***************************************************/
class WaveSample {
public:
  WaveSample()
    : m_bank(BANK_INVALID),
      m_note(midi::NOTE_INVALID),
      m_dynamics(0),
      m_size(0),
      m_offset(0),
      m_channels(1),
      m_bps(0),
      m_rate(0),
      m_align(0),
      m_file(0),
      prev(0),
      next(0)
  {}

public:
  /** the bank of this sample */
  SampleBank    m_bank;
  /** node index */
  midi::Note    m_note;
  /** threshold dynamics */
  midi::Velocity m_dynamics;

  std::string m_name;
  std::string m_rawfile;

  /** the number of bytes this wave */
  uint32_t m_size;
  /** the start position of the sample */
  uint32_t m_offset;
  /** the number of channels */
  uint32_t m_channels;
  /** bits per sample */
  uint32_t m_bps;
  /** sampling rate */
  uint32_t m_rate;
  /** sample align */
  uint32_t m_align;
  /** pointer to the file struct */
  FILE    *m_file;

  /** pointer to the previous */
  WaveSample *prev;
  /** pointer to the next */
  WaveSample *next;
};

/*
 * Data to support lazily loading
 */
struct SampleFile {
  /** Filename of this file */
  std::string name;
  /** Pointer to the file struct */
  FILE *fp;
  /** Pointer to the previous */
  struct SampleFile *prev;
  /** Pointer to the next */
  struct SampleFile *next;
};

/*
 * Polyphonic Unit
 */
class PolyUnit {
public:
  PolyUnit()
    : busy(false),
      dynamics(0),
      level(0),
      fp(0),
      len(0),
      remain(0),
      size(0)
  {}
public:
  /** Whether the unit is busy */
  bool           busy;
  /** Current MIDI note */
  midi::Event    event;
  /** The dynamics of sample */
  midi::Velocity dynamics;
  /** Compression level */
  int            level;
  /** Pointer to the current file */
  FILE          *fp;
  /** The the number of bytes that has been read. */
  size_t         len;
  /** Bytes of data remained. */
  size_t         remain;
  /** The total size of current sample */
  size_t         size;

  /** Collection of loaded files. */
  V_LIST<SampleFile> sampleFiles;
};

/***************************************************
  *****          Wave Table object             *****
  ***************************************************/
class WaveTable {
public:
  WaveTable();

  int init();
  int uninit();

  int LoadTimbres(const char *path);
  int SendMIDIEvent(const midi::Event &event, int *poly);
  int GetSampleRate();
  int GetSampleFormat();
  int GetBps();
  int GetChannels();
  int GetPipeChannelNum();
  bool PipeBusy(int index);
  int ReadPipeChannel(int index, void *ori, Sample_t *buff, size_t nsamples);

private:
  int parseSynTable(FILE *fp);

  WaveSample *queryCommonSample();

  SampleBank stringToBank(const char *src);

private:
  V_LIST<WaveSample> m_waveSamples[_MAX_POLYPHONY_NUM][midi::_MAX_NOTE_NUM];
  PolyUnit  m_units[_MAX_POLYPHONY_NUM];

  size_t m_sampleSize;
};

} // namespace wavetable

#endif //!defined(WAVETABLE_H_)
