/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "cairoint.h"

#include "cairo-private.h"
#include "cairo-error-private.h"
#include "cairo-path-private.h"
#include "cairo-path-fixed-private.h"

/**
 * SECTION:cairo-paths
 * @Title: Paths
 * @Short_Description: Creating paths and manipulating path data
 *
 * Paths are the most basic drawing tools and are primarily used to implicitly
 * generate simple masks.
 */

static const cairo_path_t _cairo_path_nil = { CAIRO_STATUS_NO_MEMORY, NULL, 0 };

/* Closure for path interpretation. */
typedef struct cairo_path_count {
  int count;
  cairo_point_t current_point;
} cpc_t;

static cairo_status_t
_cpc_move_to (void *closure,
        const cairo_point_t *point)
{
  cpc_t *cpc = closure;

  cpc->count += 2;

  cpc->current_point = *point;

  return CAIRO_STATUS_SUCCESS;
}

static cairo_status_t
_cpc_line_to (void *closure,
        const cairo_point_t *point)
{
  cpc_t *cpc = closure;

  cpc->count += 2;

  cpc->current_point = *point;

  return CAIRO_STATUS_SUCCESS;
}

static cairo_status_t
_cpc_curve_to (void        *closure,
         const cairo_point_t    *p1,
         const cairo_point_t    *p2,
         const cairo_point_t    *p3)
{
  cpc_t *cpc = closure;

  cpc->count += 4;

  cpc->current_point = *p3;

  return CAIRO_STATUS_SUCCESS;
}

static cairo_status_t
_cpc_close_path (void *closure)
{
  cpc_t *cpc = closure;

  cpc->count += 1;

  return CAIRO_STATUS_SUCCESS;
}

static int
_cairo_path_count (cairo_path_t        *path,
           cairo_path_fixed_t    *path_fixed,
           double         tolerance,
           cairo_bool_t         flatten)
{
  cairo_status_t status;
  cpc_t cpc;

  cpc.count = 0;
  cpc.current_point.x = 0;
  cpc.current_point.y = 0;

  if (flatten) {
    status = _cairo_path_fixed_interpret_flat (path_fixed,
                           CAIRO_DIRECTION_FORWARD,
                           _cpc_move_to,
                           _cpc_line_to,
                           _cpc_close_path,
                           &cpc,
                           tolerance);
  } else {
    status = _cairo_path_fixed_interpret (path_fixed,
                        CAIRO_DIRECTION_FORWARD,
                        _cpc_move_to,
                        _cpc_line_to,
                        _cpc_curve_to,
                        _cpc_close_path,
                        &cpc);
  }

  if (unlikely (status))
    return -1;

  return cpc.count;
}

/* Closure for path interpretation. */
typedef struct cairo_path_populate {
  cairo_path_data_t *data;
  cairo_gstate_t  *gstate;
  cairo_point_t    current_point;
} cpp_t;

static cairo_status_t
_cpp_move_to (void *closure,
        const cairo_point_t *point)
{
  cpp_t *cpp = closure;
  cairo_path_data_t *data = cpp->data;
  double x, y;

  x = _cairo_fixed_to_double (point->x);
  y = _cairo_fixed_to_double (point->y);

  _cairo_gstate_backend_to_user (cpp->gstate, &x, &y);

  data->header.type = CAIRO_PATH_MOVE_TO;
  data->header.length = 2;

  /* We index from 1 to leave room for data->header */
  data[1].point.x = x;
  data[1].point.y = y;

  cpp->data += data->header.length;

  cpp->current_point = *point;

  return CAIRO_STATUS_SUCCESS;
}

static cairo_status_t
_cpp_line_to (void *closure,
        const cairo_point_t *point)
{
  cpp_t *cpp = closure;
  cairo_path_data_t *data = cpp->data;
  double x, y;

  x = _cairo_fixed_to_double (point->x);
  y = _cairo_fixed_to_double (point->y);

  _cairo_gstate_backend_to_user (cpp->gstate, &x, &y);

  data->header.type = CAIRO_PATH_LINE_TO;
  data->header.length = 2;

  /* We index from 1 to leave room for data->header */
  data[1].point.x = x;
  data[1].point.y = y;

  cpp->data += data->header.length;

  cpp->current_point = *point;

  return CAIRO_STATUS_SUCCESS;
}

static cairo_status_t
_cpp_curve_to (void            *closure,
         const cairo_point_t    *p1,
         const cairo_point_t    *p2,
         const cairo_point_t    *p3)
{
  cpp_t *cpp = closure;
  cairo_path_data_t *data = cpp->data;
  double x1, y1;
  double x2, y2;
  double x3, y3;

  x1 = _cairo_fixed_to_double (p1->x);
  y1 = _cairo_fixed_to_double (p1->y);
  _cairo_gstate_backend_to_user (cpp->gstate, &x1, &y1);

  x2 = _cairo_fixed_to_double (p2->x);
  y2 = _cairo_fixed_to_double (p2->y);
  _cairo_gstate_backend_to_user (cpp->gstate, &x2, &y2);

  x3 = _cairo_fixed_to_double (p3->x);
  y3 = _cairo_fixed_to_double (p3->y);
  _cairo_gstate_backend_to_user (cpp->gstate, &x3, &y3);

  data->header.type = CAIRO_PATH_CURVE_TO;
  data->header.length = 4;

  /* We index from 1 to leave room for data->header */
  data[1].point.x = x1;
  data[1].point.y = y1;

  data[2].point.x = x2;
  data[2].point.y = y2;

  data[3].point.x = x3;
  data[3].point.y = y3;

  cpp->data += data->header.length;

  cpp->current_point = *p3;

  return CAIRO_STATUS_SUCCESS;
}

