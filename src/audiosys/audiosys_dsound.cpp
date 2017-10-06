/** @file
 * Qin - audiosystem - dsound.
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
#include <cstdio>
#include <cstdlib>
#include <windows.h>
#define DIRECTSOUND_VERSION 0x0600
#include <dsound.h>
#include <cmath>

#include "util/error.h"
#include "util/misc.h"
#include "util/fastmem.h"
#include "util/timer.h"
#include "util/log.h"

#include "audiosys/audioformat.h"
#include "audiosys/audiosystem.h"

namespace audiosys {

/*******************************************************************************
*   Macro definitions                                                          *
*******************************************************************************/

/**
@todo use the definitions from the win32 api headers when they define these
*/
#if 1
#define WAVE_FORMAT_IEEE_FLOAT 0x0003
#define WAVE_FORMAT_DOLBY_AC3_SPDIF 0x0092
#define WAVE_FORMAT_EXTENSIBLE 0xFFFE

static const GUID KSDATAFORMAT_SUBTYPE_PCM = {0x1,0x0000,0x0010, {0x80,0x00,0x00,0xaa,0x00,0x38,0x9b,0x71}};

#define SPEAKER_FRONT_LEFT             0x1
#define SPEAKER_FRONT_RIGHT            0x2
#define SPEAKER_FRONT_CENTER           0x4
#define SPEAKER_LOW_FREQUENCY          0x8
#define SPEAKER_BACK_LEFT              0x10
#define SPEAKER_BACK_RIGHT             0x20
#define SPEAKER_FRONT_LEFT_OF_CENTER   0x40
#define SPEAKER_FRONT_RIGHT_OF_CENTER  0x80
#define SPEAKER_BACK_CENTER            0x100
#define SPEAKER_SIDE_LEFT              0x200
#define SPEAKER_SIDE_RIGHT             0x400
#define SPEAKER_TOP_CENTER             0x800
#define SPEAKER_TOP_FRONT_LEFT         0x1000
#define SPEAKER_TOP_FRONT_CENTER       0x2000
#define SPEAKER_TOP_FRONT_RIGHT        0x4000
#define SPEAKER_TOP_BACK_LEFT          0x8000
#define SPEAKER_TOP_BACK_CENTER        0x10000
#define SPEAKER_TOP_BACK_RIGHT         0x20000
#define SPEAKER_RESERVED               0x80000000

#ifndef DSSPEAKER_MONO
# define DSSPEAKER_HEADPHONE         0x00000001
# define DSSPEAKER_MONO              0x00000002
# define DSSPEAKER_QUAD              0x00000003
# define DSSPEAKER_STEREO            0x00000004
# define DSSPEAKER_SURROUND          0x00000005
# define DSSPEAKER_5POINT1           0x00000006
#endif

#ifndef _WAVEFORMATEXTENSIBLE_
typedef struct {
    WAVEFORMATEX    Format;
    union {
        WORD wValidBitsPerSample;       /* bits of precision  */
        WORD wSamplesPerBlock;          /* valid if wBitsPerSample==0 */
        WORD wReserved;                 /* If neither applies, set to zero. */
    } Samples;
    DWORD           dwChannelMask;      /* which channels are */
                                        /* present in stream  */
    GUID            SubFormat;
} WAVEFORMATEXTENSIBLE, *PWAVEFORMATEXTENSIBLE;
#endif

#endif

static const int channel_mask[] = {
  SPEAKER_FRONT_LEFT   | SPEAKER_FRONT_RIGHT  | SPEAKER_LOW_FREQUENCY,
  SPEAKER_FRONT_LEFT   | SPEAKER_FRONT_RIGHT  | SPEAKER_BACK_LEFT    | SPEAKER_BACK_RIGHT,
  SPEAKER_FRONT_LEFT   | SPEAKER_FRONT_RIGHT  | SPEAKER_BACK_LEFT    | SPEAKER_BACK_RIGHT   | SPEAKER_LOW_FREQUENCY,
  SPEAKER_FRONT_LEFT   | SPEAKER_FRONT_CENTER | SPEAKER_FRONT_RIGHT  | SPEAKER_BACK_LEFT    | SPEAKER_BACK_RIGHT     | SPEAKER_LOW_FREQUENCY
};

