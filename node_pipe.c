/*
 * Copyright 2017 GoPro Inc.
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

#include <unistd.h>

#include "log.h"
#include "nodegl.h"
#include "nodes.h"

#define OFFSET(x) offsetof(struct pipe, x)
static const struct node_param pipe_params[] = {
    {"child",  PARAM_TYPE_NODE, OFFSET(child),  .flags=PARAM_FLAG_CONSTRUCTOR},
    {"fd",     PARAM_TYPE_INT,  OFFSET(fd),     .flags=PARAM_FLAG_CONSTRUCTOR},
    {"width",  PARAM_TYPE_INT,  OFFSET(width),  .flags=PARAM_FLAG_CONSTRUCTOR},
    {"height", PARAM_TYPE_INT,  OFFSET(height), .flags=PARAM_FLAG_CONSTRUCTOR},
    {NULL}
};

static int pipe_init(struct ngl_node *node)
{
    struct pipe *s = node->priv_data;

    s->buf = calloc(4 /* RGBA */, s->width * s->height);
    if (!s->buf)
        return -1;

    int ret = ngli_node_init(s->child);
    if (ret < 0)
        return ret;

    return 0;
}

static void pipe_update(struct ngl_node *node, double t)
{
    struct pipe *s = node->priv_data;
    ngli_node_update(s->child, t);
}

static void pipe_draw(struct ngl_node *node)
{
    struct pipe *s = node->priv_data;
    ngli_node_draw(s->child);

    LOG(DEBUG, "write %dx%d buffer to FD=%d", s->width, s->height, s->fd);
    glReadPixels(0, 0, s->width, s->height, GL_RGBA, GL_UNSIGNED_BYTE, s->buf);
    write(s->fd, s->buf, s->width * s->height * 4);
}

static void pipe_uninit(struct ngl_node *node)
{
    struct pipe *s = node->priv_data;
    free(s->buf);
}

const struct node_class ngli_pipe_class = {
    .id        = NGL_NODE_PIPE,
    .name      = "Pipe",
    .init      = pipe_init,
    .update    = pipe_update,
    .draw      = pipe_draw,
    .uninit    = pipe_uninit,
    .priv_size = sizeof(struct pipe),
    .params    = pipe_params,
};