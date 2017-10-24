/** @file
 * Effector - Filter.
 * In this code we use a butterworth filter to process the samples.
 * and it is all fixed-point operation within the audio pipe.
 *
 * NOTE: See MULT_COEFF, the method above is a temporary settlement as it avoid the
 * FPU fault interrupt caused and at least we will never enter the kernel mode
 * within these simple ALU instructions executing (we assume that there is no
 * other interrupts).
 * On the other hand, the low accuracy will damage the running of filter, so
 * we should do some Error Compensations.
 *
 * If we implement this instrument at the other CPU that has hardware FPU,
 * we should take measure of float instead of keeping using it.
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
#include <cmath>

#include "dsp/effect.h"
#include "util/misc.h"
#include "util/error.h"
#include "util/log.h"
#include "util/types.h"
#include "memory/mmu.h"


#define PI (4.*tan(1.))

/**@def MULT_COEFF
 * this coeff is used for scaling the data to avoid the float
 * operation, so it determines the accuracy of the filter.
 * The larger this coeff is, the more accurate can you get,
 * but it may causes overflow as well.
 */
#define MULT_COEFF (1000)

namespace dsp
{

enum {
  kFreq = 0,
  kQu,
  kGain,
  kClass,
  kMaxCount
};

enum {
  kLowpass = 0,
  kHighpass,
  kBandpass,
  kBandnotch
};

////////////////////////////////////////////////////////////////////////////////

class biquad
{
private:
  int64_t a0, a1, a2, b1, b2;
  int64_t i0, i1, i2;
  int64_t o0, o1, o2;

public:
  biquad()
    : a0(1 * MULT_COEFF), a1(0), a2(0),
      b1(0), b2(0)
  {
    reset();
  }

  void reset()
  {
    // chaining these together to assign 0 easily.
    i0 = i1 = i2 = /*a0*/ o0 = o1 = o2 = 0.0f;
  }

  void setCoeffs(int64_t a0, int64_t a1, int64_t a2, int64_t b1, int64_t b2)
  {
    this->a0 = a0;
    this->a1 = a1;
    this->a2 = a2;
    this->b1 = b1;
    this->b2 = b2;
  }