static HINSTANCE hdsound_dll = NULL;      ///handle to the dll
static LPDIRECTSOUND hds = NULL;          ///direct sound object 
static LPDIRECTSOUNDBUFFER hdspribuf = NULL; ///primary direct sound buffer
static LPDIRECTSOUNDBUFFER hdsbuf = NULL; ///secondary direct sound buffer (stream buffer)
static int buffer_size = 0;               ///size in bytes of the direct sound buffer   
static int write_offset = 0;              ///offset of the write cursor in the direct sound buffer
static int min_free_space = 0;            ///if the free space is below this value get_space() will return 0
                                          ///there will always be at least this amout of free space to prevent
                                          ///get_space() from returning wrong values when buffer is 100% full.
                                          ///will be replaced with nBlockAlign in init()
static int device_num = 0;                ///wanted device number
static GUID device;                       ///guid of the device 

/***************************************************************************************/


/*******************************************************************************
*   Internal functions                                                         *
*******************************************************************************/

static BOOL CALLBACK DirectSoundEnum(LPGUID guid,LPCSTR desc,LPCSTR module,LPVOID context);

////////////////////////////////////////////////////////////////////////////////

static class DSoundImpl : public IAudioOutput {
public:
  const char *getname() const {
    return "Windows DirectSound audio output";
  }
  const char *getshortname() const {
    return "dsound";
  }
  const char *getauthor() const {
    return "Qin2";
  }
  const char *getcomment() const {
    return "none";
  }


  /**
   * handle control commands
   * @param cmd command
   * @param arg argument
   * @return CONTROL_OK or -1 in case the command can't be handled
   */
  int control(int cmd, void *arg, ...)
  {
    LONG volume;
    switch (cmd)
    {
      case AOCONTROL_GET_VOLUME:
        {
          ao_control_vol_t* vol = (ao_control_vol_t*)arg;
          IDirectSoundBuffer_GetVolume(hdsbuf, &volume);
          vol->left = vol->right = pow(10.0, (float)(volume+10000) / 5000.0);
          //printf("ao_dsound: volume: %f\n",vol->left);
          return CONTROL_OK;
        }
      case AOCONTROL_SET_VOLUME:
        {
          ao_control_vol_t* vol = (ao_control_vol_t*)arg;
          volume = (DWORD)(log10(vol->right) * 5000.0) - 10000;
          IDirectSoundBuffer_SetVolume(hdsbuf, volume);
          //printf("ao_dsound: volume: %f\n",vol->left);
          return CONTROL_OK;
        }
    }
    return -1;
  }

