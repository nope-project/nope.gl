/*
 * Copyright 2016 GoPro Inc.
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

#ifndef INTERNAL_H
#define INTERNAL_H

#include <stdlib.h>
#include <sxplayer.h>

#include "config.h"

#if defined(HAVE_VAAPI)
#include "vaapi_ctx.h"
#endif

#if defined(TARGET_ANDROID)
#include "android_ctx.h"
#include "android_handlerthread.h"
#include "android_surface.h"
#include "android_imagereader.h"
#endif

#include "animation.h"
#include "block.h"
#include "drawutils.h"
#include "graphicstate.h"
#include "hmap.h"
#include "hud.h"
#include "hwconv.h"
#include "hwmap.h"
#include "image.h"
#include "nodegl.h"
#include "params.h"
#include "pgcache.h"
#include "program.h"
#include "pthread_compat.h"
#include "darray.h"
#include "buffer.h"
#include "format.h"
#include "rendertarget.h"
#include "rnode.h"
#include "texture.h"

struct node_class;

typedef int (*cmd_func_type)(struct ngl_ctx *s, void *arg);

struct ngl_ctx {
    /* Controller-only fields */
    int configured;
    pthread_t worker_tid;

    /* Worker-only fields */
    struct gpu_ctx *gpu_ctx;
    struct rnode rnode;
    struct rnode *rnode_pos;
    struct ngl_node *scene;
    struct ngl_config config;
    struct rendertarget *available_rendertargets[2];
    struct rendertarget *current_rendertarget;
    int render_pass_started;
    struct darray modelview_matrix_stack;
    struct darray projection_matrix_stack;

    /*
     * Array of nodes that are candidate to either prefetch (active) or release
     * (non-active). Nodes are inserted from bottom (leaves) up to the top
     * (root).
     */
    struct darray activitycheck_nodes;

    struct texture *font_atlas;
    struct pgcache pgcache;
#if defined(HAVE_VAAPI)
    struct vaapi_ctx vaapi_ctx;
#endif
#if defined(TARGET_ANDROID)
    struct android_ctx android_ctx;
#endif
    struct hud *hud;
    int64_t cpu_update_time;
    int64_t cpu_draw_time;
    int64_t gpu_draw_time;

    /* Shared fields */
    pthread_mutex_t lock;
    pthread_cond_t cond_ctl;
    pthread_cond_t cond_wkr;
    cmd_func_type cmd_func;
    void *cmd_arg;
    int cmd_ret;
};

struct ngl_node {
    const struct node_class *cls;
    struct ngl_ctx *ctx;

    int state;
    int is_active;

    double visit_time;
    double last_update_time;

    int draw_count;

    int refcount;
    int ctx_refcount;

    struct darray children;
    struct darray parents;

    char *label;

    void *priv_data;
};

#define TRANSFORM_TYPES_LIST (const int[]){NGL_NODE_ROTATE,    \
                                           NGL_NODE_ROTATEQUAT,\
                                           NGL_NODE_TRANSFORM, \
                                           NGL_NODE_TRANSLATE, \
                                           NGL_NODE_SCALE,     \
                                           NGL_NODE_SKEW,      \
                                           NGL_NODE_IDENTITY,  \
                                           -1}

/* helper structure to specify the content (or a slice) of a buffer */
struct buffer_layout {
    int type;       // any of NGLI_TYPE_*
    int format;     // any of NGLI_FORMAT_*
    int stride;     // stride of 1 element, in bytes
    int comp;       // number of components per element
    int count;      // number of elements
    int offset;     // offset where the data starts in the buffer, in bytes
};

struct geometry_priv {
    /* quad params */
    float quad_corner[3];
    float quad_width[3];
    float quad_height[3];

    float quad_uv_corner[2];
    float quad_uv_width[2];
    float quad_uv_height[2];

    /* triangle params */
    float triangle_edges[9];
    float triangle_uvs[6];

