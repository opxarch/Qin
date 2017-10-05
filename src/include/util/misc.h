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

#ifndef V_MISC_H_
#define V_MISC_H_

#include <assert.h>
#include <cstddef>
#include <iostream>

/*
 * automatically conf
 */
#include "config-generated.h"

/*
 * Switchs
 */

/** @def ARCH()
 * wheather or not we're compiling with the target CPU architecture.
 */
#define ARCH(type)  (defined ARCH_##type)
/** @def OS()
 * wheather or not we're compiling with the target OS.
 */
#define OS(type)    (defined OS_##type)
/** @def ENDIAN()
 * indicates which endian the target supported
 */
#define ENDIAN(type)  (defined ENDIAN_##type)
/** @def RING()
 * indicates whether we're in ring-n
 * In other arch except X86 or AMD64:
 *      RING0 -------> kernel
 *      RING3 -------> user
 */
#define RING(n)     (defined IN_RING##n)
/** @def USES()
 * wheather or not to use someting
 */
#define USES(obj)    (defined USES_##obj && USES_##obj)
/** @def HAVE()
 * indicates whether the host machine supports 'obj'
 */
#define HAVE(obj)   (defined HAVE_##obj && HAVE_##obj)
/** @def ENABLE()
 * indicates whether the 'obj' is enabled
 */
#define ENABLE(obj) (defined ENABLE_##obj && ENABLE_##obj)

////////////////////////////////////////////////////////////////////////////////

/** @def COMPILER()
 * indicates whether we're compiling with 'cmp' compiler
 */
#define COMPILER(cmp) (defined COMPILER_##cmp && COMPILER_##cmp)
/** @def COMPILER_SUP()
 * indicates whether the compiler supported 'obj' feature
 */
#define COMPILER_SUP(obj) (defined COMPILER_SUP_##obj && COMPILER_SUP_##obj)


/**
 determine the current compilers
 */
#if defined(__GNUC__)
#define COMPILER_GCC 1
#elif defined(_MSC_VER)
#define COMPILER_MSC 1
#endif

////////////////////////////////////////////////////////////////////////////////

/*
 * cdefs
 */

/** @def UNLIKELY
 * Branch prediction. Unlikely
 * @returns the value of expression.
 * @param   expr  The expression.
 */

#ifdef __GNUC__
# if __GNUC__ >= 3 && !defined(FORTIFY_RUNNING)
#  ifndef LIKELY
#  define LIKELY(expr)       __builtin_expect(!!(expr), 1)
#  endif
#  ifndef UNLIKELY
#  define UNLIKELY(expr)     __builtin_expect(!!(expr), 0)
#  endif
# else
#  ifndef LIKELY
#  define LIKELY(expr)       (expr)
#  endif
#  ifndef UNLIKELY
#  define UNLIKELY(expr)     (expr)
#  endif
# endif
#else
# ifndef LIKELY
# define LIKELY(expr)       (expr)
# endif
# ifndef UNLIKELY
# define UNLIKELY(expr)     (expr)
# endif
#endif

#if __GNUC__
#define V_CURRENT_FUNCTION __PRETTY_FUNCTION__
#else
#define V_CURRENT_FUNCTION __FUNCTION__
#endif
#define V_CURRENT_FILE __FILE__
#define V_CURRENT_LINE __LINE__

/** @def NULL
 * Null pointer
 */
#ifndef NULL
# ifdef __cplusplus
# define NULL 0
#else
# define NULL (void*)0
#endif
#endif

/** @def GET_ELEMENTS
 * Calculates the number of elements in a statically sized array.
 * @returns Element count.
 * @param   __array      Array in question.
 */
#define GET_ELEMENTS(__array)         ( sizeof(__array) / sizeof((__array)[0]) )

/** @def UNUSED
 * Avoid the 'unused parameter' warning.
 */
#define UNUSED(var) (void)var


////////////////////////////////////////////////////////////////////////////////


/*
 * Marks
 */
#define __OUT
#define __IN

////////////////////////////////////////////////////////////////////////////////

/*
 * Export
 */
#ifdef IN_SHARED
# ifdef __GNUC__
#  define V_EXPORT __attribute__((visibility("default")))
# else
#  define V_EXPORT __declspec(dllexport)
# endif
#else
# define V_EXPORT
#endif

/*
 * ASCII Characters
 */
#define V_EOF ((char)255)
#define V_SPACE (' ')


#endif //!defined(V_MISC_H_)
