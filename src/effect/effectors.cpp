/** @file
 * Qin - Effectors.
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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "util/misc.h"
#include "util/error.h"
#include "util/assert.h"
#include "util/log.h"
#include "effect/effect.h"

namespace effect
{

////////////////////////////////////////////////////////////////////////////////

/**
 * Create and add effector to the list. This will
 * create new instances of the source effector and the
 * source will be discarded.
 * @param scope     Which scope you want to insert.
 * @param src       Reference of source effector.
 * @return status code.
 */
int
Effectors::add(EffectScope scope, const IEffector &src)
{
  int rc;

  IEffector *instance = 0;

  switch (scope)
  {
    case EFFECT_SCOPE_GROUP:
      {
        for (int n = 0; n < _MAX_POLYPHONY_NUM; n++)
          {
            /*
             * Each slot has its independent effector instance, so
             * call create() again and again there.
             */
            instance = src.create();
            if (!instance)
              {
                return VERR_ALLOC_MEMORY;
              }

            rc = m_slotsGroup[n].push(instance);
            if (V_FAILURE(rc)) return rc;
          }
        return VINF_SUCCEEDED;
      }

    case EFFECT_SCOPE_INSTRUMENT:
      {
        instance = src.create();
        if (!instance)
          {
            return VERR_ALLOC_MEMORY;
          }
        rc = m_slotsInstrument.push(instance);
      }
      break;

    default:
      V_ASSERT(0);
      return VERR_FAILED;
  }

  return rc;

}

/**
 * Send the gate signal to the group insert effector.
 * @param nPoly       Index of target polyphony unit.
 * @param gate        true = on, false = off.
 * @return status code.
 */
void
Effectors::groupGate(int nPoly, bool gate)
{
  V_ASSERT(nPoly >=0 && nPoly < _MAX_POLYPHONY_NUM);

  for (IEffector *e = m_slotsGroup[nPoly].root; e; e = e->next)
    {
      e->gate(gate);
    }
}

/**
 * Send the bypass signal to the group insert effector.
 * @param nPoly       Index of target polyphony unit.
 * @param y           true = on, false = off.
 * @return status code.
 */
void
Effectors::groupBypass(int nPoly, bool y)
{
  V_ASSERT(nPoly >=0 && nPoly < _MAX_POLYPHONY_NUM);

  for (IEffector *e = m_slotsGroup[nPoly].root; e; e = e->next)
    {
      e->bypass(y);
    }
}

/**
 * Process the audio buffer.
 * @param nPoly       Index of targte poly unit.
 * @param buff        Pointer to the target buffer.
 * @param nsamples    How many frames are there in the target buffer.
 * @param channels    The number of channels in each frame.
 * @return status code.
 */
int
Effectors::processGroup(int nPoly, Sample_t *buff, size_t nsamples, size_t channels)
{
  int rc;
  V_ASSERT(nPoly >=0 && nPoly < _MAX_POLYPHONY_NUM);
  V_ASSERT(nsamples % channels == 0);

  V_LIST<IEffector> slots = m_slotsGroup[nPoly];

  for (IEffector *e = slots.root; e; e = e->next)
    {
      rc = e->process(&buff, nsamples / channels);
      if (V_FAILURE(rc)) return rc;
    }

  return VINF_SUCCEEDED;
}

/**
 * Process the audio buffer.
 * @param buff        Pointer to the target buffer.
 * @param nsamples    How many frames are there in the target buffer.
 * @param channels    The number of channels in each frame.
 * @return status code.
 */
int
Effectors::processInstrument(Sample_t *buff, size_t nsamples, size_t channels)
{
  int rc;
  V_ASSERT(nsamples % channels == 0);

  for (IEffector *e = m_slotsInstrument.root; e; e = e->next)
    {
      rc = e->process(&buff, nsamples / channels);
      if (V_FAILURE(rc)) return rc;
    }

  return VINF_SUCCEEDED;
}

} // namespace effect