    /* circle params */
    float radius;
    int npoints;

    /* geometry params */
    struct ngl_node *vertices;
    struct ngl_node *uvcoords;
    struct ngl_node *normals;
    struct ngl_node *indices;
    struct ngl_node *update_nodes[3]; /* {vertices, uvcoords, normals} at most */
    int nb_update_nodes;

    /* shared for all geometries */
    struct buffer *vertices_buffer;
    struct buffer *uvcoords_buffer;
    struct buffer *normals_buffer;
    struct buffer *indices_buffer;

    struct buffer_layout vertices_layout;
    struct buffer_layout uvcoords_layout;
    struct buffer_layout normals_layout;
    struct buffer_layout indices_layout;

    int topology;

    int64_t max_indices;
};

struct buffer_priv {
    struct {
        int count;
        uint8_t *data;
        int data_size;
        char *filename;
        struct ngl_node *block;
        int block_field;
    } opt;

    struct buffer_layout layout;

    uint8_t *data;          // buffer of <count> elements
    int data_size;          // total buffer data size in bytes

    char *filename;         // filename from which the data will be read
    struct ngl_node *block;
    int block_field;
    int usage;              // flags defining buffer use

    /* animatedbuffer */
    struct ngl_node **animkf;
    int nb_animkf;
    struct animation anim;

    /* streamedbuffer */
    struct ngl_node *timestamps;
    struct ngl_node *buffer_node;
    int timebase[2];
    struct ngl_node *time_anim;

    FILE *fp;
    int dynamic;
    int last_index;

    struct buffer *buffer;
    int buffer_refcount;
    double buffer_last_upload_time;
};

int ngli_node_buffer_ref(struct ngl_node *node);
int ngli_node_buffer_init(struct ngl_node *node);
void ngli_node_buffer_unref(struct ngl_node *node);
int ngli_node_buffer_upload(struct ngl_node *node);

struct livectl {
    union ngl_livectl_data val;
    char *id;
    union ngl_livectl_data min;
    union ngl_livectl_data max;
};

struct variable_priv {
    struct livectl opt;

    struct ngl_node **animkf;
    int nb_animkf;

    struct ngl_node *timestamps;
    struct ngl_node *buffer;
    int timebase[2];
    union {
        struct ngl_node *time_anim;
        struct ngl_node *anim_node; /* Velocity nodes only */
        struct ngl_node *path_node; /* AnimatedPath only */
    };

    struct animation anim;
    struct animation anim_eval;
    float vector[4];
    float matrix[4*4];
    int ivector[4];
    unsigned uvector[4];
    double dval;
    void *data;
    int data_size;
    int data_type;          // any of NGLI_TYPE_*
    struct ngl_node *transform;
    int as_mat4; /* quaternion only */
    int dynamic;
    int last_index;
};

int ngli_velocity_evaluate(struct ngl_node *node, void *dst, double t);

struct block_priv {
    struct ngl_node **fields;
    int nb_fields;
    int layout;

    struct block block;
    int force_update;

    uint8_t *data;
    int data_size;
    int usage;

    struct buffer *buffer;
    int buffer_refcount;
    int has_changed;
    double buffer_last_upload_time;
};

int ngli_node_block_ref(struct ngl_node *node);
int ngli_node_block_init(struct ngl_node *node);
void ngli_node_block_unref(struct ngl_node *node);
int ngli_node_block_upload(struct ngl_node *node);

struct program_priv {
    const char *vertex;
    const char *fragment;
    const char *compute;
    int workgroup_size[3];
    struct hmap *properties;
    struct hmap *vert_out_vars;
    int nb_frag_output;

    struct darray vert_out_vars_array; // pgcraft_iovar
};

extern const struct param_choices ngli_mipmap_filter_choices;
extern const struct param_choices ngli_filter_choices;

struct texture_priv {
    int requested_format;
    struct texture_params params;
    struct ngl_node *data_src;
    int direct_rendering;
    int clamp_video;

