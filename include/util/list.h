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

#ifndef V_LIST_H_
#define V_LIST_H_

#include "util/error.h"
#include "util/assert.h"

template <typename T>
  class V_LIST
  {
    public:
      V_LIST()
        : root(0),
          tail(0)
      {
      }

      inline int push(T* src)
      {
        src->prev = tail;
        src->next = 0;

        if (!tail && !root)
          {
            tail = src;
            root = src;
          }
        else
          {
            V_ASSERT(tail && root);
            tail->next = src;
          }
        tail = src;
        return VINF_SUCCEEDED;
      }

      void earseRefs()
      {
        T *node = root;
        while(node)
          {
            T *target = node;
            node = node->next;
            delete target;
          }
        clear();
      }

      void clear()
      {
        root = 0;
        tail = 0;
      }

    public:
      T *root;
      T *tail;
  };

#endif //!defined(V_LIST_H_)
