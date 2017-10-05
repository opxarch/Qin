/** @file
 * Qin - Wave table.
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
#include <cstdio>
#include <cstring>
#include <cmath>

#include "util/misc.h"
#include "util/types.h"
#include "util/error.h"
#include "util/log.h"
#include "util/string.h"
#include "util/bswap.h"

#include "audiosys/audioformat.h"

#include "midi/note.h" // request: stringToNote()
#include "midi/mapping.h" // request: mapNote()

#include "wavetable/wavebank.h"
#include "wavetable/wavetable.h"

#define SAMPLE_TABLE_HEADER "QIN SAMPLE TABLE 1\n"

#define MAX_LEVEL (128)

// to adjust level
#define COMP_LEVEL(src, lev) (src = (src)*(lev) / 128)

////////////////////////////////////////////////////////////////////////////////

namespace wavetable {

WaveTable::WaveTable()
  : m_sampleSize(0)
{
}

/**
 * Do the logical initialization.
 * @return status code.
 */
int
WaveTable::init()
{
  return VINF_SUCCEEDED;
}

int
WaveTable::uninit()
{
  return VINF_SUCCEEDED;
}

/**
 * Load the timbre samples.
 * @param path      Pointer to the string indicates the path
 *                  of timbre sample table file.
 * @return status code.
 */
int
WaveTable::LoadTimbres(const char *path)
{
  int rc;
  FILE *fp = fopen(path, "r");
  if (!fp)
    {
      return VERR_OPEN_FILE;
    }

  /*
   * Parse the sample table
   */
  rc = parseSynTable(fp);

  fclose(fp);

  m_sampleSize = queryCommonSample()->m_bps / 8;

  return rc;
}

/**
 * Inner, parse the syn table.
 * @param fp        Pointer to the file stream.
 * @return status code.
 */
