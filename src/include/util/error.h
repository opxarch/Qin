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

#ifndef V_ERROR_H_
#define V_ERROR_H_

#include "util/assert.h"

/*
 * Errors
 */

/* gen{{ */

/** Operation was replaced. */
#define VINF_REPLACED (2)
/** Operation was succeeded. */
#define VINF_SUCCEEDED (1)
/** Operation was failed. */
#define VERR_FAILED (0)
/** Failed on allocating the memory. */
#define VERR_ALLOC_MEMORY (-1)
/** The Buffer is overflow. */
#define VERR_BUFFER_OVERFLOW (-2)
/** Failed on opening file */
#define VERR_OPEN_FILE (-3)
/** Failed on reading from the file */
#define VERR_READING_FILE (-4)
/** The format of the source file is invalid */
#define VERR_INVALID_FORMAT (-5)
/** Invalid parameter(s) */
#define VERR_INVALID_PARAMETER (-6)
/** The parameter(s) contained invalid number */
#define VERR_INVALID_NUMBER (-7)
/** The data is invalid */
#define VERR_INVALID_DATA (-8)
/** Out of the range */
#define VERR_OUT_OF_RANGE (-9)
/** Queue is full */
#define VERR_QUEUE_FULL (-10)
/** Queue is empty */
#define VERR_QUEUE_EMPTY (-11)

/* }}gen */


#define V_SUCCESS(__rc) (__rc>0)
#define V_FAILURE(__rc) (__rc<1)

#define UPDATE_RC(__rc) do {if V_FAILURE(__rc) return __rc;} while(0)

#define AssertRC(__rc) do {V_ASSERT(V_SUCCESS(__rc)); } while(0)

/*
 Structs
 */
typedef struct ErrorInfo {
  const char *msgShort;  /**< Pointer to the short message string. */
  const char *msgFull;   /**< Pointer to the full message string. */
  const char *msgDefine; /**< Pointer to the define string. */
  int        code;       /**< Status code number. */
} ErrorInfo;


/*
 funcs
 */
const ErrorInfo* GetErrorMsg(int rc);


#endif //!defined(V_ERROR_H_)
