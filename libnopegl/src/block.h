/*
 * Copyright 2020-2022 GoPro Inc.
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

#ifndef BLOCK_H
#define BLOCK_H

#include <stdint.h>

#include "buffer.h"
#include "darray.h"
#include "program.h" // MAX_ID_LEN

enum block_layout {
    NGLI_BLOCK_LAYOUT_UNKNOWN,
    NGLI_BLOCK_LAYOUT_STD140,
    NGLI_BLOCK_LAYOUT_STD430,
    NGLI_BLOCK_NB_LAYOUTS
};

struct block_field {
    char name[MAX_ID_LEN];
    int type;
    size_t count;
    size_t offset;
    size_t size;
    size_t stride;
};

void ngli_block_field_copy(const struct block_field *fi, uint8_t *dst, const uint8_t *src);

struct block {
    enum block_layout layout;
    struct darray fields; // block_field
    size_t size;
};

#define NGLI_BLOCK_VARIADIC_COUNT -1

void ngli_block_init(struct block *s, enum block_layout layout);
size_t ngli_block_get_size(const struct block *s, size_t variadic_count);
int ngli_block_add_field(struct block *s, const char *name, int type, size_t count);
void ngli_block_reset(struct block *s);

#endif
