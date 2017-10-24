/** @file
 * Effector - Amplifier.
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

#include <cmath>

#include "dsp/effect.h"
#include "util/misc.h"
#include "util/error.h"
#include "util/log.h"
#include "util/types.h"
#include "memory/mmu.h"


namespace dsp
{

enum {
  kGain = 0,
  kLine,
  kMaxCount
};

////////////////////////////////////////////////////////////////////////////////

/**
 * Constructor of AmplifierImpl.
 * @param rate        sample rate
 * @param channels    number of channels
 */
AmplifierImpl::AmplifierImpl(int rate, int channels)
  : IEffector(rate, channels),
    m_gainSmpl(0),
    m_line(0)
{
  /*
   * set up the default values
   */
  m_kGain = 0;
  m_kLine = 100;
}

AmplifierImpl::~AmplifierImpl()
{
}

const char *AmplifierImpl::getname() const {
  return "Amplifier";
}
const char *AmplifierImpl::getshortname() const {
  return "amp";
}
const char *AmplifierImpl::getauthor() const {
  return "Qin2";
}
const char *AmplifierImpl::getcomment() const {
  return "none";
}

//------------------------------------------------------------------------
int
AmplifierImpl::getParameterCount()
{
  return kMaxCount;
}

//------------------------------------------------------------------------
void
AmplifierImpl::getParameter(int index, float *v)
{
  switch ( index )
  {
    case kGain:   *v = m_kGain; break;
    case kLine:   *v = m_kLine; break;
  }
}

//------------------------------------------------------------------------
void
AmplifierImpl::getParameterName(int index, std::string &s)
{
  switch ( index )
  {
    case kGain:   s = "Gain"; break;
    case kLine:   s = "Line"; break;
  }
}

//------------------------------------------------------------------------
void
AmplifierImpl::getParameterLable(int index, std::string &s)
{
  switch ( index )
  {
    case kGain:   s = "dB"; break;
    case kLine:   s = "%"; break;
  }
}

//------------------------------------------------------------------------
void
AmplifierImpl::getParameterDisplay(int index, std::string &s)
{
  char buff[12];
  switch ( index )
    {
      case kGain:   gcvt(m_kGain, 8, buff); break;
      case kLine:   gcvt(m_kLine, 8, buff); break;
    }
  s = buff;
}

//------------------------------------------------------------------------
void
AmplifierImpl::setParameter(int index, float v)
{
  switch ( index )
  {
    case kGain:   m_kGain = v; break;
    case kLine:   m_kLine = v; break;
  }
  updateParameters();
}

/**
 * Create a new instance of this effector.
 * @reutrn 0 failed.
 * @return !=0 Pointer to the new instance.
 */
IEffector *
AmplifierImpl::create() const
{
  IEffector *instance = new (/*MEM_TAG_EFFECTOR_INSTANCE,*/ std::nothrow) AmplifierImpl(m_rate, m_channels);
  if (instance)
    {
      if(V_SUCCESS(instance->init(0)))
        {
          return instance;
        }
      delete instance;
    }
  return 0;
}

/**
 * Setup the amplifier effector.
 * @param flags       unused
 * @return status code.
 */
int
AmplifierImpl::init(int flags)
{
  updateParameters();

  bypass(false);

  reset();
  return VINF_SUCCEEDED;
}

/**
 * Close amplifier effector.
 * @param flags unused
 * @return status code.
 */
int
AmplifierImpl::uninit(int flags)
{
  return VINF_SUCCEEDED;
}

/**
 * RESET the effector.
 * @return status code.
 */
int
AmplifierImpl::reset()
{
  return VINF_SUCCEEDED;
}

/**
 * Recalculate the parameters.
 */
void
AmplifierImpl::updateParameters()
{
  //m_kGain = 20 * log10(m_gainSmpl / INT32_MAX);
  //m_gainSmpl = pow(10, (m_kGain / 20)) * INT32_MAX;

  int64_t smpl = m_kGain * (INT32_MAX / 10000);
  if (smpl > INT32_MAX)
    {
      smpl = INT32_MAX;
    }
  m_gainSmpl = smpl;
  m_line = (int)m_kLine;

  LOG(INFO) << "gain smlp = " << m_gainSmpl << "\n";
}

/*
 * Set the GATE signal.
 * @param gate      true = on, false = off.
 */
void
AmplifierImpl::gate(bool gate)
{
}

static inline void
limitSampleLevel(int64_t &dst_sample)
{
  const int64_t max_audioval = ((1<<(sizeof(Sample_t)*8-1))-1);
  const int64_t min_audioval = -(1<<(sizeof(Sample_t)*8-1));

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
}

/**
 * Process the audio buffer.
 * @param buff        Pointer to the target buffer.
 * @param nframes     How many frames are there in the target buffer.
 * @return status code.
 */
int
AmplifierImpl::process(Sample_t *buff, size_t nframes)
{
  register int64_t sample;
  register int n = 0;

  if (m_bypass) return VINF_SUCCEEDED;

  while ( nframes-- )
    {
      for (n = 0; n < m_channels; n++)
        {
          sample = *buff;
          /*
           * Gain
           */
          sample += sample > 0 ? sample + m_gainSmpl : sample - m_gainSmpl;
          limitSampleLevel(sample);

          /*
           * Line
           */
          sample = sample * m_line / 100;
          limitSampleLevel(sample);

          *buff++ = sample;
        }
    }
  return VINF_SUCCEEDED;
}

} // namespace effect
