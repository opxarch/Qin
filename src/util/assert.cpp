/** @file
 * Qin - Assertion management.
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

#include <iostream>
#include "util/misc.h"
#include "util/assert.h"


////////////////////////////////////////////////////////////////////////////////

#if ENABLE(ASSERTIONS)

void
lpAssertFailure(const char* file, int line, const char* func, const char*expr)
{
  UNUSED(file);
  UNUSED(line);
  UNUSED(func);
  UNUSED(expr);

#if 1
  std::cerr << "\nAssertion failed at :" << file << ":"<< line << ":" << func << " expression = (" << expr << ")." << std::endl;
#endif

}

void
lpAssertFailureLog(const char* format,...)
{
  UNUSED(format);
}

#endif

