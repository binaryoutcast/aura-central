/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CAIRO_PATH_FIXED_PRIVATE_H
#define CAIRO_PATH_FIXED_PRIVATE_H

#include "cairo-types-private.h"
#include "cairo-compiler-private.h"
#include "cairo-list-private.h"

#define WATCH_PATH 0
#if WATCH_PATH
#include <stdio.h>
#endif

enum cairo_path_op {
  CAIRO_PATH_OP_MOVE_TO = 0,
  CAIRO_PATH_OP_LINE_TO = 1,
  CAIRO_PATH_OP_CURVE_TO = 2,
  CAIRO_PATH_OP_CLOSE_PATH = 3
};

/* we want to make sure a single byte is used for the enum */
typedef char cairo_path_op_t;

/* make _cairo_path_fixed fit into ~512 bytes -- about 50 items */
#define CAIRO_PATH_BUF_SIZE ((512 - sizeof (cairo_path_buf_t)) \
               / (2 * sizeof (cairo_point_t) + sizeof (cairo_path_op_t)))

typedef struct _cairo_path_buf {
  cairo_list_t link;
  unsigned int num_ops;
  unsigned int size_ops;
  unsigned int num_points;
  unsigned int size_points;

  cairo_path_op_t *op;
  cairo_point_t *points;
} cairo_path_buf_t;

typedef struct _cairo_path_buf_fixed {
  cairo_path_buf_t base;

  cairo_path_op_t op[CAIRO_PATH_BUF_SIZE];
  cairo_point_t points[2 * CAIRO_PATH_BUF_SIZE];
} cairo_path_buf_fixed_t;

struct _cairo_path_fixed {
  cairo_point_t last_move_point;
  cairo_point_t current_point;
  unsigned int has_current_point    : 1;
  unsigned int has_last_move_point    : 1;
  unsigned int has_curve_to        : 1;
  unsigned int is_rectilinear        : 1;
  unsigned int maybe_fill_region    : 1;
  unsigned int is_empty_fill        : 1;

  cairo_box_t extents;

  cairo_path_buf_fixed_t  buf;
};

cairo_private void
_cairo_path_fixed_translate (cairo_path_fixed_t *path,
               cairo_fixed_t offx,
               cairo_fixed_t offy);

cairo_private cairo_status_t
_cairo_path_fixed_append (cairo_path_fixed_t          *path,
              const cairo_path_fixed_t      *other,
              cairo_direction_t           dir,
              cairo_fixed_t               tx,
              cairo_fixed_t               ty);

cairo_private unsigned long
_cairo_path_fixed_hash (const cairo_path_fixed_t *path);

cairo_private unsigned long
_cairo_path_fixed_size (const cairo_path_fixed_t *path);

cairo_private cairo_bool_t
_cairo_path_fixed_equal (const cairo_path_fixed_t *a,
             const cairo_path_fixed_t *b);

typedef struct _cairo_path_fixed_iter {
  const cairo_path_buf_t *first;
  const cairo_path_buf_t *buf;
  unsigned int n_op;
  unsigned int n_point;
} cairo_path_fixed_iter_t;

cairo_private void
_cairo_path_fixed_iter_init (cairo_path_fixed_iter_t *iter,
               const cairo_path_fixed_t *path);

cairo_private cairo_bool_t
_cairo_path_fixed_iter_is_fill_box (cairo_path_fixed_iter_t *_iter,
                  cairo_box_t *box);

cairo_private cairo_bool_t
_cairo_path_fixed_iter_at_end (const cairo_path_fixed_iter_t *iter);

static inline cairo_bool_t
_cairo_path_fixed_fill_is_empty (const cairo_path_fixed_t *path)
{
  return path->is_empty_fill;
}

static inline cairo_bool_t
_cairo_path_fixed_is_rectilinear_fill (const cairo_path_fixed_t *path)
{
  if (! path->is_rectilinear)
    return 0;

  if (! path->has_current_point)
    return 1;

  /* check whether the implicit close preserves the rectilinear property */
  return path->current_point.x == path->last_move_point.x ||
       path->current_point.y == path->last_move_point.y;
}

static inline cairo_bool_t
_cairo_path_fixed_maybe_fill_region (const cairo_path_fixed_t *path)
{
#if WATCH_PATH
  fprintf (stderr, "_cairo_path_fixed_maybe_fill_region () = %s\n",
       path->maybe_fill_region ? "true" : "false");
#endif
  return path->maybe_fill_region;
}

#endif /* CAIRO_PATH_FIXED_PRIVATE_H */