    uint32_t supported_image_layouts;
    struct texture *texture;
    struct image image;
    struct hwmap hwmap;
};

struct media_priv {
    const char *filename;
    int sxplayer_min_level;
    struct ngl_node *anim;
    int audio_tex;
    int max_nb_packets;
    int max_nb_frames;
    int max_nb_sink;
    int max_pixels;
    int stream_idx;
    int hwaccel;
    char *filters;
    char *vt_pix_fmt;

    struct sxplayer_ctx *player;
    struct sxplayer_frame *frame;
    int nb_parents;

#if defined(TARGET_ANDROID)
    struct android_surface *android_surface;
    struct android_handlerthread *android_handlerthread;
    struct android_imagereader *android_imagereader;
#endif
};

struct timerangemode_priv {
    double start_time;
    double render_time;
    int updated;
};

struct transform_priv {
    struct ngl_node *child;
    NGLI_ALIGNED_MAT(matrix);
};

struct io_priv {
    int precision_out;
    int precision_in;
    int type;
};

struct resourceprops_priv {
    int precision;
    int as_image;
    int writable;
    int variadic;
};

enum easing_id {
    EASING_LINEAR,
    EASING_QUADRATIC_IN,
    EASING_QUADRATIC_OUT,
    EASING_QUADRATIC_IN_OUT,
    EASING_QUADRATIC_OUT_IN,
    EASING_CUBIC_IN,
    EASING_CUBIC_OUT,
    EASING_CUBIC_IN_OUT,
    EASING_CUBIC_OUT_IN,
    EASING_QUARTIC_IN,
    EASING_QUARTIC_OUT,
    EASING_QUARTIC_IN_OUT,
    EASING_QUARTIC_OUT_IN,
    EASING_QUINTIC_IN,
    EASING_QUINTIC_OUT,
    EASING_QUINTIC_IN_OUT,
    EASING_QUINTIC_OUT_IN,
    EASING_POWER_IN,
    EASING_POWER_OUT,
    EASING_POWER_IN_OUT,
    EASING_POWER_OUT_IN,
    EASING_SINUS_IN,
    EASING_SINUS_OUT,
    EASING_SINUS_IN_OUT,
    EASING_SINUS_OUT_IN,
    EASING_EXP_IN,
    EASING_EXP_OUT,
    EASING_EXP_IN_OUT,
    EASING_EXP_OUT_IN,
    EASING_CIRCULAR_IN,
    EASING_CIRCULAR_OUT,
    EASING_CIRCULAR_IN_OUT,
    EASING_CIRCULAR_OUT_IN,
    EASING_BOUNCE_IN,
    EASING_BOUNCE_OUT,
    EASING_ELASTIC_IN,
    EASING_ELASTIC_OUT,
    EASING_BACK_IN,
    EASING_BACK_OUT,
    EASING_BACK_IN_OUT,
    EASING_BACK_OUT_IN,
};

typedef double easing_type;
typedef easing_type (*easing_function)(easing_type, int, const easing_type *);

struct animkeyframe_priv {
    double time;
    float value[4];
    double scalar;
    uint8_t *data;
    int data_size;
    int easing;
    easing_function function;
    easing_function derivative;
    easing_function resolution;
    double *args;
    int nb_args;
    double offsets[2];
    int scale_boundaries;
    double boundaries[2];
    double derivative_scale;
};

struct pathkey_move_priv {
    float to[3];
};

struct pathkey_line_priv {
    float to[3];
};

struct pathkey_bezier2_priv {
    float control[3];
    float to[3];
};

struct pathkey_bezier3_priv {
    float control1[3];
    float control2[3];
    float to[3];
};

struct textureview_priv {
    struct ngl_node *texture;
    int layer;
};