int
WaveTable::parseSynTable(FILE *fp)
{
  int rc;
  size_t len = 0;
  char header[sizeof(SAMPLE_TABLE_HEADER)];

  /*
   * Match the header
   */
  len = fread(header, sizeof(header)-2, 1, fp);
  if (len != 1)
    {
      return VERR_READING_FILE;
    }
  if (strncmp(header, SAMPLE_TABLE_HEADER, sizeof(SAMPLE_TABLE_HEADER)-2) != 0)
    {
      return VERR_INVALID_FORMAT;
    }
  if (fgetc(fp) != '\n')
    {
      return VERR_INVALID_FORMAT;
    }

  size_t pos = 0;
  char buf[2048];
  char ch;

  int index = 0;
  std::string note;
  std::string name;
  std::string rawfile;
  std::string bank;
  std::string dynamics;
  std::string channels;
  std::string size;
  std::string bps;
  std::string rate;
  std::string tag;
  std::string align;

  std::string *dst;

  midi::Note _note = midi::NOTE_INVALID;
  SampleBank _bank = BANK_INVALID;
  std::string _rawfile;
  int _dynamics = 0;
  int _channels = 0;
  int _size = 0;
  int _bps = 0;
  int _rate = 0;
  int _align = 0;

  uint32_t _offset = sizeof(WaveBank_t);

  /*
   * Read the each line
   */
  for (;;)
    {
      ch = fgetc(fp);
      if (feof(fp))
        {
          break;
        }
      else if (ferror(fp))
        {
          return VERR_READING_FILE;
        }

      /*
       * input based state machine
       */
      switch (ch)
      {
        /*
         * Fetch a token word
         */
        case ' ':
          if (pos)
            {
              switch(index++)
              {
                case 0: dst = &note;    break;
                case 1: dst = &name;    break;
                case 2: dst = &rawfile; break;
                case 3: dst = &bank;    break;
                case 4: dst = &dynamics; break;
                case 5: dst = &size;     break;
                case 6: dst = &channels; break;
                case 7: dst = &bps;     break;
                case 8: dst = &rate;    break;
                case 9: dst = &tag;     break;
                case 10: dst = &align;   break;
                default:
                  return VERR_FAILED;
              }

              dst->assign(buf, pos);
              trim(*dst);

              if (dst->empty())
                {
                  return VERR_INVALID_FORMAT;
                }
            }
          pos = 0;
          continue;

        /*
         * Read a new line
         */
        case '\n':
          {
            /*
             * check the number of parameters
             */
            if (index != 11)
              {
                return VERR_INVALID_FORMAT;
              }

            /*
             * convert index or paths
             */
            _note = midi::stringToNote(note.c_str());
            _bank = stringToBank(bank.c_str());
            _rawfile = CONF_SAMPLE_PATH;
            _rawfile += rawfile;

            /*
             * atoi
             */
            rc = stringToInteger(size.c_str(), (int)size.length(), &_size);             UPDATE_RC(rc);
            rc = stringToInteger(dynamics.c_str(), (int)dynamics.length(), &_dynamics);  UPDATE_RC(rc);
            rc = stringToInteger(channels.c_str(), (int)channels.length(), &_channels);  UPDATE_RC(rc);
            rc = stringToInteger(bps.c_str(), (int)bps.length(), &_bps);                UPDATE_RC(rc);
            rc = stringToInteger(rate.c_str(), (int)rate.length(), &_rate);             UPDATE_RC(rc);
            rc = stringToInteger(align.c_str(), (int)align.length(), &_align);          UPDATE_RC(rc);

            /*
             * Validate the parameters and apply them.
             */
            if ((_note > midi::NOTE_INVALID && _note < midi::_MAX_NOTE_NUM) &&
                (_bank != BANK_INVALID) &&
                (_dynamics >= 0 && _dynamics <=128) &&
                (_size > 0) &&
                (_channels > 0) &&
                (_bps >= 8) &&
                (_rate >= 44100) &&
                (_align > 0))
              {
                /*
                 * Store the mapping information for each polyphonic unit.
                 */
                for (int nPoly = 0; nPoly < _MAX_POLYPHONY_NUM; nPoly++)
                  {
                    /*
                     * Open the sample file. We made use of lazy loading
                     * as we are in single-thread design. Each note element
                     * in the same polyphonic unit shared with the same sample
                     * file buffer.
                     */
                    FILE *fp = 0;
                    for (SampleFile *sf = m_units[nPoly].sampleFiles.root; sf; sf = sf->next)
                      {
                        if (sf->name.compare(rawfile /*(1)*/) == 0)
                          {
                            fp = sf->fp; // so the requested has been opened before.
                            break;
                          }
                      }

                    if (!fp)
                      {
                        fp = fopen(_rawfile.c_str(), "rb");
                        if (!fp)
                          {
                            LOG(ERR) << "failed on loading sample: " << rawfile << "\n";
                            return VERR_OPEN_FILE;
                          }
                        /*
                         * once opened the file, we will cache the fp so that
                         * the other note units will be able to reused it.
                         */
                        SampleFile *nsf = new (std::nothrow) SampleFile;
                        if (!nsf)
                          {
                            return VERR_ALLOC_MEMORY;
                          }
                        nsf->name = rawfile; /*(1)*/
                        nsf->fp = fp;
                        rc = m_units[nPoly].sampleFiles.push(nsf);
                        UPDATE_RC(rc);
                      }

                    WaveSample *ws = new (std::nothrow) WaveSample;
                    if (!ws)
                      {
                        return VERR_ALLOC_MEMORY;
                      }
                    ws->m_note      = _note;
                    ws->m_name      = name;
                    ws->m_rawfile   = rawfile;
                    ws->m_bank      = _bank;
                    ws->m_dynamics  = _dynamics;
                    ws->m_size      = _size;
                    ws->m_offset    = _offset;
                    ws->m_channels  = _channels;
                    ws->m_bps       = _bps;
                    ws->m_rate      = _rate;
                    ws->m_align     = _align;
                    ws->m_file      = fp;

                    rc = m_waveSamples[nPoly][_note].push(ws);
                    UPDATE_RC(rc);
                  }

                // pointer to the next wave
                _offset += _size;

                LOG(INFO) << "sample loaded: " << name << "\n";
              }
            else
              {
                return VERR_INVALID_DATA;
              }

            pos = 0;
            index = 0;
          }
          break;

        /*
         * Store to the buffer
         */
        default:
          if (pos < sizeof(buf)-2)
            {
              buf[pos++] = ch;
            }
          else
            /* if this happens, FIXME! */
            return VERR_BUFFER_OVERFLOW;
      }
    }

  //fgets(buf, sizeof(buf), fp);

  return VINF_SUCCEEDED;
}

/*
 * Send a MIDI event to the polyphonic unit.
 * @param event         Reference of the source event.
 * @return status code.
 */
