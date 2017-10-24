/** @file
 * Qin - Swapping word order.
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

#ifndef BSWAP_H_
#define BSWAP_H_

#include "util/misc.h"
#include "util/types.h"

#define BSWAP16C(x) (((x) << 8 & 0xff00)  | ((x) >> 8 & 0x00ff))
#define BSWAP32C(x) (BSWAP16C(x) << 16 | BSWAP16C((x) >> 16))
#define BSWAP64C(x) (BSWAP32C(x) << 32 | BSWAP32C((x) >> 32))

#define BSWAPC(s, x) BSWAP##s##C(x)

#ifndef bswap16
static inline uint16_t bswap16(uint16_t x) {
    x= (x>>8) | (x<<8);
    return x;
}
#endif

#ifndef bswap32
static inline uint32_t bswap32(uint32_t x)
{
    x= ((x<<8)&0xFF00FF00) | ((x>>8)&0x00FF00FF);
    x= (x>>16) | (x<<16);
    return x;
}
#endif

#ifndef bswap64
static inline uint64_t bswap64(uint64_t x)
{
#if 0
    x= ((x<< 8)&0xFF00FF00FF00FF00ULL) | ((x>> 8)&0x00FF00FF00FF00FFULL);
    x= ((x<<16)&0xFFFF0000FFFF0000ULL) | ((x>>16)&0x0000FFFF0000FFFFULL);
    return (x>>32) | (x<<32);
#else
    union {
        uint64_t ll;
        uint32_t l[2];
    } w, r;
    w.ll = x;
    r.l[0] = bswap32 (w.l[1]);
    r.l[1] = bswap32 (w.l[0]);
    return r.ll;
#endif
}
#endif

// be2ne ... big-endian to native-endian
// le2ne ... little-endian to native-endian

#if HAVE(BIGENDIAN)
#define ne2be16(x) (x)
#define ne2be32(x) (x)
#define ne2be64(x) (x)
#define be2ne16(x) (x)
#define be2ne32(x) (x)
#define be2ne64(x) (x)
#define le2ne16(x) bswap16(x)
#define le2ne32(x) bswap32(x)
#define le2ne64(x) bswap64(x)
#define BE2NEC(s, x) (x)
#define LE2NEC(s, x) BSWAPC(s, x)
#else
#define ne2le16(x) (x)
#define ne2le32(x) (x)
#define ne2le64(x) (x)
#define be2ne16(x) bswap16(x)
#define be2ne32(x) bswap32(x)
#define be2ne64(x) bswap64(x)
#define le2ne16(x) (x)
#define le2ne32(x) (x)
#define le2ne64(x) (x)
#define BE2NEC(s, x) BSWAPC(s, x)
#define LE2NEC(s, x) (x)
#endif

#define BE2NE16C(x) BE2NEC(16, x)
#define BE2NE32C(x) BE2NEC(32, x)
#define BE2NE64C(x) BE2NEC(64, x)
#define LE2NE16C(x) LE2NEC(16, x)
#define LE2NE32C(x) LE2NEC(32, x)
#define LE2NE64C(x) LE2NEC(64, x)

#if HAVE(BIGENDIAN)
#define be2me_16(x) (x)
#define be2me_32(x) (x)
#define be2me_64(x) (x)
#define le2me_16(x) bswap16(x)
#define le2me_32(x) bswap32(x)
#define le2me_64(x) bswap64(x)
#else
#define be2me_16(x) bswap16(x)
#define be2me_32(x) bswap32(x)
#define be2me_64(x) bswap64(x)
#define le2me_16(x) (x)
#define le2me_32(x) (x)
#define le2me_64(x) (x)
#endif

#endif //!defined(BSWAP_H_)