static cairo_status_t
_cpp_close_path (void *closure)
{
  cpp_t *cpp = closure;
  cairo_path_data_t *data = cpp->data;

  data->header.type = CAIRO_PATH_CLOSE_PATH;
  data->header.length = 1;

  cpp->data += data->header.length;

  return CAIRO_STATUS_SUCCESS;
}

static cairo_status_t
_cairo_path_populate (cairo_path_t        *path,
            cairo_path_fixed_t    *path_fixed,
            cairo_gstate_t        *gstate,
            cairo_bool_t         flatten)
{
  cairo_status_t status;
  cpp_t cpp;

  cpp.data = path->data;
  cpp.gstate = gstate;
  cpp.current_point.x = 0;
  cpp.current_point.y = 0;

  if (flatten) {
    double tolerance = _cairo_gstate_get_tolerance (gstate);
    status = _cairo_path_fixed_interpret_flat (path_fixed,
                           CAIRO_DIRECTION_FORWARD,
                           _cpp_move_to,
                           _cpp_line_to,
                           _cpp_close_path,
                           &cpp,
                           tolerance);
  } else {
    status = _cairo_path_fixed_interpret (path_fixed,
                      CAIRO_DIRECTION_FORWARD,
                      _cpp_move_to,
                      _cpp_line_to,
                      _cpp_curve_to,
                      _cpp_close_path,
                      &cpp);
  }

  if (unlikely (status))
    return status;

  /* Sanity check the count */
  assert (cpp.data - path->data == path->num_data);

  return CAIRO_STATUS_SUCCESS;
}

cairo_path_t *
_cairo_path_create_in_error (cairo_status_t status)
{
  cairo_path_t *path;

  /* special case NO_MEMORY so as to avoid allocations */
  if (status == CAIRO_STATUS_NO_MEMORY)
    return (cairo_path_t*) &_cairo_path_nil;

  path = malloc (sizeof (cairo_path_t));
  if (unlikely (path == NULL)) {
    _cairo_error_throw (CAIRO_STATUS_NO_MEMORY);
    return (cairo_path_t*) &_cairo_path_nil;
  }

  path->num_data = 0;
  path->data = NULL;
  path->status = status;

  return path;
}

static cairo_path_t *
_cairo_path_create_internal (cairo_path_fixed_t *path_fixed,
               cairo_gstate_t   *gstate,
               cairo_bool_t     flatten)
{
  cairo_path_t *path;

  path = malloc (sizeof (cairo_path_t));
  if (unlikely (path == NULL)) {
    _cairo_error_throw (CAIRO_STATUS_NO_MEMORY);
    return (cairo_path_t*) &_cairo_path_nil;
  }

  path->num_data = _cairo_path_count (path, path_fixed,
                    _cairo_gstate_get_tolerance (gstate),
                    flatten);
  if (path->num_data < 0) {
    free (path);
    return (cairo_path_t*) &_cairo_path_nil;
  }

  if (path->num_data) {
    path->data = _cairo_malloc_ab (path->num_data,
                     sizeof (cairo_path_data_t));
    if (unlikely (path->data == NULL)) {
      free (path);
      _cairo_error_throw (CAIRO_STATUS_NO_MEMORY);
      return (cairo_path_t*) &_cairo_path_nil;
    }

    path->status = _cairo_path_populate (path, path_fixed,
                       gstate, flatten);
  } else {
    path->data = NULL;
    path->status = CAIRO_STATUS_SUCCESS;
  }

  return path;
}

/**
 * cairo_path_destroy:
 * @path: a path previously returned by either cairo_copy_path() or
 * cairo_copy_path_flat().
 *
 * Immediately releases all memory associated with @path. After a call
 * to cairo_path_destroy() the @path pointer is no longer valid and
 * should not be used further.
 *
 * Note: cairo_path_destroy() should only be called with a
 * pointer to a #cairo_path_t returned by a cairo function. Any path
 * that is created manually (ie. outside of cairo) should be destroyed
 * manually as well.
 **/
void
cairo_path_destroy (cairo_path_t *path)
{
  if (path == NULL || path == &_cairo_path_nil)
    return;

  if (path->data)
    free (path->data);

  free (path);
}

