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

#ifndef MIXER_H_
#define MIXER_H_

#include "util/types.h"

namespace mixer {

#define MIXER_MAXVOLUME (128)

/***************************************************
  *****                Mixer                   *****
  ***************************************************/
class Mixer {
public:
  static int MixAudio (Sample_t *dst, const Sample_t *src, uint32_t nsamples, int volume);
  static int Resample_S16LE (const Sample_t *src, uint8_t *dst, uint32_t nsamples, uint32_t *newlen, int format);
  static int Resample_S32LE (const Sample_t *src, uint8_t *dst, uint32_t nsamples);
};

} // namespace mixer

#endif //!defined(MIXER_H_)
