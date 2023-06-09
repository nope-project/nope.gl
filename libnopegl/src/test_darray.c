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

#include "darray.h"
#include "utils.h"

int main(void)
{
    struct darray darray = {0};
    ngli_darray_init(&darray, sizeof(int), 0);

    size_t count = ngli_darray_count(&darray);
    ngli_assert(count == 0);

    int *element = ngli_darray_push(&darray, NULL);
    ngli_assert(element);
    *element = 0xFF;
    count = ngli_darray_count(&darray);
    ngli_assert(count == 1);

    element = ngli_darray_push(&darray, NULL);
    ngli_assert(element);
    *element = 0xFFFF;
    count = ngli_darray_count(&darray);
    ngli_assert(count == 2);

    element = ngli_darray_tail(&darray);
    ngli_assert(*element == 0xFFFF);

    element = ngli_darray_pop(&darray);
    ngli_assert(*element == 0xFFFF);

    element = ngli_darray_pop(&darray);
    ngli_assert(*element == 0xFF);

    element = ngli_darray_pop(&darray);
    ngli_assert(element == NULL);

    count = ngli_darray_count(&darray);
    ngli_assert(count == 0);

    for (size_t i = 0; i < 1000; i++) {
        element = ngli_darray_push(&darray, NULL);
        ngli_assert(element);
    }

    ngli_darray_reset(&darray);

    return 0;
}
