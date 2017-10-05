/** @file
 * Qin - Mixer.
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

#include "util/misc.h"
#include "util/types.h"
#include "util/error.h"
#include "util/log.h"

#include "audiosys/audioformat.h"

#include "mixer/mixer.h"

////////////////////////////////////////////////////////////////////////////////

namespace mixer {


/* The volume ranges from 0 - 128 */
#define ADJUST_VOLUME(s, v)	(s = (s*v)/MIXER_MAXVOLUME)
#define ADJUST_VOLUME_U8(s, v)	(s = (((s-128)*v)/MIXER_MAXVOLUME)+128)

/**
 * Mix the two audio stream.
 * @param dst       Pointer to the target buffer.
 * @param src       Pointer to the source buffer.
 * @param nsamples  The number of samples.
 * @param volume    The volume the mixer limited.
 * @return status code.
 */
int
Mixer::MixAudio (Sample_t *dst, const Sample_t *src, uint32_t nsamples, int volume)
{
  if ( volume == 0 )
    {
      return VERR_FAILED;
    }

  int64_t src1, src2;
  int64_t dst_sample;
  const int64_t max_audioval = ((1<<(sizeof(Sample_t)*8-1))-1);
  const int64_t min_audioval = -(1<<(sizeof(Sample_t)*8-1));

  while ( nsamples-- )
    {
      /*
       * synth
       */
      src1 = *src;
      ADJUST_VOLUME(src1, volume);
      src2 = *dst;
      src++;
      dst_sample = src1 + src2;

      /*
       * clip ?
       */
      if ( dst_sample > max_audioval )
        {
          dst_sample = max_audioval;
        }
      else
      if ( dst_sample < min_audioval )
        {
          dst_sample = min_audioval;
        }

      *dst = dst_sample;
      dst++;
    }

  return VINF_SUCCEEDED;
}

} // namespace mixer


