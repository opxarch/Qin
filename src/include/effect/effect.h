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

#ifndef EFFECT_H_
#define EFFECT_H_

#include "util/types.h"
#include "util/list.h"
#include "audiosys/audioformat.h"

#include "midi/note.h" // request: _MAX_POLYPHONY_NUM

namespace effect {

/***************************************************
  *****         Interface of Effector          *****
  ***************************************************/

class IEffector {
public:
  IEffector(int rate, int channels)
    : m_rate(rate),
      m_channels(channels),
      m_bypass(false),
      prev(0),
      next(0)
  {}
  virtual ~IEffector() {};

  virtual IEffector *create() const=0;

  virtual int init(int flags)=0;
  virtual int uninit(int flags)=0;
  virtual int reset()=0;
  virtual int process(Sample_t *buff[], size_t nsamples)=0;

  virtual void bypass(bool y)
  {
    m_bypass = y;
  }

  virtual void gate(bool gate)=0;

  virtual int getParameterCount()=0;
  virtual void getParameter(int index, float *valout)=0;
  virtual void getParameterName(int index, std::string &s)=0;
  virtual void getParameterLable(int index, std::string &s)=0;
  virtual void getParameterDisplay(int index, std::string &s)=0;
  virtual void setParameter(int index, float val)=0;

  virtual const char *getname() const=0;
  virtual const char *getshortname() const=0;
  virtual const char *getauthor() const=0;
  virtual const char *getcomment() const=0;

public:
  int           m_rate;
  int           m_channels;
  bool          m_bypass;
  IEffector    *prev;
  IEffector    *next;
};

/*
 * Set up the max channels supported
 */
#define _MAX_EFFECT_CHANNELS (16)


/***************************************************
  *****             ADSR Class                 *****
  ***************************************************/

#define ADSR_MAXVOLUME (256)

class ADSRImpl : public IEffector {
public:
  ADSRImpl(int rate, int channels);
  ~ADSRImpl();

  IEffector *create() const;

  int init(int flags);
  int uninit(int flags);
  int reset();
  int process(Sample_t *buff[], size_t nsamples);
  void gate(bool gate);

  int getParameterCount();
  void getParameter(int index, float *v);
  void getParameterName(int index, std::string &s);
  void getParameterLable(int index, std::string &s);
  void getParameterDisplay(int index, std::string &s);
  void setParameter(int index, float v);

  const char *getname() const;
  const char *getshortname() const;
  const char *getauthor() const;
  const char *getcomment() const;

private:
  void updateParameters();

  inline void
  accum()
  {
    m_accum++;
  }

  inline int genenv();

private:
  float         m_kAttack;
  float         m_kDecay;
  float         m_kSustain;
  float         m_kRelease;

  uint32_t      m_attackRate;
  uint32_t      m_decayRate;
  uint32_t      m_releaseRate;
  uint32_t      m_sustainLevel;

  uint32_t      m_accum;
  int32_t       m_output;

  enum envState {
      env_idle = 0,
      env_attack,
      env_decay,
      env_sustain,
      env_release
  };

  envState      m_state;

};

/***************************************************
  *****            Amplifier Class             *****
  ***************************************************/

class AmplifierImpl : public IEffector {
public:
  AmplifierImpl(int rate, int channels);
  ~AmplifierImpl();

  IEffector *create() const;

  int init(int flags);
  int uninit(int flags);
  int reset();
  int process(Sample_t *buff[], size_t nsamples);
  void gate(bool gate);

  int getParameterCount();
  void getParameter(int index, float *v);
  void getParameterName(int index, std::string &s);
  void getParameterLable(int index, std::string &s);
  void getParameterDisplay(int index, std::string &s);
  void setParameter(int index, float v);

  const char *getname() const;
  const char *getshortname() const;
  const char *getauthor() const;
  const char *getcomment() const;

private:
  void updateParameters();

private:
  float         m_kGain;
  float         m_kLine;

