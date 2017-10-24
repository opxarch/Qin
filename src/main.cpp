/** @file
 * Qin - Errors management.
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
#include <cstring>
#include <ctime>
#include "util/error.h"
#include "util/log.h"
#include "util/timer.h"

#include "memory/mmu.h"
#include "wavetable/wavetable.h"
#include "mixer/mixer.h"
#include "audiosys/audiosystem.h"
#include "dsp/effect.h"
#include "midi/ports.h"
#include "midi/message.h"
#include "midi/mapping.h"
#include "mididev/mididev.h"

#define DEBUG_LEVEL 0

////////////////////////////////////////////////////////////////////////////////


int main(int argc, char *argv[])
{
  int rc;
  wavetable::WaveTable      *wavetable;
  mixer::Mixer              *mixer;
  audiosys::AudioSystem     *audiosys;
  dsp::Effectors            *effects;
  midi::Ports               *ports;
  mididev::MidiDev          *mdev;

  rc = AllocReservedMem();
  if (V_FAILURE(rc))
    {
      LOG(ERR) << "failed on allocating the necessary initialization memory.\n";
      return 1;
    }

  wavetable = new wavetable::WaveTable;
  mixer     = new mixer::Mixer;
  audiosys  = new audiosys::AudioSystem;
  effects   = new dsp::Effectors;
  ports     = new midi::Ports;
  mdev      = new mididev::MidiDev;

  /*
   * initiate the wave table
   */
  rc = wavetable->init();
  if (V_SUCCESS(rc))
    {
      rc = wavetable->LoadTimbres("./../samples/qin2.syntab");
      if (V_SUCCESS(rc))
        {
          LOG(INFO) << "successed.\n";

          int bps = wavetable->GetBps();
          int rate = wavetable->GetSampleRate();
          int channels = wavetable->GetChannels();
          int format = wavetable->GetSampleFormat();
          int samplesize = bps / 8;
          float delay = 50.0f; //ms
          int oriFormat = format;
          bool needResample = false;

          /*
           * Validate the format data
           */
          if (!(
              (bps > 0) &&
              (rate >= 44100) &&
              (channels > 0)))
            {
              LOG(ERR) << "failed on reading the format data of samples.\n";
            }

          /*
           * Try to open the audio system.
           */
          audiosys::IAudioOutput *ao = 0;
          rc = audiosys->initDevice(&ao, "dsound", rate, channels, format, delay, 0);

          /*
           * We have failed on it as the format was not supported.
           * Use the default format and try again. In that case, we should do
           * the re-sample to fit in the requirement of interface.
           */
          if (V_FAILURE(rc) || rc == VINF_REPLACED)
            {
              format = AF_FORMAT_S16_LE;
              needResample = true;
              rc = audiosys->initDevice(&ao, "dsound", rate, channels, format, delay, 0);
            }

          if (V_FAILURE(rc))
            {
              LOG(ERR) << "failed on Initialize the audio system.\n";
              return 1;
            }

          if (ao->m_format != format)
            {
              needResample = true;
            }
          if (needResample)
            {
              LOG(WARNING) << "The required audio format is not supported by your device, so we will do the re-sample to fit in, but it's harmful of the sound quality.\n";
            }

          LOG(INFO) << "AUDIO: '" << ao->getname() << "'\n";

          /*
           * Sync the parameters with audio device.
           */
          bps       = ao->m_bps;
          rate      = ao->m_samplerate;
          channels  = ao->m_channels;
          format    = ao->m_format;
          samplesize = bps / 8;


          /*
           * initiate the MIDI devices
           */
          rc = mdev->openDevices();
          if (V_FAILURE(rc))
            {
              LOG(ERR) << "failed on opening MIDI devices.\n";
              return 1;
            }

          mididev::IMidiController *controller = mdev->current();

          LOG(INFO) << "\nMIDI devices:\n";

          std::string nctrl;
          for (int n = 0; n < controller->getInputNum(); n++)
            {
              rc = controller->getInputName(n, nctrl);
              if (V_SUCCESS(rc))
                LOG(INFO) << "# MIDI Input: " << nctrl << "\n";
            }

          controller->addInputPort(ports);
#if 0
          /*
           * Generate the test square.
           */
          midi::Event event;
          event.setType(midi::NoteOn);

          event.setKey(midi::mapKey(midi::NOTE_D1));
          event.setVelocity(80);

          rc = wavetable->SendMIDIEvent(event);
          LOG(INFO) << "sended a midi with rc = " << GetErrorMsg(rc)->msgDefine << "\n";

          if (V_SUCCESS(rc))
            {
              event.setVelocity(70);
              for (int j = 0; j< 15; j++)
                {
                  event.setKey(midi::mapKey((midi::Note)(j + 1)));
                  wavetable->SendMIDIEvent(event);
                }
            }
#endif

          if (V_SUCCESS(rc))
            {
              int lw = 0;
              size_t burst;
              size_t outn;
              size_t outlen;
              size_t newlen;
              size_t total;
              uint8_t *buf;
              uint8_t *ori;
              Sample_t *samples;
              Sample_t *samples2;

              size_t padsize = 0;
              size_t targetsize = 0;
              double audio_delay = 0;
              double audio_clock = 0;

              size_t a_out_buffer_len = 0;
              size_t a_out_buffer_size = 2048 + MAX_OUTBURST;
              unsigned char *a_out_buffer = 0;

              buf = new (MEM_TAG_AUDIO_BUFFER, std::nothrow) uint8_t[a_out_buffer_size];
              ori = new (MEM_TAG_AUDIO_BUFFER, std::nothrow) uint8_t[a_out_buffer_size];
              samples = new (MEM_TAG_AUDIO_BUFFER, std::nothrow) Sample_t[a_out_buffer_size];
              samples2 = new (MEM_TAG_AUDIO_BUFFER, std::nothrow) Sample_t[a_out_buffer_size];

              if (!buf || !ori || !samples || !samples2)
                {
                  return VERR_ALLOC_MEMORY;
                }

              std::memset(buf, 0, a_out_buffer_size);
              std::memset(samples, 0, a_out_buffer_size);

              a_out_buffer = needResample ? buf : (uint8_t*)samples;

              int polySum = wavetable->GetPipeChannelNum();

              /*
               * Create effectors.
               */
              rc = effects->add(
                  dsp::EFFECT_SCOPE_GROUP,
                  dsp::ADSRImpl(rate, channels));

              if (V_FAILURE(rc))
                {
                  LOG(ERR) << "failed on setup group insert effectors.\n";
                  return 1;
                }

              //rc = effects->add(effect::EFFECT_SCOPE_INSTRUMENT, effect::AmplifierImpl(rate, channels));
              //rc = effects->add(effect::EFFECT_SCOPE_INSTRUMENT, effect::FilterImpl(rate, channels));
              //rc = effects->add(effect::EFFECT_SCOPE_INSTRUMENT, effect::DelayImpl(rate, channels));
              //rc = effects->add(effect::EFFECT_SCOPE_INSTRUMENT, effect::InverterImpl(rate, channels));

              if (V_FAILURE(rc))
                {
                  LOG(ERR) << "failed on setup instrument insert effectors.\n";
                  return 1;
                }

              for (int j=0;j < 16; j++) {
              effects->groupBypass(j, true);
              effects->groupGate(j, true);
              }

              midi::Event event;
#if DEBUG_LEVEL > 1
              std::string msg;
#endif
              int eventPoly;

              bool end = false;


              /******************************************************************************/
              /* BEGIN - KEY AUDIO PIPE */
              /******************************************************************************/

              FILE *dump = std::fopen("dump.pcm", "wb");
              if (!dump) { LOG(ERR) << "error\n"; return 1; }

              for (;!end;)
                {
                  targetsize = ao->get_space();

                  while (targetsize)
                    {
                      /*
                       * Fetch the MIDI event from the queue, if there is a
                       * event available, then give it to the wave table.
                       */
                      rc = ports->queuePop(event);
                      if (V_SUCCESS(rc))
                        {
#if DEBUG_LEVEL > 1
                          formatMidiMessage(msg, event);
                          LOG(INFO) << msg << "\n";
#endif
                          rc = wavetable->SendMIDIEvent(event, &eventPoly);
                          if (V_FAILURE(rc))
                            {
                              LOG(ERR) << "send MIDI event.\n";
                              return 1;
                            }

                          // send it to effectors
                          effects->groupGate(eventPoly, true);
                        }

                      padsize = (targetsize > MAX_OUTBURST)?MAX_OUTBURST:targetsize;
                      targetsize -= padsize;

                      /* Fill buffer if needed */
                      while (a_out_buffer_len < padsize)
                        {
                          size_t buflen = a_out_buffer_len;

                          /*
                           * Ensure that all the addresses is aligned at
                           * 4bytes bound, or we will got wrong result.
                           */
                          V_ASSERT(buflen % sizeof(Sample_t) == 0);

                          /*
                           * Resolving the writing interval
                           */
                          int min = padsize - buflen;
                          int max = a_out_buffer_size - buflen;

                          outlen = channels * sizeof(Sample_t);
                          outlen = (min + outlen - 1) / outlen * outlen;
                          if ((int)outlen > max)
                            {
                              LOG(ERR) << "buffer overflow\n";
                              return 1;
                            }
                          outn = outlen / sizeof(Sample_t);
//clock_t clk0, clk1;
//clk0 = clock();
                          /*
                           * Rendering the audio data from each channel,
                           * Specifically, processing the 1st channel to fill in
                           * the initial data.
                           */
                          rc = wavetable->ReadPipeChannel(0, ori, samples, outn);
                      if (V_SUCCESS(rc))
                        {
                          rc = effects->processGroup(0, samples, outn, channels);

                          if (V_SUCCESS(rc))
                            {
                              for (int nPoly = polySum -1; nPoly; nPoly--)
                                {
                                  if (!wavetable->PipeBusy(nPoly))
                                    continue;

                                  rc = wavetable->ReadPipeChannel(nPoly, ori, samples2, outn);
                                  if (V_FAILURE(rc)) break;

                                  rc = effects->processGroup(nPoly, samples2, outn, channels);
                                  if (V_FAILURE(rc))
                                    {
                                      LOG(ERR) << "Process the group insert effectors.\n";
                                      return 1;
                                    }

                                  rc = mixer->MixAudio(samples, samples2, outn, MIXER_MAXVOLUME);
                                  if (V_FAILURE(rc))
                                    {
                                      LOG(ERR) << "failed on mixing the audio.\n";
                                      return 1;
                                    }
                                }
                            }
                        }

                          /*
                           * The audio is at the end?
                           */
                          if (V_FAILURE(rc))
                            {
                              while(ao->get_delay() > .04);

                              a_out_buffer_len = 0;
                              targetsize = 0;
                              end = true;

                              LOG(INFO) << "Audio output truncated at end.\n";
                              break;
                            }

                          /*
                           * Apply the instrument insert effectors.
                           */
                          rc = effects->processInstrument(samples, outn, channels);
                          if (V_FAILURE(rc))
                            {
                              return 1;
                            }


                          if (needResample)
                            {
                              rc = mixer->Resample_S16LE(samples, &buf[buflen], outn, &newlen, oriFormat);

                              a_out_buffer_len += newlen;
                            }
                          else
                            {
                              rc = mixer->Resample_S32LE(samples, &buf[buflen], outn);

                              a_out_buffer_len += outlen;
                            }
                          if (V_FAILURE(rc))
                            {
                              LOG(ERR) << "failed on re-sampling.\n";
                              return 1;
                            }
//clk1 = clock();
//if (clk1 - clk0)
//  LOG(INFO) << "max process time = " << clk1 - clk0 << " ms.\n";

                        }  // while

                      if (padsize > a_out_buffer_len)
                        {
                          padsize = a_out_buffer_len;
                        }
                      if (!padsize) break;

                      /*
                       * Write audio device
                       */
                      padsize = ao->write(a_out_buffer, padsize, 0);
                      //fwrite(a_out_buffer, 1, padsize, dump);

                      if (padsize > 0)
                        {
                          a_out_buffer_len -= padsize;
                          std::memmove(a_out_buffer,
                                  &a_out_buffer[padsize],
                                  a_out_buffer_len);

                          audio_delay = ao->get_delay();
                          audio_clock += (double)padsize / bps;
#if 0
                          std::cout << "\rCLOCK: " << audio_clock - audio_delay << " sec, DELAY: " << audio_delay << " sec";
#endif
                        }
                      else
                        {
                          //LOG(ERR) << "failed on writing audio.\n";
                          //return 0;
                        }

                    } // while(total)

                } // for(;;)

              /******************************************************************************/
              /* END - KEY AUDIO PIPE */
              /******************************************************************************/

            }
        }
      else
        LOG(ERR) << "failed on loading samples.\n";
    }
  else
    LOG(ERR) << "failed on initiate the wavetable.\n";

  LOG(ERR) << "rc = " << GetErrorMsg(rc)->msgDefine << "\n";

  LOG(INFO) << "current memory: " << GetBytesMemAllocated() / 1024 << " KBytes.\n";

  ReleaseReservedMem();

  return 0;
}