/**
 * _cairo_path_create:
 * @path: a fixed-point, device-space path to be converted and copied
 * @gstate: the current graphics state
 *
 * Creates a user-space #cairo_path_t copy of the given device-space
 * @path. The @gstate parameter provides the inverse CTM for the
 * conversion.
 *
 * Return value: the new copy of the path. If there is insufficient
 * memory a pointer to a special static nil #cairo_path_t will be
 * returned instead with status==%CAIRO_STATUS_NO_MEMORY and
 * data==%NULL.
 **/
cairo_path_t *
_cairo_path_create (cairo_path_fixed_t *path,
          cairo_gstate_t   *gstate)
{
  return _cairo_path_create_internal (path, gstate, FALSE);
}

/**
 * _cairo_path_create_flat:
 * @path: a fixed-point, device-space path to be flattened, converted and copied
 * @gstate: the current graphics state
 *
 * Creates a flattened, user-space #cairo_path_t copy of the given
 * device-space @path. The @gstate parameter provide the inverse CTM
 * for the conversion, as well as the tolerance value to control the
 * accuracy of the flattening.
 *
 * Return value: the flattened copy of the path. If there is insufficient
 * memory a pointer to a special static nil #cairo_path_t will be
 * returned instead with status==%CAIRO_STATUS_NO_MEMORY and
 * data==%NULL.
 **/
cairo_path_t *
_cairo_path_create_flat (cairo_path_fixed_t *path,
             cairo_gstate_t   *gstate)
{
  return _cairo_path_create_internal (path, gstate, TRUE);
}

/**
 * _cairo_path_append_to_context:
 * @path: the path data to be appended
 * @cr: a cairo context
 *
 * Append @path to the current path within @cr.
 *
 * Return value: %CAIRO_STATUS_INVALID_PATH_DATA if the data in @path
 * is invalid, and %CAIRO_STATUS_SUCCESS otherwise.
 **/
cairo_status_t
_cairo_path_append_to_context (const cairo_path_t    *path,
                 cairo_t            *cr)
{
  const cairo_path_data_t *p, *end;
  cairo_fixed_t x1_fixed, y1_fixed;
  cairo_fixed_t x2_fixed, y2_fixed;
  cairo_fixed_t x3_fixed, y3_fixed;
  cairo_matrix_t user_to_backend;
  cairo_status_t status;
  double x, y;

  user_to_backend = cr->gstate->ctm;
  cairo_matrix_multiply (&user_to_backend,
               &user_to_backend,
               &cr->gstate->target->device_transform);

  end = &path->data[path->num_data];
  for (p = &path->data[0]; p < end; p += p->header.length) {
    switch (p->header.type) {
    case CAIRO_PATH_MOVE_TO:
      if (unlikely (p->header.length < 2))
        return _cairo_error (CAIRO_STATUS_INVALID_PATH_DATA);

      x = p[1].point.x, y = p[1].point.y;
      cairo_matrix_transform_point (&user_to_backend, &x, &y);
      x1_fixed = _cairo_fixed_from_double (x);
      y1_fixed = _cairo_fixed_from_double (y);

      status = _cairo_path_fixed_move_to (cr->path, x1_fixed, y1_fixed);
      break;

    case CAIRO_PATH_LINE_TO:
      if (unlikely (p->header.length < 2))
        return _cairo_error (CAIRO_STATUS_INVALID_PATH_DATA);

      x = p[1].point.x, y = p[1].point.y;
      cairo_matrix_transform_point (&user_to_backend, &x, &y);
      x1_fixed = _cairo_fixed_from_double (x);
      y1_fixed = _cairo_fixed_from_double (y);

      status = _cairo_path_fixed_line_to (cr->path, x1_fixed, y1_fixed);
      break;

    case CAIRO_PATH_CURVE_TO:
      if (unlikely (p->header.length < 4))
        return _cairo_error (CAIRO_STATUS_INVALID_PATH_DATA);

      x = p[1].point.x, y = p[1].point.y;
      cairo_matrix_transform_point (&user_to_backend, &x, &y);
      x1_fixed = _cairo_fixed_from_double (x);
      y1_fixed = _cairo_fixed_from_double (y);

      x = p[2].point.x, y = p[2].point.y;
      cairo_matrix_transform_point (&user_to_backend, &x, &y);
      x2_fixed = _cairo_fixed_from_double (x);
      y2_fixed = _cairo_fixed_from_double (y);

      x = p[3].point.x, y = p[3].point.y;
      cairo_matrix_transform_point (&user_to_backend, &x, &y);
      x3_fixed = _cairo_fixed_from_double (x);
      y3_fixed = _cairo_fixed_from_double (y);

      status = _cairo_path_fixed_curve_to (cr->path,
                         x1_fixed, y1_fixed,
                         x2_fixed, y2_fixed,
                         x3_fixed, y3_fixed);
      break;

    case CAIRO_PATH_CLOSE_PATH:
      if (unlikely (p->header.length < 1))
        return _cairo_error (CAIRO_STATUS_INVALID_PATH_DATA);

      status = _cairo_path_fixed_close_path (cr->path);
      break;

    default:
      return _cairo_error (CAIRO_STATUS_INVALID_PATH_DATA);
    }

    if (unlikely (status))
      return status;
  }

  return CAIRO_STATUS_SUCCESS;
}