  /**
   * Setup sound device
   * @param rate samplerate
   * @param channels number of channels
   * @param format format
   * @param delay the length of buffer counted by time (msec).
   * @param flags unused
   * @return status code.
  */
  int init(int rate, int channels, int format, float delay, int flags)
  {
    int res;
    if (!InitDirectSound()) return 0;

    // ok, now create the buffers
    WAVEFORMATEXTENSIBLE wformat;
    DSBUFFERDESC dsbpridesc;
    DSBUFFERDESC dsbdesc;

    //check if the format is supported in general
    switch(format)
    {
      case AF_FORMAT_AC3:
      case AF_FORMAT_S24_LE:
      case AF_FORMAT_S16_LE:
      case AF_FORMAT_S8:
        break;
      default:
        LOG(VERBOSE) << "as_dsound: format " << af_fmt2str_short(format) << " not supported defaulting to Signed 16-bit Little-Endian.\n";
        format=AF_FORMAT_S16_LE;
    }
    //fill global ao_data
    m_channels = channels;
    m_samplerate = rate;
    m_format = format;
    m_bps = channels * rate * (af_fmt2bits(format)>>3);
    if(m_buffersize==-1) m_buffersize = m_bps * delay / 1000;

    LOG(VERBOSE)
        << "ao_dsound: Samplerate:" << rate << "Hz " << "\n"
        << "Channels:" << channels << "\n"
        << " Format:" << af_fmt2str_short(format) << "\n";
    LOG(VERBOSE)
        << "ao_dsound: Buffersize:" << m_buffersize << " bytes"
        << " (" <<  (float)m_buffersize / (float)m_bps * 1000.0f << " msec)\n";

    //fill waveformatex
    ZeroMemory(&wformat, sizeof(WAVEFORMATEXTENSIBLE));
    wformat.Format.cbSize          = (channels > 2) ? sizeof(WAVEFORMATEXTENSIBLE)-sizeof(WAVEFORMATEX) : 0;
    wformat.Format.nChannels       = channels;
    wformat.Format.nSamplesPerSec  = rate;
    if (format == AF_FORMAT_AC3)
      {
        wformat.Format.wFormatTag      = WAVE_FORMAT_DOLBY_AC3_SPDIF;
        wformat.Format.wBitsPerSample  = 16;
        wformat.Format.nBlockAlign     = 4;
      }
    else
      {
        wformat.Format.wFormatTag      = (channels > 2) ? WAVE_FORMAT_EXTENSIBLE : WAVE_FORMAT_PCM;
        wformat.Format.wBitsPerSample  = af_fmt2bits(format);
        wformat.Format.nBlockAlign     = wformat.Format.nChannels * (wformat.Format.wBitsPerSample >> 3);
      }

    // fill in primary sound buffer descriptor
    memset(&dsbpridesc, 0, sizeof(DSBUFFERDESC));
    dsbpridesc.dwSize = sizeof(DSBUFFERDESC);
    dsbpridesc.dwFlags       = DSBCAPS_PRIMARYBUFFER;
    dsbpridesc.dwBufferBytes = 0;
    dsbpridesc.lpwfxFormat   = NULL;


    // fill in the secondary sound buffer (=stream buffer) descriptor
    memset(&dsbdesc, 0, sizeof(DSBUFFERDESC));
    dsbdesc.dwSize = sizeof(DSBUFFERDESC);
    dsbdesc.dwFlags = DSBCAPS_GETCURRENTPOSITION2 /** Better position accuracy */
                    | DSBCAPS_GLOBALFOCUS         /** Allows background playing */
                    | DSBCAPS_CTRLVOLUME;         /** volume control enabled */

    if (channels > 2)
      {
        wformat.dwChannelMask = channel_mask[channels - 3];
        wformat.SubFormat = KSDATAFORMAT_SUBTYPE_PCM;
        wformat.Samples.wValidBitsPerSample = wformat.Format.wBitsPerSample;
        // Needed for 5.1 on emu101k - shit soundblaster
        dsbdesc.dwFlags |= DSBCAPS_LOCHARDWARE;
      }
    wformat.Format.nAvgBytesPerSec = wformat.Format.nSamplesPerSec * wformat.Format.nBlockAlign;

    dsbdesc.dwBufferBytes = m_buffersize;
    dsbdesc.lpwfxFormat = (WAVEFORMATEX *)&wformat;
    buffer_size = dsbdesc.dwBufferBytes;
    write_offset = 0;
    min_free_space = wformat.Format.nBlockAlign;
    m_outburst = wformat.Format.nBlockAlign * 512;

    // create primary buffer and set its format

    res = IDirectSound_CreateSoundBuffer( hds, &dsbpridesc, &hdspribuf, NULL );
    if ( res != DS_OK )
      {
        UninitDirectSound();
        LOG(ERR)
            << "ao_dsound: cannot create primary buffer ("
            << dserr2str(res) << ")\n";
        return VERR_FAILED;
      }
    res = IDirectSoundBuffer_SetFormat( hdspribuf, (WAVEFORMATEX *)&wformat );
    if ( res != DS_OK )
      {
        LOG(WARNING)
            << "ao_dsound: cannot set primary buffer format ("
            << dserr2str(res) << "), using standard setting (bad quality)\n";
      }

    LOG(VERBOSE)
        << "ao_dsound: primary buffer created\n";

    // now create the stream buffer

    res = IDirectSound_CreateSoundBuffer(hds, &dsbdesc, &hdsbuf, NULL);
    if (res != DS_OK)
      {
        if (dsbdesc.dwFlags & DSBCAPS_LOCHARDWARE)
          {
            // Try without DSBCAPS_LOCHARDWARE
            dsbdesc.dwFlags &= ~DSBCAPS_LOCHARDWARE;
            res = IDirectSound_CreateSoundBuffer(hds, &dsbdesc, &hdsbuf, NULL);
          }
        if (res != DS_OK)
          {
            UninitDirectSound();
            LOG(ERR)
                << "ao_dsound: cannot create secondary (stream)buffer ("
                << dserr2str(res) << ")\n";
            return VERR_FAILED;
          }
      }
    LOG(VERBOSE)
        << "ao_dsound: secondary (stream)buffer created\n";
    return VINF_SUCCEEDED;
  }



