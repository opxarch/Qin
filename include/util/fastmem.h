/** @file
 * Qin - fast memory operation.
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

#ifndef UTIL_FASTMEMCPY_H_
#define UTIL_FASTMEMCPY_H_

#include "util/misc.h"
#include "util/types.h"

#if USES(FASTMEMCPY)
#if HAVE(MMX) || HAVE(MMX2) || HAVE(3DNOW) \
/*    || HAVE(SSE) || HAVE(SSE2) */
#include <stddef.h>

extern void * fast_memcpy(void * to, const void * from, size_t len);
extern void * mem2agpcpy(void * to, const void * from, size_t len);

#else /* HAVE(MMX/MMX2/3DNOW/SSE/SSE2) */
#define mem2agpcpy(a,b,c) memcpy(a,b,c)
#define fast_memcpy(a,b,c) memcpy(a,b,c)
#endif

#else /* USES(FASTMEMCPY) */
#define mem2agpcpy(a,b,c) memcpy(a,b,c)
#define fast_memcpy(a,b,c) memcpy(a,b,c)
#endif

static inline void * mem2agpcpy_pic(void * dst, const void * src, int bytesPerLine, int height, int dstStride, int srcStride)
{
	int i;
	void *retval=dst;

	if(dstStride == srcStride)
	{
		if (srcStride < 0) {
	    		src = (uint8_t*)src + (height-1)*srcStride;
	    		dst = (uint8_t*)dst + (height-1)*dstStride;
	    		srcStride = -srcStride;
		}

		mem2agpcpy(dst, src, srcStride*height);
	}
	else
	{
		for(i=0; i<height; i++)
		{
			mem2agpcpy(dst, src, bytesPerLine);
			src = (uint8_t*)src + srcStride;
			dst = (uint8_t*)dst + dstStride;
		}
	}

	return retval;
}

#define memcpy_pic(d, s, b, h, ds, ss) memcpy_pic2(d, s, b, h, ds, ss, 0)
#define my_memcpy_pic(d, s, b, h, ds, ss) memcpy_pic2(d, s, b, h, ds, ss, 1)

/**
 * \param limit2width always skip data between end of line and start of next
 *                    instead of copying the full block when strides are the same
 */
static inline void * memcpy_pic2(void * dst, const void * src,
                                 int bytesPerLine, int height,
                                 int dstStride, int srcStride, int limit2width)
{
	int i;
	void *retval=dst;

	if(!limit2width && dstStride == srcStride)
	{
		if (srcStride < 0) {
	    		src = (uint8_t*)src + (height-1)*srcStride;
	    		dst = (uint8_t*)dst + (height-1)*dstStride;
	    		srcStride = -srcStride;
		}

		fast_memcpy(dst, src, srcStride*height);
	}
	else
	{
		for(i=0; i<height; i++)
		{
			fast_memcpy(dst, src, bytesPerLine);
			src = (uint8_t*)src + srcStride;
			dst = (uint8_t*)dst + dstStride;
		}
	}

	return retval;
}

#endif //!defined(UTIL_FASTMEMCPY_H_)
