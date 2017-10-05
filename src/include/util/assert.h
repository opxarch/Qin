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

#ifndef V_ASSERT_H_
#define V_ASSERT_H_

#include "util/misc.h"

/*
 * Assertions
 */
#if ENABLE(ASSERTIONS)

#define V_ASSERT_PANIC() assert(0)

void lpAssertFailure(const char* file, int line, const char* func, const char*expr);
void lpAssertFailureLog(const char* format,...);

/** @todo: implement the variable parameter by __VA_ARGS__ */

/** @def V_ASSERT
 * ASSERT that an expression is true. If it's not emit the breakpoint.
 * @param   assertion  The Expression.
 */
#define V_ASSERT(assertion) \
    (UNLIKELY(!(assertion)) ? \
        (lpAssertFailure(__FILE__, __LINE__, V_CURRENT_FUNCTION, #assertion), \
         V_ASSERT_PANIC()) : \
        (void)0)


/** @def V_ASSERT_LOG
 * If the expression isn't true, will report the txt-message and emit the breakpoint.
 * @param   assertion  The Expression.
 * @param   msg        Message format,args,... (must be in brackets).
 *                     eg. V_ASSERT_LOG(expr, ("msg format", arg1, arg2, ...) );
 */
#define V_ASSERT_LOG(assertion, msg) \
    (UNLIKELY(!(assertion)) ? \
        (lpAssertFailure(__FILE__, __LINE__, V_CURRENT_FUNCTION, #assertion), \
         lpAssertFailureMsg msg , \
         V_ASSERT_PANIC()) : \
        (void)0)


/** @def ASSERT_STATIC_INT
 * static_assert emulated.(non standard)
 * This differs from AssertCompile in that it accepts some more expressions
 * than what C++0x allows
 * @param   assertion    The expression.
 */
#define V_ASSERT_STATIC_INT(assertion) typedef int _known[(assertion) ? 1 : -1];

/** @def ASSERT_STATIC
 * Asserts that a C++0x compile-time expression is true. If it's not break the
 * build.
 * @param   assertion    The Expression.
 */
#ifdef HAVE_STATIC_ASSERT
# define V_ASSERT_STATIC(assertion) static_assert(!!(assertion), #assertion)
#else
# define V_ASSERT_STATIC(assertion) V_ASSERT_STATIC_INT(assertion)
#endif


#else

/*
 dummy
 */
#define V_ASSERT(assertion) ((void)0)
#define V_ASSERT_LOG(assertion, msg) ((void)0)
#define V_ASSERT_STATIC(assertion) ((void)0)

#endif //ENABLE(ASSERTIONS)


#endif //!defined(V_ASSERT_H_)