  /**
   * stop playing and empty buffers (for seeking/pause)
   */
  int reset(void)
  {
    IDirectSoundBuffer_Stop(hdsbuf);
    // reset directsound buffer
    IDirectSoundBuffer_SetCurrentPosition(hdsbuf, 0);
    write_offset=0;
    return VINF_SUCCEEDED;
  }

  /**
  * stop playing, keep buffers (for pause)
  */
  void pause(void)
  {
    IDirectSoundBuffer_Stop(hdsbuf);
  }

  /**
  * resume playing, after audio_pause()
  */
  void resume(void)
  {
    IDirectSoundBuffer_Play(hdsbuf, 0, 0, DSBPLAY_LOOPING);
  }

  /**
  * Close audio device
  * @param immed stop playback immediately
  */
  int uninit(int immed)
  {
    if(immed)reset();
    else
      {
        DWORD status;
        IDirectSoundBuffer_Play(hdsbuf, 0, 0, 0);
        while(!IDirectSoundBuffer_GetStatus(hdsbuf,&status) && (status&DSBSTATUS_PLAYING))
            usecSleep(20000);
      }
    DestroyBuffer();
    UninitDirectSound();
    return VINF_SUCCEEDED;
  }

  /**
   * Find out how many bytes can be written into the audio buffer without
   * @return free space in bytes, has to return 0 if the buffer is almost full
   */
  int get_space(void)
  {
    int space;
    DWORD play_offset;
    IDirectSoundBuffer_GetCurrentPosition(hdsbuf,&play_offset,NULL);
    space=buffer_size-(write_offset-play_offset);
    //                |                                                      | <-- const --> |                |                 |
    //                buffer start                                           play_cursor     write_cursor     write_offset      buffer end
    // play_cursor is the actual postion of the play cursor
    // write_cursor is the position after which it is assumed to be save to write data
    // write_offset is the postion where we actually write the data to
    if(space > buffer_size)space -= buffer_size; // write_offset < play_offset
    if(space < min_free_space)return 0;
    return space-min_free_space;
  }

