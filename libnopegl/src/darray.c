/*
 * Copyright 2018-2022 GoPro Inc.
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#include <string.h>

#include "nopegl.h"
#include "darray.h"
#include "memory.h"
#include "utils.h"

static int reserve_non_aligned(struct darray *darray, size_t capacity)
{
    if (capacity < darray->capacity)
        return 0;

    void *ptr = ngli_realloc(darray->data, capacity, darray->element_size);
    if (!ptr)
        return NGL_ERROR_MEMORY;
    darray->data = ptr;
    darray->capacity = capacity;
    return 0;
}

static int reserve_aligned(struct darray *darray, size_t capacity)
{
    if (capacity < darray->capacity)
        return 0;

#if HAVE_BUILTIN_OVERFLOW
    size_t bytes;
    if (__builtin_mul_overflow(capacity, darray->element_size, &bytes))
        return NGL_ERROR_MEMORY;
#else
    size_t bytes = capacity * darray->element_size;
#endif
    void *ptr = ngli_malloc_aligned(bytes);
    if (!ptr)
        return NGL_ERROR_MEMORY;
    if (darray->data) {
        memcpy(ptr, darray->data, darray->capacity * darray->element_size);
        ngli_free_aligned(darray->data);
    }
    darray->data = ptr;
    darray->capacity = capacity;
    return 0;
}

void ngli_darray_init(struct darray *darray, size_t element_size, int aligned)
{
    darray->data = NULL;
    darray->count = 0;
    darray->capacity = 0;
    darray->element_size = element_size;
    if (aligned) {
        darray->reserve = reserve_aligned;
        darray->release = ngli_free_aligned;
    } else {
        darray->reserve = reserve_non_aligned;
        darray->release = ngli_free;
    }
}

void *ngli_darray_push(struct darray *darray, const void *element)
{
    if (darray->count >= darray->capacity) {
#if HAVE_BUILTIN_OVERFLOW
        size_t new_capacity;
        if (__builtin_mul_overflow(darray->capacity, 2, &new_capacity))
            return NULL;
#else
        /* Also includes the realloc overflow check */
        if (darray->capacity >= 1ULL << (sizeof(darray->capacity)*8 - 2))
            return NULL;
        size_t new_capacity = darray->capacity * 2;
#endif
        int ret = darray->reserve(darray, darray->capacity ? new_capacity : 8);
        if (ret < 0)
            return NULL;
    }
    void *new_element = darray->data + darray->count * darray->element_size;
    darray->count++;
    if (element)
        memcpy(new_element, element, darray->element_size);
    return new_element;
}

void *ngli_darray_tail(const struct darray *darray)
{
    if (darray->count <= 0)
        return NULL;
    return darray->data + (darray->count - 1) * darray->element_size;
}

void *ngli_darray_pop(struct darray *darray)
{
    void *element = ngli_darray_tail(darray);
    darray->count = darray->count > 0 ? darray->count - 1 : 0;
    return element;
}

void *ngli_darray_pop_unsafe(struct darray *darray)
{
    darray->count--;
    return darray->data + darray->count * darray->element_size;
}

void *ngli_darray_get(const struct darray *darray, size_t index)
{
    ngli_assert(index < darray->count);
    return darray->data + index * darray->element_size;
}

void ngli_darray_remove(struct darray *darray, size_t index)
{
    ngli_darray_remove_range(darray, index, 1);
}

void ngli_darray_remove_range(struct darray *darray, size_t index, size_t count)
{
    ngli_assert((index + count) <= darray->count);
    uint8_t *dst = darray->data + index * darray->element_size;
    const uint8_t *src = darray->data + (index + count) * darray->element_size;
    const size_t n = (darray->count - index - count) * darray->element_size;
    memmove(dst, src, n);
    darray->count -= count;
}

void ngli_darray_reset(struct darray *darray)
{
    if (darray->release)
        darray->release(darray->data);
    memset(darray, 0, sizeof(*darray));
}
