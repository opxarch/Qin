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

#ifndef V_STRING_H_
#define V_STRING_H_

#include <string>

int v_strcasecmp(const char *s1, const char *s2);
int v_strncasecmp(const char *s1, const char *s2, int len);

int stringToInteger(const char *src, int len, int *val);
int stringToFloat(const char *src, int len, float *val);

void trim(std::string &src);

#endif //!defined(V_STRING_H_)