  /**
   * Write 'len' bytes of 'data' to internal buffer.
   * @param data pointer to the data to play
   * @param len size in bytes of the data buffer, gets rounded down to outburst*n
   * @param flags currently unused
   * @return number of played bytes.
   */
  int write(void* data, int len, int flags)
  {
    DWORD play_offset;
    int space;

    // make sure we have enough space to write data
    IDirectSoundBuffer_GetCurrentPosition(hdsbuf,&play_offset,NULL);
    space=buffer_size-(write_offset-play_offset);
    if(space > buffer_size)space -= buffer_size; // write_offset < play_offset
    if(space < len) len = space;

    if (!(flags & AOPLAY_FINAL_CHUNK))
    len = (len / m_outburst) * m_outburst;
    return write_buffer((unsigned char*)data, len);
  }

  /**
  * get the delay between the first and last sample in the buffer
  * @return delay in seconds
  */
  float get_delay(void)
  {
    DWORD play_offset;
    int space;
    IDirectSoundBuffer_GetCurrentPosition(hdsbuf,&play_offset,NULL);
    space=play_offset-write_offset;
    if(space <= 0)space += buffer_size;
    return (float)(buffer_size - space) / (float)m_bps;
  }

private:

  /**
   * output error message
   * @param err error code
   * @return string with the error message
   */
  const char * dserr2str(int err)
  {
    switch (err)
    {
      case DS_OK: return "DS_OK";
      case DS_NO_VIRTUALIZATION: return "DS_NO_VIRTUALIZATION";
      case DSERR_ALLOCATED: return "DS_NO_VIRTUALIZATION";
      case DSERR_CONTROLUNAVAIL: return "DSERR_CONTROLUNAVAIL";
      case DSERR_INVALIDPARAM: return "DSERR_INVALIDPARAM";
      case DSERR_INVALIDCALL: return "DSERR_INVALIDCALL";
      case DSERR_GENERIC: return "DSERR_GENERIC";
      case DSERR_PRIOLEVELNEEDED: return "DSERR_PRIOLEVELNEEDED";
      case DSERR_OUTOFMEMORY: return "DSERR_OUTOFMEMORY";
      case DSERR_BADFORMAT: return "DSERR_BADFORMAT";
      case DSERR_UNSUPPORTED: return "DSERR_UNSUPPORTED";
      case DSERR_NODRIVER: return "DSERR_NODRIVER";
      case DSERR_ALREADYINITIALIZED: return "DSERR_ALREADYINITIALIZED";
      case DSERR_NOAGGREGATION: return "DSERR_NOAGGREGATION";
      case DSERR_BUFFERLOST: return "DSERR_BUFFERLOST";
      case DSERR_OTHERAPPHASPRIO: return "DSERR_OTHERAPPHASPRIO";
      case DSERR_UNINITIALIZED: return "DSERR_UNINITIALIZED";
      case DSERR_NOINTERFACE: return "DSERR_NOINTERFACE";
      case DSERR_ACCESSDENIED: return "DSERR_ACCESSDENIED";
      default: return "unknown";
    }
  }

  /**
   * terminate direct sound
   */
  void UninitDirectSound(void)
  {
    // finally release the DirectSound object
    if (hds)
      {
        IDirectSound_Release(hds);
        hds = NULL;
      }
    // free DSOUND.DLL
    if (hdsound_dll)
      {
        FreeLibrary(hdsound_dll);
        hdsound_dll = NULL;
      }
    LOG(VERBOSE)
        << "ao_dsound: DirectSound uninitialized\n";
  }

