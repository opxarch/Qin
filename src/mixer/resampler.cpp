/** @file
 * Qin - Re-sampler.
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

#include <cstring>

#include "util/misc.h"
#include "util/types.h"
#include "util/error.h"
#include "util/log.h"
#include "util/bswap.h"

#include "audiosys/audioformat.h"

#include "mixer/mixer.h"

////////////////////////////////////////////////////////////////////////////////

namespace mixer {

/**
 * Resample the audio stream to 16bit signed LE.
 * @param src       Pointer to the source samples.
 * @param dst       Pointer to the target buffer.
 * @param nsamples  The count of source samples.
 * @param newlen    Where to store the new buffer length in bytes.
 * @param format    The format of source audio data.
 * @return status code.
 */
int
Mixer::Resample_S16LE (const Sample_t *src0, uint8_t *dst, uint32_t nsamples, uint32_t *newlen, int format)
{
  const uint8_t *src = reinterpret_cast<const uint8_t *>(src0);

  *newlen = nsamples * 2;

  switch (format)
  {
    case AF_FORMAT_S24_LE:
      {
        while ( nsamples-- )
          {
#if HAVE(BIGENDIAN)
            dst[1] = src++;
            dst[0] = src++;
            src++;
            dst += 2;
#else
            src++;
            *dst++ = *src++;
            *dst++ = *src++;
#endif
          }
        return VINF_SUCCEEDED;
      }

    case AF_FORMAT_S32_LE:
      {
        while ( nsamples-- )
          {
#if HAVE(BIGENDIAN)
            dst[1] = src++;
            dst[0] = src++;
            src++;
            src++;
            dst += 2;
#else
            src++;
            src++;
            *dst++ = *src++;
            *dst++ = *src++;
#endif
          }
        return VINF_SUCCEEDED;
      }

    default: /* If this happens... FIXME! */
      LOG(ERR) << "Resample_S16LE(): unknown audio format";
  }

  *newlen = 0;
  return VERR_FAILED;
}

int
Mixer::Resample_S32LE (const Sample_t *src, uint8_t *dst0, uint32_t nsamples)
{
#if HAVE(BIGENDIAN)
  Sample_t *dst = reinterpret_cast<Sample_t *>(dst0);
  while ( nsamples-- )
    {
      *dst++ = be2le32(*src++);
    }
#else
  memcpy(dst0, src, nsamples * sizeof(Sample_t));
#endif
  return VINF_SUCCEEDED;
}

} // namespace mixer