  int           m_gainSmpl;
  int           m_line;
};


/***************************************************
  *****             Filter Class               *****
  ***************************************************/

class FilterImpl : public IEffector {
public:
  FilterImpl(int rate, int channels);
  ~FilterImpl();

  IEffector *create() const;

  int init(int flags);
  int uninit(int flags);
  int reset();
  int process(Sample_t *buff[], size_t nsamples);
  void gate(bool gate);

  int getParameterCount();
  void getParameter(int index, float *v);
  void getParameterName(int index, std::string &s);
  void getParameterLable(int index, std::string &s);
  void getParameterDisplay(int index, std::string &s);
  void setParameter(int index, float v);

  const char *getname() const;
  const char *getshortname() const;
  const char *getauthor() const;
  const char *getcomment() const;

private:
  void updateParameters();

private:
  bool          m_inited;
  float         m_kFreq;
  float         m_kQu;
  float         m_kGain;
  float         m_kClass;

  // parameters of filter

  /** sample rate */
  float sr;
  /** cutoff frequency */
  float cf;
  /** Q factor */
  float q;
  /** gain */
  float g;

  void *m_biquads[_MAX_EFFECT_CHANNELS];
};


/***************************************************
  *****             Delay Class                *****
  ***************************************************/

class DelayImpl : public IEffector {
public:
  DelayImpl(int rate, int channels);
  ~DelayImpl();

  IEffector *create() const;

  int init(int flags);
  int uninit(int flags);
  int reset();
  int process(Sample_t *buff[], size_t nsamples);
  void gate(bool gate);

  int getParameterCount();
  void getParameter(int index, float *v);
  void getParameterName(int index, std::string &s);
  void getParameterLable(int index, std::string &s);
  void getParameterDisplay(int index, std::string &s);
  void setParameter(int index, float v);

  const char *getname() const;
  const char *getshortname() const;
  const char *getauthor() const;
  const char *getcomment() const;

private:
  void updateParameters();

private:
  bool          m_inited;
  float         m_kSamples;
  float         m_kLevel;
  float         m_kFeedback;

  // parameters of delay

  struct ringBuffer
  {
    int index;
    Sample_t *buffer;
  };

  ringBuffer m_ringBuffs[_MAX_EFFECT_CHANNELS];

  int32_t       m_delaySamples;
  int32_t       m_delayLevel;
  int32_t       m_feedback;
};


/***************************************************
  *****            Inverter Class              *****
  ***************************************************/

class InverterImpl : public IEffector {
public:
  InverterImpl(int rate, int channels);
  ~InverterImpl();

  IEffector *create() const;

  int init(int flags);
  int uninit(int flags);
  int reset();
  int process(Sample_t *buff[], size_t nsamples);
  void gate(bool gate);

  int getParameterCount();
  void getParameter(int index, float *v);
  void getParameterName(int index, std::string &s);
  void getParameterLable(int index, std::string &s);
  void getParameterDisplay(int index, std::string &s);
  void setParameter(int index, float v);

  const char *getname() const;
  const char *getshortname() const;
  const char *getauthor() const;
  const char *getcomment() const;

private:
  void updateParameters();

private:
  float         m_kInvert;
  bool          m_invert;
};



enum EffectScope
{
  EFFECT_SCOPE_GROUP = 0,
  EFFECT_SCOPE_INSTRUMENT,
  _MAX_EFFECT_SCOPE
};

/***************************************************
  *****         Effectors class                *****
  ***************************************************/

class Effectors {
public:

  int add(EffectScope scope, const IEffector &src);
  void groupGate(int nPoly, bool gate);
  void groupBypass(int nPoly, bool y);

  int processGroup(int nPoly, Sample_t *buff, size_t nsamples, size_t channels);
  int processInstrument(Sample_t *buff, size_t nsamples, size_t channels);

private:
  V_LIST<IEffector> m_slotsGroup[_MAX_POLYPHONY_NUM];
  V_LIST<IEffector> m_slotsInstrument;
};

} // namespace effect

#endif //!defined(EFFECT_H_)
