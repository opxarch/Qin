/** @file
 * Effector - Delay.
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

#include "dsp/effect.h"
#include "util/misc.h"
#include "util/error.h"
#include "util/log.h"
#include "util/types.h"
#include "memory/mmu.h"


#define RING_BUFFER_LENGTH 192000
#define MAX_DELAY_SAMPLES 192000
#define MAX_FEEDBACK 1
#define MAX_DELAY_LEVEL 1

#define MULT_COEFF 100

namespace dsp
{

enum {
  kSamples = 0,
  kLevel,
  kFeedback,
  kMaxCount
};

////////////////////////////////////////////////////////////////////////////////

/**
 * Constructor of DelayImpl.
 * @param rate        sample rate
 * @param channels    number of channels
 */
DelayImpl::DelayImpl(int rate, int channels)
  : IEffector(rate, channels),
    m_inited(false),
    m_delaySamples(0),
    m_delayLevel(0),
    m_feedback(0)
{
  /*
   * set up the default values
   */
  m_kSamples = 50000;
  m_kLevel = 0.5;
  m_kFeedback = 0.5;
}

DelayImpl::~DelayImpl()
{
}

const char *DelayImpl::getname() const {
  return "Delay";
}
const char *DelayImpl::getshortname() const {
  return "delay";
}
const char *DelayImpl::getauthor() const {
  return "Qin2";
}
const char *DelayImpl::getcomment() const {
  return "none";
}

//------------------------------------------------------------------------
int
DelayImpl::getParameterCount()
{
  return kMaxCount;
}

//------------------------------------------------------------------------
void
DelayImpl::getParameter(int index, float *v)
{
  switch ( index )
  {
    case kSamples:  *v = m_kSamples;    break;
    case kLevel:    *v = m_kLevel;      break;
    case kFeedback: *v = m_kFeedback;   break;
  }
}

//------------------------------------------------------------------------
void
DelayImpl::getParameterName(int index, std::string &s)
{
  switch ( index )
  {
    case kSamples:  s = "Samples";    break;
    case kLevel:    s = "Level";      break;
    case kFeedback: s = "Feedback";   break;
  }
}

//------------------------------------------------------------------------
void
DelayImpl::getParameterLable(int index, std::string &s)
{
  switch ( index )
  {
    case kSamples:  s = "smpl";    break;
    case kLevel:    s = "dB";      break;
    case kFeedback: s = "dB";   break;
  }
}

//------------------------------------------------------------------------
void
DelayImpl::getParameterDisplay(int index, std::string &s)
{
  char buff[12];
  switch ( index )
  {
    case kSamples:  gcvt(m_kSamples, 8, buff);  break;
    case kLevel:    gcvt(m_kLevel, 8, buff);    break;
    case kFeedback: gcvt(m_kFeedback, 8, buff); break;
  }
  s = buff;
}

//------------------------------------------------------------------------
void
DelayImpl::setParameter(int index, float v)
{
  switch ( index )
  {
    case kSamples:  m_kSamples = v;    break;
    case kLevel:    m_kLevel = v;      break;
    case kFeedback: m_kFeedback = v;   break;
  }

  updateParameters();
}

/**
 * Create a new instance of this effector.
 * @reutrn 0 failed.
 * @return !=0 Pointer to the new instance.
 */
IEffector *
DelayImpl::create() const
{
  IEffector *instance = new (/*MEM_TAG_EFFECTOR_INSTANCE,*/ std::nothrow) DelayImpl(m_rate, m_channels);
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
 * Setup the delay effector.
 * @param flags       unused
 * @return status code.
 */
int
DelayImpl::init(int flags)
{
  if (m_channels > _MAX_EFFECT_CHANNELS)
    {
      return VERR_OUT_OF_RANGE;
    }
  for (int i = 0; i < m_channels; i++)
    {
      m_ringBuffs[i].buffer = new (MEM_TAG_EFFECTOR_BUFFER, std::nothrow) Sample_t[RING_BUFFER_LENGTH];
      if (!m_ringBuffs[i].buffer)
        {
          return VERR_ALLOC_MEMORY;
        }
      m_ringBuffs[i].index = 0;
    }

  updateParameters();

  bypass(false);
  reset();

  m_inited = true;
  return VINF_SUCCEEDED;
}

/**
 * Close ADSR effector.
 * @param flags unused
 * @return status code.
 */
int
DelayImpl::uninit(int flags)
{
  if (m_inited)
    {
      for (int i = 0; i < m_channels; i++)
        {
          delete [] m_ringBuffs[i].buffer;
        }
      m_inited = false;
    }
  return VINF_SUCCEEDED;
}

/**
 * RESET the effector.
 * @return status code.
 */
int
DelayImpl::reset()
{
  for (int i = 0; i < m_channels; i++)
    {
      m_ringBuffs[i].index = 0;
      std::memset(m_ringBuffs[i].buffer, 0, RING_BUFFER_LENGTH * sizeof(Sample_t));
    }
  return VINF_SUCCEEDED;
}

/**
 * Recalculate the parameters.
 */
void
DelayImpl::updateParameters()
{
  /*
   * Validate the parameters
   */
  if (m_kSamples <= 0 || m_kSamples > MAX_DELAY_SAMPLES)
    m_kSamples = MAX_DELAY_SAMPLES;
  if (m_kLevel <= 0 || m_kLevel > MAX_DELAY_LEVEL)
    m_delayLevel = MAX_DELAY_LEVEL;
  if (m_kFeedback <= 0 || m_kFeedback > MAX_FEEDBACK)
    m_feedback = MAX_FEEDBACK;

  /*
   * Calculating parameters
   */
  m_delaySamples  = m_kSamples;
  m_delayLevel = m_kLevel * MULT_COEFF;
  m_feedback = m_kFeedback * MULT_COEFF;
}


/*
 * Set the GATE signal.
 * @param gate      true = on, false = off.
 */
void
DelayImpl::gate(bool gate)
{
}

/**
 * Process the audio buffer.
 * @param buff        Pointer to the target buffer.
 * @param nframes     How many frames are there in the target buffer.
 * @return status code.
 */
int
DelayImpl::process(Sample_t *buff, size_t nframes)
{
  register int64_t  sample;
  register int      n = 0;
  register int64_t  delayed_sample;
  int               cursor;
  ringBuffer        *rb;
  
  if (m_bypass) return VINF_SUCCEEDED;

  
  while ( nframes-- )
    {
      for (n = 0; n < m_channels; n++)
        {
          sample = *buff;

          rb = &m_ringBuffs[n];

          cursor = ((rb->index - m_delaySamples + RING_BUFFER_LENGTH) % RING_BUFFER_LENGTH);
          //LOG(VERBOSE) << "cursor = " << cursor << "\n";
          delayed_sample = (rb->buffer)[cursor];

          /*
           * write current sample to buffer
           */
          (rb->buffer)[rb->index] = sample + (delayed_sample * m_feedback / MULT_COEFF);

          /*
           * Calculate + write output sample
           */
          *buff++ = sample + (m_delayLevel * delayed_sample / MULT_COEFF);

          /*
           * Increment buffer index
           */
          rb->index = (rb->index+1) % RING_BUFFER_LENGTH;
        }
    }

  return VINF_SUCCEEDED;
}

} // namespace effect
