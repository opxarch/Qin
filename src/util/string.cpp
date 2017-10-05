/** @file
 * Qin - String common.
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
#include "util/assert.h"
#include "util/string.h"

#include <cstring>
#include <cctype>
#include <string>

////////////////////////////////////////////////////////////////////////////////


int
v_strcasecmp(const char *s1, const char *s2)
{
  int cmp = 0;
  while ((cmp = tolower(*s1) - tolower(*s2)) == 0)
    {
      if (*s1++ == 0 || *s2++ == 0)
        break;
    }
  return cmp;
}

int
v_strncasecmp(const char *s1, const char *s2, int len)
{
  while (--len && *s1 && tolower(*s1) == tolower(*s2))
    {
      s1++; s2++;
    }
  return *s1 - *s2;
}


/**
 * Convert the string to integer.
 *
 * The process of conversation will stop if it meets
 * any non-number characters, expect '+' or '-' that can
 * sign positive and negative.
 *
 * @return status code.
 * @param src           Pointer to source string buffer.
 * @param len           Length of string.
 * @parma val           Where to store the result.
 */
int
stringToInteger(const char *src, int len, int *val)
{
  V_ASSERT(src && val);
  if (!len)
    return VERR_INVALID_NUMBER;
  bool minus = false;
  int res = 0;
  for (int i = 0; i < len; i++)
    {
      if (src[i] == '-')
        {
          minus = true;
          continue;
        }
      if (src[i] == ' ')
        continue;
      if ('0' > src[i] || src[i] > '9')
        break;
      else
        {
          res = res * 10 + (src[i] - '0');
          continue;
        }
    }
  *val = minus ? -res : res;
  return VINF_SUCCEEDED;
}

/**
 * Convert the string to float.
 *
 * The process of conversation will stop if it meets
 * any non-number characters, expect '+' or '-' that can
 * sign positive and negative.
 *
 * @return status code.
 * @param src           Pointer to source string buffer.
 * @param len           Length of string.
 * @parma val           Where to store the result.
 */
int
stringToFloat(const char *src, int len, float *val)
{
  V_ASSERT(src && val);
  if (!len)
      return VERR_INVALID_NUMBER;
  int i;
  double d = 0.0,power = 1.0;
  int sign = 1;
  for (i = 0; i < len; i++)
    {
      if (src[i] == ' ') continue;
      if (src[i] == '-') { sign = -1; continue; }
      if (src[i] == '+') { sign = 1; continue; }
      break;
    }
  if (i == (len - 1))
      return VERR_INVALID_NUMBER;
  // read the integer part
  for (; i < len; i++)
    {
      if ((src[i] >= '0')&&(src[i] <= '9'))
        d = d* 10.0 + (src[i] - '0');
      else if (src[i] == '.') {
        // read the float part
        for (i++; i < len; ++i)
          {
            if ((src[i] >= '0')&&(src[i] <= '9'))
              {
                d = d* 10.0 + (src[i] - '0');
                power *= 10;
              }
            else
              break;
          }
        break;
      }
      else
       break;
    }
  *val = sign*d/power;
  return VINF_SUCCEEDED;
}

/**
 * This function deletes the spaces at the begining or ending.
 * of a string.
 * Warning: Never use it in the key performance node, instead, consider
 * the traditional pointer-type string processing to improve the
 * performance.
 *
 * @param src       Reference to the target string class.
 */
void
trim(std::string &src)
{
  if (src.empty())
      return;
  src.erase(0, src.find_first_not_of(" "));
  src.erase(src.find_last_not_of(" ") + 1);
}
