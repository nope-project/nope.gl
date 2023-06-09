/*
 * Copyright 2023 Matthieu Bouron <matthieu.bouron@gmail.com>
 * Copyright 2019-2022 GoPro Inc.
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

#ifndef TYPE_H
#define TYPE_H

enum {
    NGLI_TYPE_NONE,
    NGLI_TYPE_I32,
    NGLI_TYPE_IVEC2,
    NGLI_TYPE_IVEC3,
    NGLI_TYPE_IVEC4,
    NGLI_TYPE_U32,
    NGLI_TYPE_UVEC2,
    NGLI_TYPE_UVEC3,
    NGLI_TYPE_UVEC4,
    NGLI_TYPE_F32,
    NGLI_TYPE_VEC2,
    NGLI_TYPE_VEC3,
    NGLI_TYPE_VEC4,
    NGLI_TYPE_MAT3,
    NGLI_TYPE_MAT4,
    NGLI_TYPE_BOOL,
    NGLI_TYPE_SAMPLER_2D,
    NGLI_TYPE_SAMPLER_2D_ARRAY,
    NGLI_TYPE_SAMPLER_2D_RECT,
    NGLI_TYPE_SAMPLER_3D,
    NGLI_TYPE_SAMPLER_CUBE,
    NGLI_TYPE_SAMPLER_EXTERNAL_OES,
    NGLI_TYPE_SAMPLER_EXTERNAL_2D_Y2Y_EXT,
    NGLI_TYPE_IMAGE_2D,
    NGLI_TYPE_IMAGE_2D_ARRAY,
    NGLI_TYPE_IMAGE_3D,
    NGLI_TYPE_IMAGE_CUBE,
    NGLI_TYPE_UNIFORM_BUFFER,
    NGLI_TYPE_UNIFORM_BUFFER_DYNAMIC,
    NGLI_TYPE_STORAGE_BUFFER,
    NGLI_TYPE_STORAGE_BUFFER_DYNAMIC,
    NGLI_TYPE_NB
};

const char *ngli_type_get_name(int type);

#endif