int
WaveTable::SendMIDIEvent(const midi::Event &event)
{
  /*
   * Filter the MIDI events
   */
  switch (event.type())
  {
    case midi::NoteOff:
      return VINF_SUCCEEDED;
    case midi::NoteOn:
      break;
    case midi::PitchBend:
      return VINF_SUCCEEDED;
    default:
      return VINF_SUCCEEDED;
  }

  /*
   * First of all, Dispatching the units
   */
  int nPoly;
  for (nPoly = 0; nPoly < _MAX_POLYPHONY_NUM; nPoly++)
    {
      if (!m_units[nPoly].busy) break;
    }
  /*
   * There is no free unit available, so we have to
   * force the most suitable unit to stall.
   */
  if (nPoly == _MAX_POLYPHONY_NUM)
    {
      nPoly = 0;
      /*
       * Find the minimum remain
       */
      size_t min = m_units[0].remain;
      for (int n = 1; n < _MAX_POLYPHONY_NUM; n++)
        {
          if (m_units[n].remain < min)
            {
              min = m_units[n].remain;
              nPoly = n;
            }
        }
    }

  midi::Note note = midi::mapNote(event.key());

  if (note == midi::NOTE_INVALID)
    {
      return VINF_SUCCEEDED;
    }

  V_ASSERT(note > midi::NOTE_INVALID && note < midi::_MAX_NOTE_NUM);

  /*
   * Match the bank and map the velocity to dynamics.
   */
  int dV, Vm = 128;
  WaveSample *ws = 0, *m = 0;
  for (WaveSample *w = m_waveSamples[nPoly][note].root; w; w = w->next)
    {
      ws = w;
      dV = abs(event.velocity() - w->m_dynamics);
      if (dV < Vm)
        {
          m = w;
          Vm = dV;
        }
    }
  if (m) ws = m;

  /*
   * Work out the compression level according to the velocity
   * of source midi event.
   */
  dV = event.velocity() - ws->m_dynamics;
  int level = MAX_LEVEL + dV;

  /*
   * Prepare the file stream, this is a slow procedure as it
   * will cause heavily disk accessing.
   */
  fseek(ws->m_file, ws->m_offset, SEEK_SET);

  /*
   * Post the event
   */
  m_units[nPoly].busy = true;
  m_units[nPoly].event = event;
  m_units[nPoly].dynamics = ws->m_dynamics;
  m_units[nPoly].level = level;
  m_units[nPoly].fp = ws->m_file;
  m_units[nPoly].len = 0;
  m_units[nPoly].remain = ws->m_size;
  m_units[nPoly].size = ws->m_size;

  LOG(INFO) << "sample:" << ws->m_name << " velocity = " << (uint32_t)ws->m_dynamics << " level = " << level << "\n";

  return VINF_SUCCEEDED;
}


WaveSample *
WaveTable::queryCommonSample()
{
  for (int i = 0; i < _MAX_POLYPHONY_NUM; i++)
    {
      for (int j = 0; j < midi::_MAX_NOTE_NUM; j++)
        {
          if (m_waveSamples[i][j].root)
            {
              return m_waveSamples[i][j].root;
            }
        }
    }
  return 0;
}

int
WaveTable::GetSampleRate()
{
  WaveSample *ws = queryCommonSample();
  return ws ? ws->m_rate : 0;
}

int
WaveTable::GetSampleFormat()
{
#if 0
  int bps = GetBps();
  switch (bps)
  {
    case 8:  return AF_FORMAT_S8;
    case 16: return AF_FORMAT_S16_LE;
    case 24: return AF_FORMAT_S24_LE;
    default:
      {
        LOG(WARNING) << "unknown sample format: bps = " << bps << "\n";
        return AF_FORMAT_S16_LE;
      }
  }
#endif
  return AF_FORMAT_S32_LE;
}

int
WaveTable::GetBps()
{
#if 0
  WaveSample *ws = queryCommonSample();
  return ws ? ws->m_bps : 0;
#endif
  return 32;
}

int
WaveTable::GetChannels()
{
  WaveSample *ws = queryCommonSample();
  return ws ? ws->m_channels : 0;
}


/******************************************************************************/
/* BEGIN - KEY AUDIO PIPE */
/******************************************************************************/

/**
 * Get the number of audio channels.
 * @return the result
 */
int
WaveTable::GetPipeChannelNum()
{
  return _MAX_POLYPHONY_NUM;
}

