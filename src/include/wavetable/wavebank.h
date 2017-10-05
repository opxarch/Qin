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

#ifndef WAVETABLE_WAVEBANK_H_
#define WAVETABLE_WAVEBANK_H_

/**
 * Header of the wave bank file
 */
typedef struct WaveBank_s
{
  /** Magic number of this file. */
  char magic[4];
  /** Reserved fields. */
  int reserved[8];
} WaveBank_t;

#endif //!defined(WAVETABLE_WAVEBANK_H_)
