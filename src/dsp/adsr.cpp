/** @file
 * Effector - ADSR (Attack Decay Sustain Release) envelope generator.
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

#include "dsp/effect.h"
#include "util/misc.h"
#include "util/error.h"
#include "util/log.h"
#include "util/types.h"
#include "memory/mmu.h"


#define ADSR_MAXVOLUME (256)

namespace dsp
{

enum {
  kAttack = 0,
  kDecay,
  kSustain,
  kRelease,
  kMaxCount
};

////////////////////////////////////////////////////////////////////////////////

/**
 * Constructor of ADSRImpl.
 * @param rate        sample rate
 * @param channels    number of channels
 */
ADSRImpl::ADSRImpl(int rate, int channels)
  : IEffector(rate, channels),
    m_attackRate(0),
    m_decayRate(0),
    m_releaseRate(0),
    m_sustainLevel(0),
    m_accum(0),
    m_output(0),
    m_state(env_idle)
{
  /*
   * set up the default values
   */
  m_kAttack = 500;
  m_kDecay = 2000;
  m_kRelease = 30;
  m_kSustain = 88;
}

ADSRImpl::~ADSRImpl()
{
}

const char *ADSRImpl::getname() const {
  return "ADSR (Attack Decay Sustain Release) envelope generator";
}
const char *ADSRImpl::getshortname() const {
  return "adsr";
}
const char *ADSRImpl::getauthor() const {
  return "Qin2";
}
const char *ADSRImpl::getcomment() const {
  return "none";
}

//------------------------------------------------------------------------
int
ADSRImpl::getParameterCount()
{
  return kMaxCount;
}

//------------------------------------------------------------------------
void
ADSRImpl::getParameter(int index, float *v)
{
  switch ( index )
  {
    case kAttack:   *v = m_kAttack; break;
    case kDecay:    *v = m_kDecay;  break;
    case kSustain:  *v = m_kSustain; break;
    case kRelease:  *v = m_kRelease; break;
  }
}

//------------------------------------------------------------------------
void
ADSRImpl::getParameterName(int index, std::string &s)
{
  switch ( index )
  {
    case kAttack:   s = "Attack time"; break;
    case kDecay:    s = "Decay time";  break;
    case kSustain:  s = "Sustain level"; break;
    case kRelease:  s = "Release time"; break;
  }
}

//------------------------------------------------------------------------
void
ADSRImpl::getParameterLable(int index, std::string &s)
{
  switch ( index )
  {
    case kAttack:   s = "ms";   break;
    case kDecay:    s = "ms";   break;
    case kSustain:  s = "%";    break;
    case kRelease:  s = "ms";   break;
  }
}

//------------------------------------------------------------------------
void
ADSRImpl::getParameterDisplay(int index, std::string &s)
{
  char buff[12];
  switch ( index )
  {
    case kAttack:   gcvt(m_kAttack, 8, buff);   break;
    case kDecay:    gcvt(m_kDecay, 8, buff);    break;
    case kSustain:  gcvt(m_kSustain, 8, buff);  break;
    case kRelease:  gcvt(m_kRelease, 8, buff);  break;
  }
  s = buff;
}

//------------------------------------------------------------------------
void
ADSRImpl::setParameter(int index, float v)
{
  switch ( index )
  {
    case kAttack:   m_kAttack = v; break;
    case kDecay:    m_kDecay = v;  break;
    case kSustain:  m_kSustain = v; break;
    case kRelease:  m_kRelease = v; break;
  }

  updateParameters();
}

/**
 * Create a new instance of this effector.
 * @reutrn 0 failed.
 * @return !=0 Pointer to the new instance.
 */
IEffector *
ADSRImpl::create() const
{
  IEffector *instance = new (/*MEM_TAG_EFFECTOR_INSTANCE,*/ std::nothrow) ADSRImpl(m_rate, m_channels);
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
 * Setup the ADSR effector.
 * @param flags       unused
 * @return status code.
 */
int
ADSRImpl::init(int flags)
{
  updateParameters();

  bypass(false);

#if 1
  static bool printed = false;
  if (!printed)
    {
      LOG(INFO) << "\n" <<
          "Attack = " << m_attackRate << " s/s\n" <<
          "Decay = " << m_decayRate << " s/s\n" <<
          "Release = " << m_releaseRate << " s/s\n" <<
          "Sustain = " << m_sustainLevel << "\n";
      printed = true;
    }
#endif

  reset();
  return VINF_SUCCEEDED;
}

/**
 * Close ADSR effector.
 * @param flags unused
 * @return status code.
 */
int
ADSRImpl::uninit(int flags)
{
  return VINF_SUCCEEDED;
}

/**
 * RESET the effector.
 * @return status code.
 */
int
ADSRImpl::reset()
{
  m_state = env_idle;
  m_accum = 0;
  m_output = 0;
  return VINF_SUCCEEDED;
}

/**
 * Recalculate the parameters.
 */
void
ADSRImpl::updateParameters()
{
  m_attackRate  = m_kAttack * m_rate / 1000 / ADSR_MAXVOLUME;
  m_decayRate   = m_kDecay * m_rate / 1000 / ADSR_MAXVOLUME;
  m_releaseRate = m_kRelease * m_rate / 1000 / ADSR_MAXVOLUME;
  m_sustainLevel = m_kSustain * ADSR_MAXVOLUME / 100;
}

inline int
ADSRImpl::genenv()
{
  /*
   * State machine of envelope generator
   */
  switch (m_state)
  {
    case env_idle:
      break;

    //
    // Attack stage
    //
    case env_attack:
      {
        if (m_accum < m_attackRate)
          break;
        else
          m_accum = 0;

        /* inner processing */
        m_output++;
        if (m_output >= ADSR_MAXVOLUME)
          {
            m_output = ADSR_MAXVOLUME;
            m_state = env_decay;
          }
      }
      break;

    //
    // Decay stage
    //
    case env_decay:
      {
        if (m_accum < m_decayRate)
          break;
        else
          m_accum = 0;

        /* inner processing */
        m_output--;
        if (m_output <= (int)m_sustainLevel)
          {
            m_output = m_sustainLevel;
            m_state = env_sustain;
          }
      }
      break;

    //
    // Sustain stage (just keep the level)
    //
    case env_sustain:
      break;

    //
    // Release stage
    //
    case env_release:
      {
        if (m_accum < m_releaseRate)
          break;
        else
          m_accum = 0;

        /* inner processing */
        m_output--;
        if (m_output <= 0)
          {
            m_output = 0;
            m_state = env_idle;
          }
      }
  }
  return m_output;
}

/*
 * Set the GATE signal.
 * @param gate      true = on, false = off.
 */
void
ADSRImpl::gate(bool gate)
{
  if (gate)
    m_state = env_attack;
  else if (m_state != env_idle)
    m_state = env_release;
}

/**
 * Process the audio buffer.
 * @param buff        Pointer to the target buffer.
 * @param nframes     How many frames are there in the target buffer.
 * @return status code.
 */
int
ADSRImpl::process(Sample_t *buff, size_t nframes)
{
  register int64_t sample;
  register int n = 0;

  if (m_bypass) return VINF_SUCCEEDED;

  while ( nframes-- )
    {
      accum();
      for (n = 0; n < m_channels; n++)
        {
          sample = *buff;
          sample = sample * genenv() / ADSR_MAXVOLUME;
          *buff++ = sample;
        }
    }
  return VINF_SUCCEEDED;
}

} // namespace effect