/**
 * Read the data from a audio pipe.
 * @param index         The index of target pipe.
 * @param ori           Where to store the original sample data.
 *                      The bits of each of the sample is specified
 *                      by GetBps().
 * @param buff          Where to store the unified sample data, the
 *                      buffer should have enough space to be filled in.
 * @param nsamples      The count of samples you want to read. It should
 *                      be the multiple of sizeof(Sample_t), otherwise
 *                      it will cause something unforeseen.
 * @return status code.
 */
int
WaveTable::ReadPipeChannel(int index, void *ori, Sample_t *buff, size_t nsamples)
{
  int64_t remain = 0;

  V_ASSERT(index >= 0 && index < _MAX_POLYPHONY_NUM);
  PolyUnit *unit = &m_units[index];

  if (unit->busy)
    {
      V_ASSERT(unit->fp && unit->size);

      /*
       * Work out the length of original data
       */
      size_t len = nsamples * m_sampleSize;

      remain = (int64_t)len - (int64_t)unit->remain;
      if (remain > 0)
        {
          len = unit->remain;
        }

      /*
       * Read the sample from the file. The following
       * procedure is a slow one. Should we consider a
       * extra special cache there ?
       */
      size_t rd = fread(ori, 1,len, unit->fp);
      if (rd != len || ferror(unit->fp))
        {
          return VERR_READING_FILE;
        }
      if (remain > 0)
        {
          memset((char*)ori + rd, 0, remain);
        }

      int level = unit->level;

      size_t n = nsamples;
      uint8_t *src = reinterpret_cast<uint8_t *>(ori);
      uint8_t *dst = reinterpret_cast<uint8_t *>(buff);

      register int64_t sample;

      /*
       * Convert the original data into unified samples.
       * This could be optimized by vector asm.
       */
      switch (m_sampleSize)
      {
        case 1: // 8bit
          {
            while ( n-- )
              {
#if HAVE(BIGENDIAN)
                *dst++ = *src++;
                *dst++ = 0;
                *dst++ = 0;
                *dst++ = 0;
#else
                *dst++ = 0;
                *dst++ = 0;
                *dst++ = 0;
                *dst++ = *src++;
#endif
                sample = *buff;
                COMP_LEVEL(sample, level);
                *buff++ = sample;
              }
          }
          break;
        case 2: // 16bit
          {
            while ( n-- )
              {
#if HAVE(BIGENDIAN)
                *dst++ = src[1];
                *dst++ = src[0];
                *dst++ = 0;
                *dst++ = 0;
                src += 2;
#else
                *dst++ = 0;
                *dst++ = 0;
                *dst++ = *src++;
                *dst++ = *src++;
#endif
                sample = *buff;
                COMP_LEVEL(sample, level);
                *buff++ = sample;
              }
          }
          break;
        case 3: // 24bit
          {
            while ( n-- )
              {
#if HAVE(BIGENDIAN)
                *dst++ = src[2];
                *dst++ = src[1];
                *dst++ = src[0];
                *dst++ = 0;
                src += 3;
#else
                *dst++ = 0;
                *dst++ = *src++;
                *dst++ = *src++;
                *dst++ = *src++;
#endif
                sample = *buff;
                COMP_LEVEL(sample, level);
                *buff++ = sample;
              }
          }
          break;

        case 4: // 32bit
        {
          memcpy(buff, ori, rd);
        }
        break;

        default: /* if this happens, FIXME! */
          V_ASSERT(0);
          return VERR_FAILED;
      }

      /*
       * Refresh the status of this unit.
       */
      unit->len += rd;
      unit->remain -= rd;
      if ((int64_t)(unit->remain) <= 0)
        {
          unit->busy = false;
        }
    }
  else
    {
      /*
       * Fill the buffer with silence zeros.
       */
      memset(buff, 0, nsamples * sizeof(Sample_t));
    }

  /* succeeded! */
  return VINF_SUCCEEDED;
}

/******************************************************************************/
/* END - KEY AUDIO PIPE */
/******************************************************************************/


/*
 * Convert a string to bank index.
 * @param src               Source string.
 * @return BANK_INVALID     The source string is invalid.
 * @return bank index if succeeded.
 */
SampleBank
WaveTable::stringToBank(const char *src)
{
  if (strcmp(src, "sn") == 0)
    {
      return BANK_SN;
    }
  else
    {
      return BANK_INVALID;
    }
}

} // namespace wavetable
