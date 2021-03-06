/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "cairoint.h"

cairo_private void
_cairo_box_from_doubles (cairo_box_t *box,
             double *x1, double *y1,
             double *x2, double *y2)
{
  box->p1.x = _cairo_fixed_from_double (*x1);
  box->p1.y = _cairo_fixed_from_double (*y1);
  box->p2.x = _cairo_fixed_from_double (*x2);
  box->p2.y = _cairo_fixed_from_double (*y2);
}

cairo_private void
_cairo_box_to_doubles (const cairo_box_t *box,
             double *x1, double *y1,
             double *x2, double *y2)
{
  *x1 = _cairo_fixed_to_double (box->p1.x);
  *y1 = _cairo_fixed_to_double (box->p1.y);
  *x2 = _cairo_fixed_to_double (box->p2.x);
  *y2 = _cairo_fixed_to_double (box->p2.y);
}

void
_cairo_box_from_rectangle (cairo_box_t         *box,
               const cairo_rectangle_int_t *rect)
{
  box->p1.x = _cairo_fixed_from_int (rect->x);
  box->p1.y = _cairo_fixed_from_int (rect->y);
  box->p2.x = _cairo_fixed_from_int (rect->x + rect->width);
  box->p2.y = _cairo_fixed_from_int (rect->y + rect->height);
}

void
_cairo_boxes_get_extents (const cairo_box_t *boxes,
              int num_boxes,
              cairo_box_t *extents)
{
  int n;

  assert (num_boxes > 0);
  *extents = *boxes;

  for (n = 1; n < num_boxes; n++) {
    if (boxes[n].p1.x < extents->p1.x)
      extents->p1.x = boxes[n].p1.x;
    if (boxes[n].p2.x > extents->p2.x)
      extents->p2.x = boxes[n].p2.x;

    if (boxes[n].p1.y < extents->p1.y)
      extents->p1.y = boxes[n].p1.y;
    if (boxes[n].p2.y > extents->p2.y)
      extents->p2.y = boxes[n].p2.y;
  }
}

/* This function will return 'true' if the containing_rectangle contains the
 * contained_rectangle, and false otherwise.
 */ 
cairo_bool_t
_cairo_rectangle_contains (const cairo_rectangle_int_t *containing_rectangle,
               const cairo_rectangle_int_t *contained_rectangle)
{
  if (containing_rectangle->x > contained_rectangle->x ||
    containing_rectangle->y > contained_rectangle->y)
    return FALSE;
  
  if (containing_rectangle->x + containing_rectangle->width <
    contained_rectangle->x + contained_rectangle->width ||
    containing_rectangle->y + containing_rectangle->height <
    contained_rectangle->y + contained_rectangle->height)
    return FALSE;

  return TRUE;
}

/* XXX We currently have a confusing mix of boxes and rectangles as
 * exemplified by this function.  A #cairo_box_t is a rectangular area
 * represented by the coordinates of the upper left and lower right
 * corners, expressed in fixed point numbers.  A #cairo_rectangle_int_t is
 * also a rectangular area, but represented by the upper left corner
 * and the width and the height, as integer numbers.
 *
 * This function converts a #cairo_box_t to a #cairo_rectangle_int_t by
 * increasing the area to the nearest integer coordinates.  We should
 * standardize on #cairo_rectangle_fixed_t and #cairo_rectangle_int_t, and
 * this function could be renamed to the more reasonable
 * _cairo_rectangle_fixed_round.
 */

void
_cairo_box_round_to_rectangle (const cairo_box_t   *box,
                 cairo_rectangle_int_t *rectangle)
{
  rectangle->x = _cairo_fixed_integer_floor (box->p1.x);
  rectangle->y = _cairo_fixed_integer_floor (box->p1.y);
  rectangle->width = _cairo_fixed_integer_ceil (box->p2.x) - rectangle->x;
  rectangle->height = _cairo_fixed_integer_ceil (box->p2.y) - rectangle->y;
}