  /**
   * initiate direct sound
   * @return 0 if error, 1 if ok
   */
  int InitDirectSound(void)
  {
    DSCAPS dscaps;

    // initialize directsound
    typedef HRESULT (WINAPI *OurDirectSoundCreate_t)(LPGUID, LPDIRECTSOUND *, LPUNKNOWN);
    typedef HRESULT (WINAPI *OurDirectSoundEnumerate_t)(LPDSENUMCALLBACKA, LPVOID);
    OurDirectSoundCreate_t OurDirectSoundCreate;
    OurDirectSoundEnumerate_t OurDirectSoundEnumerate;
    int device_index=0;
#if 0
    opt_t subopts[] =
    {
      {"device", OPT_ARG_INT, &device_num,NULL},
      {NULL}
    };
    if (subopt_parse(ao_subdevice, subopts) != 0)
      {
        print_help();
        return 0;
      }
#endif

    hdsound_dll = LoadLibrary("DSOUND.DLL");
    if (hdsound_dll == NULL)
      {
        LOG(ERR)
            << "ao_dsound: cannot load DSOUND.DLL\n";
        return 0;
      }
    OurDirectSoundCreate = (OurDirectSoundCreate_t)GetProcAddress(hdsound_dll, "DirectSoundCreate");
    OurDirectSoundEnumerate = (OurDirectSoundEnumerate_t)GetProcAddress(hdsound_dll, "DirectSoundEnumerateA");

    if (OurDirectSoundCreate == NULL || OurDirectSoundEnumerate == NULL)
      {
        LOG(ERR)
            << "ao_dsound: GetProcAddress FAILED\n";
        FreeLibrary(hdsound_dll);
        return 0;
      }

    // Enumerate all directsound devices
    LOG(VERBOSE)
        << "ao_dsound: Output Devices:";
    OurDirectSoundEnumerate(DirectSoundEnum,&device_index);
    LOG(VERBOSE) << "\n";

    // Create the direct sound object
    if FAILED(OurDirectSoundCreate((device_num)?&device:NULL, &hds, NULL ))
      {
        LOG(ERR)
            << "ao_dsound: cannot create a DirectSound device\n";
        FreeLibrary(hdsound_dll);
        return 0;
      }

    /* Set DirectSound Cooperative level, ie what control we want over Windows
     * sound device. In our case, DSSCL_EXCLUSIVE means that we can modify the
     * settings of the primary buffer, but also that only the sound of our
     * application will be hearable when it will have the focus.
     * !!! (this is not really working as intended yet because to set the
     * cooperative level you need the window handle of your application, and
     * I don't know of any easy way to get it. Especially since we might play
     * sound without any video, and so what window handle should we use ???
     * The hack for now is to use the Desktop window handle - it seems to be
     * working */
    if (IDirectSound_SetCooperativeLevel(hds, GetDesktopWindow(), DSSCL_EXCLUSIVE))
      {
        LOG(ERR)
            << "ao_dsound: cannot set direct sound cooperative level\n";
        IDirectSound_Release(hds);
        FreeLibrary(hdsound_dll);
        return 0;
      }
    LOG(VERBOSE)
        << "ao_dsound: DirectSound initialized\n";

    memset(&dscaps, 0, sizeof(DSCAPS));
    dscaps.dwSize = sizeof(DSCAPS);
    if (DS_OK == IDirectSound_GetCaps(hds, &dscaps))
      {
        if (dscaps.dwFlags & DSCAPS_EMULDRIVER)
          {
            LOG(VERBOSE)
                << "ao_dsound: DirectSound is emulated, waveOut may give better performance\n";
          }
      }
    else
      {
        LOG(VERBOSE)
            << "ao_dsound: cannot get device capabilities\n";
      }

    return 1;
  }

  /**
   * destroy the direct sound buffer
   */
  void DestroyBuffer(void)
  {
    if (hdsbuf)
      {
        IDirectSoundBuffer_Release(hdsbuf);
        hdsbuf = NULL;
      }
    if (hdspribuf)
      {
        IDirectSoundBuffer_Release(hdspribuf);
        hdspribuf = NULL;
      }
  }

