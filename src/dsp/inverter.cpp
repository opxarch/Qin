/** @file
 * Effector - Inverter.
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

#include "dsp/effect.h"
#include "util/misc.h"
#include "util/error.h"
#include "util/log.h"
#include "util/types.h"
#include "memory/mmu.h"


namespace dsp
{

enum {
  kInvert = 0,
  kMaxCount
};

////////////////////////////////////////////////////////////////////////////////

/**
 * Constructor of InverterImpl.
 * @param rate        sample rate
 * @param channels    number of channels
 */
InverterImpl::InverterImpl(int rate, int channels)
  : IEffector(rate, channels),
    m_invert(false)
{
  /*
   * set up the default values
   */
  m_kInvert = 0.5;
}

InverterImpl::~InverterImpl()
{
}

const char *InverterImpl::getname() const {
  return "Inverter";
}
const char *InverterImpl::getshortname() const {
  return "inverter";
}
const char *InverterImpl::getauthor() const {
  return "Qin2";
}
const char *InverterImpl::getcomment() const {
  return "none";
}

//------------------------------------------------------------------------
int
InverterImpl::getParameterCount()
{
  return kMaxCount;
}

//------------------------------------------------------------------------
void
InverterImpl::getParameter(int index, float *v)
{
  switch ( index )
  {
    case kInvert:   *v = m_kInvert;  break;
  }
}

//------------------------------------------------------------------------
void
InverterImpl::getParameterName(int index, std::string &s)
{
  switch ( index )
  {
    case kInvert:   s = "Invert";  break;
  }
}

//------------------------------------------------------------------------
void
InverterImpl::getParameterLable(int index, std::string &s)
{
  switch ( index )
  {
    case kInvert:   s = "";   break;
  }
}

//------------------------------------------------------------------------
void
InverterImpl::getParameterDisplay(int index, std::string &s)
{
  switch ( index )
  {
    case kInvert:
      {
        s = m_kInvert < 0.5 ? "off" : "on";
      }
  }
}

//------------------------------------------------------------------------
void
InverterImpl::setParameter(int index, float v)
{
  switch ( index )
  {
    case kInvert:   m_kInvert = v;  break;
  }

  updateParameters();
}

/**
 * Create a new instance of this effector.
 * @reutrn 0 failed.
 * @return !=0 Pointer to the new instance.
 */
IEffector *
InverterImpl::create() const
{
  IEffector *instance = new (/*MEM_TAG_EFFECTOR_INSTANCE,*/ std::nothrow) InverterImpl(m_rate, m_channels);
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
 * Setup the inverter effector.
 * @param flags       unused
 * @return status code.
 */
int
InverterImpl::init(int flags)
{
  updateParameters();

  bypass(false);

  reset();
  return VINF_SUCCEEDED;
}

/**
 * Close inverter effector.
 * @param flags unused
 * @return status code.
 */
int
InverterImpl::uninit(int flags)
{
  return VINF_SUCCEEDED;
}

/**
 * RESET the effector.
 * @return status code.
 */
int
InverterImpl::reset()
{
  return VINF_SUCCEEDED;
}

/**
 * Recalculate the parameters.
 */
void
InverterImpl::updateParameters()
{
  if (m_channels != 2)
    {
      m_invert = false;
      return;
    }
  m_invert = m_kInvert < 0.5 ? false : true;
}

/*
 * Set the GATE signal.
 * @param gate      true = on, false = off.
 */
void
InverterImpl::gate(bool gate)
{
}

/**
 * Process the audio buffer.
 * @param buff        Pointer to the target buffer.
 * @param nframes     How many frames are there in the target buffer.
 * @return status code.
 */
int
InverterImpl::process(Sample_t *buff, size_t nframes)
{
  register Sample_t sample;

  if (m_bypass || !m_invert)
    return VINF_SUCCEEDED;

  V_ASSERT(m_channels == 2);

  while ( nframes-- )
    {
      /*
       * Swap the channels, only make sense when we have 2 chs.
       */
      sample = *buff;
      *buff = *(buff+1); buff++; // never = *(++buff) there
      *buff++ = sample;
    }
  return VINF_SUCCEEDED;
}

} // namespace effect