cairo_bool_t
_cairo_rectangle_intersect (cairo_rectangle_int_t *dst,
              const cairo_rectangle_int_t *src)
{
  int x1, y1, x2, y2;

  x1 = MAX (dst->x, src->x);
  y1 = MAX (dst->y, src->y);
  /* Beware the unsigned promotion, fortunately we have bits to spare
   * as (CAIRO_RECT_INT_MAX - CAIRO_RECT_INT_MIN) < UINT_MAX
   */
  x2 = MIN (dst->x + (int) dst->width,  src->x + (int) src->width);
  y2 = MIN (dst->y + (int) dst->height, src->y + (int) src->height);

  if (x1 >= x2 || y1 >= y2) {
    dst->x = 0;
    dst->y = 0;
    dst->width  = 0;
    dst->height = 0;

    return FALSE;
  } else {
    dst->x = x1;
    dst->y = y1;
    dst->width  = x2 - x1;
    dst->height = y2 - y1;

    return TRUE;
  }
}

#define P1x (line->p1.x)
#define P1y (line->p1.y)
#define P2x (line->p2.x)
#define P2y (line->p2.y)
#define B1x (box->p1.x)
#define B1y (box->p1.y)
#define B2x (box->p2.x)
#define B2y (box->p2.y)

/*
 * Check whether any part of line intersects box.  This function essentially
 * computes whether the ray starting at line->p1 in the direction of line->p2
 * intersects the box before it reaches p2.  Normally, this is done
 * by dividing by the lengths of the line projected onto each axis.  Because
 * we're in fixed point, this function does a bit more work to avoid having to
 * do the division -- we don't care about the actual intersection point, so
 * it's of no interest to us.
 */

cairo_bool_t
_cairo_box_intersects_line_segment (cairo_box_t *box, cairo_line_t *line)
{
  cairo_fixed_t t1=0, t2=0, t3=0, t4=0;
  cairo_int64_t t1y, t2y, t3x, t4x;

  cairo_fixed_t xlen, ylen;

  if (_cairo_box_contains_point (box, &line->p1) ||
    _cairo_box_contains_point (box, &line->p2))
    return TRUE;

  xlen = P2x - P1x;
  ylen = P2y - P1y;

  if (xlen) {
    if (xlen > 0) {
      t1 = B1x - P1x;
      t2 = B2x - P1x;
    } else {
      t1 = P1x - B2x;
      t2 = P1x - B1x;
      xlen = - xlen;
    }

    if (t1 > xlen || t2 < 0)
      return FALSE;
  } else {
    /* Fully vertical line -- check that X is in bounds */
    if (P1x < B1x || P1x > B2x)
      return FALSE;
  }

  if (ylen) {
    if (ylen > 0) {
      t3 = B1y - P1y;
      t4 = B2y - P1y;
    } else {
      t3 = P1y - B2y;
      t4 = P1y - B1y;
      ylen = - ylen;
    }

    if (t3 > ylen || t4 < 0)
      return FALSE;
  } else {
    /* Fully horizontal line -- check Y */
    if (P1y < B1y || P1y > B2y)
      return FALSE;
  }

  /* If we had a horizontal or vertical line, then it's already been checked */
  if (P1x == P2x || P1y == P2y)
    return TRUE;

  /* Check overlap.  Note that t1 < t2 and t3 < t4 here. */
  t1y = _cairo_int32x32_64_mul (t1, ylen);
  t2y = _cairo_int32x32_64_mul (t2, ylen);
  t3x = _cairo_int32x32_64_mul (t3, xlen);
  t4x = _cairo_int32x32_64_mul (t4, xlen);

  if (_cairo_int64_lt(t1y, t4x) &&
    _cairo_int64_lt(t3x, t2y))
    return TRUE;

  return FALSE;
}

cairo_bool_t
_cairo_box_contains_point (cairo_box_t *box, const cairo_point_t *point)
{
  if (point->x < box->p1.x || point->x > box->p2.x ||
    point->y < box->p1.y || point->y > box->p2.y)
    return FALSE;
  return TRUE;
}