enum {
    NGLI_NODE_CATEGORY_NONE,
    NGLI_NODE_CATEGORY_VARIABLE,
    NGLI_NODE_CATEGORY_TEXTURE,
    NGLI_NODE_CATEGORY_BUFFER,
    NGLI_NODE_CATEGORY_BLOCK,
    NGLI_NODE_CATEGORY_IO,
    NGLI_NODE_CATEGORY_RENDER, /* node executes a graphics pipeline */
};

/*
 * Node is an exposed live control.
 *
 * A few important notes when setting this flag:
 *
 * - the private node context must contain a livectl struct, and
 *   node_class.livectl_offset must point to it (we can not have any static
 *   check for this because 0 is a valid offset)
 * - an option named "live_id" must be exposed in the parameters (and
 *   associated with `livectl.id`)
 * - the value parameter can have any arbitrary name but must be present before
 *   "live_id", point to `livectl.val`, and has to be the first parameter
 *   flagged with NGLI_PARAM_FLAG_ALLOW_LIVE_CHANGE
 */
#define NGLI_NODE_FLAG_LIVECTL (1 << 0)

/**
 *   Operation        State result
 * -----------------------------------
 * I Init           STATE_INITIALIZED
 * P Prefetch       STATE_READY
 * D Update/Draw
 * R Release        STATE_INITIALIZED
 * U Uninit         STATE_UNINITIALIZED
 *
 * Dependency callgraph:
 *
 *             .------------.
 *             v            |
 *     (I) <- [P] <- [D]   (R) <- [U]
 *      |                          ^
 *      `--------------------------'
 *
 * The starting state is [U].
 *
 * .--[ Legend ]-------
 * |
 * | X:      Operation X
 * | X -> Y: X call depends on Y state result
 * | [X]:    if X dependency is not met, the change state call will be made
 * | (X):    if X dependency is not met, it will noop
 * |
 * `-------------------
 *
 * Some examples:
 *  - calling prefetch() will always call init() if necessary
 *  - release() has a weak dependency to prefetch(), so it will noop if not in
 *    the READY state.
 *
 * Note: nodes implementation do NOT have to implement this logic, but they can
 * rely on these properties in their callback implementations.
 */
struct node_class {
    int id;
    int category;
    const char *name;
    int (*init)(struct ngl_node *node);
    int (*prepare)(struct ngl_node *node);
    int (*visit)(struct ngl_node *node, int is_active, double t);
    int (*prefetch)(struct ngl_node *node);
    int (*invalidate)(struct ngl_node *node);
    int (*update)(struct ngl_node *node, double t);
    void (*draw)(struct ngl_node *node);
    void (*release)(struct ngl_node *node);
    void (*uninit)(struct ngl_node *node);
    char *(*info_str)(const struct ngl_node *node);
    size_t priv_size;
    const struct node_param *params;
    const char *params_id;
    size_t livectl_offset;
    uint32_t flags;
    const char *file;
};

void ngli_node_print_specs(void);

int ngli_node_prepare(struct ngl_node *node);
int ngli_node_visit(struct ngl_node *node, int is_active, double t);
int ngli_node_honor_release_prefetch(struct ngl_node *scene, double t);
int ngli_node_update(struct ngl_node *node, double t);
int ngli_node_update_children(struct ngl_node *node, double t);
int ngli_prepare_draw(struct ngl_ctx *s, double t);
void ngli_node_draw(struct ngl_node *node);

int ngli_node_attach_ctx(struct ngl_node *node, struct ngl_ctx *ctx);
void ngli_node_detach_ctx(struct ngl_node *node, struct ngl_ctx *ctx);

int ngli_node_livectls_get(const struct ngl_node *scene, int *nb_livectlsp, struct ngl_livectl **livectlsp);
void ngli_node_livectls_freep(struct ngl_livectl **livectlsp);

char *ngli_node_default_label(const char *class_name);
int ngli_is_default_label(const char *class_name, const char *str);
const struct node_param *ngli_node_param_find(const struct ngl_node *node, const char *key,
                                              uint8_t **base_ptrp);

#endif
