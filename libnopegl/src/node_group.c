/*
 * Copyright 2016-2022 GoPro Inc.
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

#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include "nopegl.h"
#include "internal.h"

struct group_opts {
    struct ngl_node **children;
    size_t nb_children;
};

#define OFFSET(x) offsetof(struct group_opts, x)
static const struct node_param group_params[] = {
    {"children", NGLI_PARAM_TYPE_NODELIST, OFFSET(children),
                 .desc=NGLI_DOCSTRING("a set of scenes")},
    {NULL}
};

static int group_prepare(struct ngl_node *node)
{
    struct ngl_ctx *ctx = node->ctx;
    const struct group_opts *o = node->opts;

    int ret = 0;
    struct rnode *rnode_pos = ctx->rnode_pos;
    for (size_t i = 0; i < o->nb_children; i++) {
        struct rnode *rnode = ngli_rnode_add_child(rnode_pos);
        if (!rnode)
            return NGL_ERROR_MEMORY;
        ctx->rnode_pos = rnode;

        struct ngl_node *child = o->children[i];
        ret = ngli_node_prepare(child);
        if (ret < 0)
            goto done;
    }

done:
    ctx->rnode_pos = rnode_pos;
    return ret;
}

static void group_draw(struct ngl_node *node)
{
    struct ngl_ctx *ctx = node->ctx;
    const struct group_opts *o = node->opts;

    struct rnode *rnode_pos = ctx->rnode_pos;
    struct rnode *rnodes = ngli_darray_data(&rnode_pos->children);
    for (size_t i = 0; i < o->nb_children; i++) {
        ctx->rnode_pos = &rnodes[i];
        struct ngl_node *child = o->children[i];
        ngli_node_draw(child);
    }
    ctx->rnode_pos = rnode_pos;
}

const struct node_class ngli_group_class = {
    .id        = NGL_NODE_GROUP,
    .name      = "Group",
    .prepare   = group_prepare,
    .update    = ngli_node_update_children,
    .draw      = group_draw,
    .opts_size = sizeof(struct group_opts),
    .params    = group_params,
    .file      = __FILE__,
};