  /**
   * fill sound buffer
   * @param data pointer to the sound data to copy
   * @param len length of the data to copy in bytes
   * @return number of copyed bytes
   */
  int write_buffer(unsigned char *data, int len)
  {
    HRESULT res;
    LPVOID lpvPtr1;
    DWORD dwBytes1;
    LPVOID lpvPtr2;
    DWORD dwBytes2;

    // Lock the buffer
    res = IDirectSoundBuffer_Lock(hdsbuf,write_offset, len, &lpvPtr1, &dwBytes1, &lpvPtr2, &dwBytes2, 0);
    // If the buffer was lost, restore and retry lock.
    if (DSERR_BUFFERLOST == res)
    {
      IDirectSoundBuffer_Restore(hdsbuf);
      res = IDirectSoundBuffer_Lock(hdsbuf,write_offset, len, &lpvPtr1, &dwBytes1, &lpvPtr2, &dwBytes2, 0);
    }


    if (SUCCEEDED(res))
    {
      if( (m_channels == 6) && (m_format!=AF_FORMAT_AC3) )
        {
          // reorder channels while writing to pointers.
          // it's this easy because buffer size and len are always
          // aligned to multiples of channels*bytespersample
          // there's probably some room for speed improvements here
          const int chantable[6] = {0, 1, 4, 5, 2, 3}; // reorder "matrix"
          int i, j;
          int numsamp,sampsize;

          sampsize = af_fmt2bits(m_format)>>3; // bytes per sample
          numsamp = dwBytes1 / (m_channels * sampsize);  // number of samples for each channel in this buffer

          for( i = 0; i < numsamp; i++ ) for( j = 0; j < m_channels; j++ )
            {
              memcpy((char*)lpvPtr1+(i*m_channels*sampsize)+(chantable[j]*sampsize),data+(i*m_channels*sampsize)+(j*sampsize),sampsize);
            }

          if (NULL != lpvPtr2 )
            {
              numsamp = dwBytes2 / (m_channels * sampsize);
              for( i = 0; i < numsamp; i++ ) for( j = 0; j < m_channels; j++ )
                {
                  memcpy((char*)lpvPtr2+(i*m_channels*sampsize)+(chantable[j]*sampsize),data+dwBytes1+(i*m_channels*sampsize)+(j*sampsize),sampsize);
                }
            }

          write_offset+=dwBytes1+dwBytes2;
          if(write_offset>=buffer_size)write_offset=dwBytes2;
      }
    else
      {
        // Write to pointers without reordering.
        fast_memcpy(lpvPtr1,data,dwBytes1);
        if (NULL != lpvPtr2 )fast_memcpy(lpvPtr2,data+dwBytes1,dwBytes2);
        write_offset+=dwBytes1+dwBytes2;
        if(write_offset>=buffer_size)write_offset=dwBytes2;
      }

     // Release the data back to DirectSound.
      res = IDirectSoundBuffer_Unlock(hdsbuf,lpvPtr1,dwBytes1,lpvPtr2,dwBytes2);
      if (SUCCEEDED(res))
      {
        // Success.
        DWORD status;
        IDirectSoundBuffer_GetStatus(hdsbuf, &status);
        if (!(status & DSBSTATUS_PLAYING)){
          res = IDirectSoundBuffer_Play(hdsbuf, 0, 0, DSBPLAY_LOOPING);
        }
        return dwBytes1+dwBytes2;
      }
    }
    // Lock, Unlock, or Restore failed.
    return 0;
  }

friend BOOL CALLBACK DirectSoundEnum(LPGUID guid,LPCSTR desc,LPCSTR module,LPVOID context);


} audio_out_dsound_instance;

//
// extern
//
IAudioOutput *audio_out_dsound = static_cast<IAudioOutput*>(&audio_out_dsound_instance);


/**
 * enumerate direct sound devices
 * @return TRUE to continue with the enumeration
*/
static BOOL CALLBACK DirectSoundEnum(LPGUID guid,LPCSTR desc,LPCSTR module,LPVOID context) {
    int* device_index=(int*)context;
    LOG(VERBOSE)
        << *device_index << " " << desc << " ";
    if(device_num==*device_index){
        LOG(VERBOSE) << "<--";
        if(guid){
            memcpy(&device,guid,sizeof(GUID));
        }
    }
    (*device_index)++;
    return TRUE;
}

} // namespace audiosys
