/** @file
 * Qin - audiosystem - NULL.
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
#include <cstdlib>
#include <sys/time.h>

#include "util/error.h"
#include "util/misc.h"

#include "audiosys/audioformat.h"
#include "audiosys/audiosystem.h"

namespace audiosys {


static class NULLImpl : public IAudioOutput {
public:
    const char *getname() const {
        return "Null audio output";
    }
    const char *getshortname() const {
        return "null";
    }
    const char *getauthor() const {
        return "OpenWSP";
    }
    const char *getcomment() const {
        return "none";
    }

    // to set/get/query special features/parameters
    int control(int cmd,void *arg, ...) {
        return VERR_FAILED;
    }

    // open & setup audio device
    // return: 1=success 0=fail
    int init(int rate,int channels,int format, float delay, int flags){

        m_buffersize= 65536;
        m_outburst=1024;
        m_channels=channels;
        m_samplerate=rate;
        m_format=format;
        m_bps=channels*rate;
        if (format != AF_FORMAT_U8 && format != AF_FORMAT_S8)
        m_bps*=2;
        buffer=0;
        gettimeofday(&last_tv, 0);

        return VINF_SUCCEEDED;
    }

    // close audio device
    int uninit(int flags){
        return VINF_SUCCEEDED;
    }

    // stop playing and empty buffers (for seeking/pause)
    int reset(void){
        buffer=0;
        return VINF_SUCCEEDED;
    }
    
    // stop playing, keep buffers (for pause)
    void pause(void) {
        // for now, just call reset();
        reset();
    }

    // resume playing, after audio_pause()
    void resume(void) {
    }

    // return: how many bytes can be played without blocking
    int get_space(void){
        drain();
        return m_buffersize - buffer;
    }

    // plays 'len' bytes of 'data'
    // it should round it down to outburst*n
    // return: number of bytes played
    int write(void* data,int len,int flags){
        int maxbursts = (m_buffersize - buffer) / m_outburst;
        int playbursts = len / m_outburst;
        int bursts = playbursts > maxbursts ? maxbursts : playbursts;
        buffer += bursts * m_outburst;
        return bursts * m_outburst;
    }

    // return: delay in seconds between first and last sample in buffer
    float get_delay(void){
        drain();
        return (float) buffer / (float) m_bps;
    }

private:
    struct timeval last_tv;
    int buffer;

private:
    void drain(void){
        struct timeval now_tv;
        int temp, temp2;

        gettimeofday(&now_tv, 0);
        temp = now_tv.tv_sec - last_tv.tv_sec;
        temp *= m_bps;

        temp2 = now_tv.tv_usec - last_tv.tv_usec;
        temp2 /= 1000;
        temp2 *= m_bps;
        temp2 /= 1000;
        temp += temp2;

        buffer-=temp;
        if (buffer<0) buffer=0;

        if(temp>0) last_tv = now_tv;//audio is fast
    }

} audio_out_null_instance;

//
// extern
//
IAudioOutput *audio_out_null = static_cast<IAudioOutput*>(&audio_out_null_instance);

} // namespace audiosys