  inline Sample_t getSample(const Sample_t &src)
  {
    i0 = src;

    //o0 = a0 * i0 + a1 * i1 + a2 * i2;
    //o0 -= b2 * o2 + b1 * o1;

    o0 = a0 * i0 / MULT_COEFF + a1 * i1 / MULT_COEFF + a2 * i2 / MULT_COEFF;
    o0 -= b2 * o2 / MULT_COEFF + b1 * o1 / MULT_COEFF;

    i2 = i1;
    i1 = i0;
    o2 = o1;
    o1 = o0;
    return o0;
  }
};

/**
 * Constructor of FilterImpl.
 * @param rate        sample rate
 * @param channels    number of channels
 */
FilterImpl::FilterImpl(int rate, int channels)
  : IEffector(rate, channels),
    m_inited(false)
{
  /*
   * set up the default values
   */
  m_kFreq = 300.0f;
  m_kGain = 1.0f;
  m_kQu = 1.0f;
  m_kClass = kBandnotch;

  reset();
}

FilterImpl::~FilterImpl()
{
}

const char *FilterImpl::getname() const {
  return "Filter";
}
const char *FilterImpl::getshortname() const {
  return "amp";
}
const char *FilterImpl::getauthor() const {
  return "Qin2";
}
const char *FilterImpl::getcomment() const {
  return "none";
}

//------------------------------------------------------------------------
int
FilterImpl::getParameterCount()
{
  return kMaxCount;
}

//------------------------------------------------------------------------
void
FilterImpl::getParameter(int index, float *v)
{
  switch ( index )
  {
    case kFreq:   *v = m_kFreq;  break;
    case kQu:     *v = m_kQu;    break;
    case kGain:   *v = m_kGain;  break;
    case kClass:  *v = m_kClass; break;
  }
}

//------------------------------------------------------------------------
void
FilterImpl::getParameterName(int index, std::string &s)
{
  switch ( index )
  {
    case kFreq:   s = "Freq"; break;
    case kQu:     s = "Q";    break;
    case kGain:   s = "Gain"; break;
    case kClass:  s = "Class"; break;
  }
}

//------------------------------------------------------------------------
void
FilterImpl::getParameterLable(int index, std::string &s)
{
  switch ( index )
  {
    case kFreq:   s = "Hz"; break;
    case kQu:     s = "Q";  break;
    case kGain:   s = "dB"; break;
    case kClass:  s = "";   break;
  }
}

//------------------------------------------------------------------------
void
FilterImpl::getParameterDisplay(int index, std::string &s)
{
  char buff[12];
  switch ( index )
  {
    case kFreq:   gcvt(m_kFreq, 8, buff);  break;
    case kQu:     gcvt(m_kQu, 8, buff);    break;
    case kGain:   gcvt(m_kGain, 8, buff);  break;
    case kClass:
      {
        switch ((int)m_kClass)
        {
          case kLowpass:
            s = "Low pass"; return;
          case kHighpass:
            s = "High pass"; return;
          case kBandpass:
            s = "Band pass"; return;
          case kBandnotch:
            s = "Band notch"; return;
          default:
            s = "unknown"; return;
        }
      }
  }
  s = buff;
}

//------------------------------------------------------------------------
void
FilterImpl::setParameter(int index, float v)
{
  switch ( index )
  {
    case kFreq:   m_kFreq = v; break;
    case kQu:     m_kQu   = v; break;
    case kGain:   m_kGain = v; break;
    case kClass:  m_kClass = v; break;
  }
  updateParameters();
}

/**
 * Create a new instance of this effector.
 * @reutrn 0 failed.
 * @return !=0 Pointer to the new instance.
 */
IEffector *
FilterImpl::create() const
{
  IEffector *instance = new (/*MEM_TAG_EFFECTOR_INSTANCE,*/ std::nothrow) FilterImpl(m_rate, m_channels);
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
 * Setup the filter effector.
 * @param flags       unused
 * @return status code.
 */
int
FilterImpl::init(int flags)
{
  if (m_channels > _MAX_EFFECT_CHANNELS)
    {
      return VERR_OUT_OF_RANGE;
    }

  for (int i = 0; i < m_channels; i++)
    {
      m_biquads[i] = (void*) new (/*MEM_TAG_EFFECTOR_INSTANCE,*/ std::nothrow) biquad;
      if (!m_biquads[i])
        {
          return VERR_ALLOC_MEMORY;
        }
    }
  updateParameters();

  bypass(false);
  reset();

  m_inited = true;
  return VINF_SUCCEEDED;
}

/**
 * Close filter effector.
 * @param flags unused
 * @return status code.
 */
int
FilterImpl::uninit(int flags)
{
  if (m_inited)
    {
      for (int i = 0; i < m_channels; i++)
        {
          delete static_cast<biquad*>(m_biquads[i]);
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
FilterImpl::reset()
{
  return VINF_SUCCEEDED;
}

/**
 * Recalculate the parameters.
 */
void
FilterImpl::updateParameters()
{
  this->cf = m_kFreq;
  this->q = m_kQu;
  this->g = m_kGain;
  this->sr = m_rate;

  reset();

  float bw = 0.f;
  float c = 0.f;
  float d = 0.f;

  float _a0 = 0.f;
  float _a1 = 0.f;
  float _a2 = 0.f;
  float _b1 = 0.f;
  float _b2 = 0.f;

  /*
   * Calculating the coeffs
   */
  switch((int)m_kClass)
  {
    case (kLowpass):
      {
        c = 1.0f / tan(PI * cf / sr);
        _a0 = 1.0f / (1.0f + sqrt(2) * c + pow(c, 2));
        _a1 = 2 * _a0;
        _a2 = _a0;
        _b1 = 2 * _a0 * (1 - pow(c,2));
        _b2 = _a0 * (1 - sqrt(2) * c + pow(c, 2));
      }
      break;
    case (kHighpass):
      {
        c = tan(PI * cf / sr);
        _a0 = 1.f / (1.f + (sqrt(2) * c) + pow(c, 2));
        _a1 = (2.f * _a0) * - 1;
        _a2 = _a0;
        _b1 = 2.f * _a0 * (pow(c, 2) - 1.f);
        _b2 = _a0 * (1.f - (sqrt(2) * c) + pow(c, 2));
      }
      break;
    case (kBandpass):
      {
        bw = cf / q;
        c = 1.f / tan(PI * (bw / sr));
        d = 2.f * cos(2.f * PI * (cf / sr));
        _a0 = 1.f / (1.f + c);
        _a1 = 0.f;
        _a2 = -1.f * _a0;
        _b1 = (-1.f * _a0) * c * d;
        _b2 = _a0 * (c - 1.f);
      }
      break;
    case (kBandnotch):
      {
        bw = cf / q;
        c = tan(PI * bw / sr);
        d = 2.f * cos(2.f * PI * (cf / sr));
        _a0 = 1.f / (1.f + c);
        _a1 = (-1.f * _a0) * d;
        _a2 = _a0;
        _b1 = (-1.f * _a0) * d;
        _b2 = _a0 * (1.f - c);
      }
      break;

    default:
      return;
  }

  _a0 *= MULT_COEFF;
  _a1 *= MULT_COEFF;
  _a2 *= MULT_COEFF;
  _b1 *= MULT_COEFF;
  _b2 *= MULT_COEFF;

  int32_t a0 = static_cast<int64_t>(_a0);
  int32_t a1 = static_cast<int64_t>(_a1);
  int32_t a2 = static_cast<int64_t>(_a2);
  int32_t b1 = static_cast<int64_t>(_b1);
  int32_t b2 = static_cast<int64_t>(_b2);

#if 1
  LOG(INFO) << "\nfiler params:\n" <<
      "_a0 = " << _a0 << "\n" <<
      "_a1 = " << _a1 << "\n" <<
      "_b1 = " << _b1 << "\n" <<
      "_b2 = " << _b2 << "\n" <<
      "a0 = " << a0 << "\n" <<
      "a1 = " << a1 << "\n" <<
      "b1 = " << b1 << "\n" <<
      "b2 = " << b2 << "\n";
#endif

  for (int i = 0;i < m_channels; i++)
    {
      static_cast<biquad*>(m_biquads[i])->setCoeffs(a0, a1, a2, b1, b2);
    }
}

/*
 * Set the GATE signal.
 * @param gate      true = on, false = off.
 */
void
FilterImpl::gate(bool gate)
{
}

/**
 * Process the audio buffer.
 * @param buff        Pointer to the target buffer.
 * @param nframes     How many frames are there in the target buffer.
 * @return status code.
 */
int
FilterImpl::process(Sample_t *buff, size_t nframes)
{
  register int64_t sample;
  register int n = 0;

  if (m_bypass) return VINF_SUCCEEDED;

  while ( nframes-- )
    {
      for (n = 0; n < m_channels; n++)
        {
          sample = *buff;
          *buff++ = static_cast<biquad*>(m_biquads[n])->getSample(sample);
        }
    }

  return VINF_SUCCEEDED;
}

} // namespace effect
