/** @file
 * Qin - Errors management.
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
#include "util/error.h"
#include "util/misc.h"


/*******************************************************************************
*   Automatics                                                                 *
*******************************************************************************/
static const ErrorInfo g_errorsDescriptors[] = {
#   include "errors-generated.h"
  {"Unknown", "Unknown status", "UNKNOWN", 0}
};

////////////////////////////////////////////////////////////////////////////////

/**
 * Get the messages by error code.
 * @param   rc          Returned status code.
 * @return A poninter to ErrorInfo struct whose content is
 *         read only, if the status code is matched.
 * @return Default 'UNKNOWN' struct if rc is not matched.
 */
const ErrorInfo* GetErrorMsg(int rc) {
  unsigned i;
  bool matched = false;

  for(i=0;i<GET_ELEMENTS(g_errorsDescriptors)-1;i++)
    {
      if(g_errorsDescriptors[i].code == rc)
        {
          matched = true;
          break;
        }
    }

  if(matched)
    return &g_errorsDescriptors[i];
  else
    {
      return &g_errorsDescriptors[GET_ELEMENTS(g_errorsDescriptors)-2];
    }
}
