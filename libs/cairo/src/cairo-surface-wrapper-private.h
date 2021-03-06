/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CAIRO_SURFACE_WRAPPER_PRIVATE_H
#define CAIRO_SURFACE_WRAPPER_PRIVATE_H

#include "cairo-types-private.h"

CAIRO_BEGIN_DECLS

struct _cairo_surface_wrapper {
  cairo_surface_t *target;

  cairo_bool_t has_extents;
  cairo_rectangle_int_t extents;
};

cairo_private void
_cairo_surface_wrapper_init (cairo_surface_wrapper_t *wrapper,
               cairo_surface_t *target);

cairo_private void
_cairo_surface_wrapper_set_extents (cairo_surface_wrapper_t *wrapper,
                  const cairo_rectangle_int_t *extents);

cairo_private void
_cairo_surface_wrapper_fini (cairo_surface_wrapper_t *wrapper);

cairo_private cairo_status_t
_cairo_surface_wrapper_acquire_source_image (cairo_surface_wrapper_t *wrapper,
                       cairo_image_surface_t  **image_out,
                       void           **image_extra);

cairo_private void
_cairo_surface_wrapper_release_source_image (cairo_surface_wrapper_t *wrapper,
                       cairo_image_surface_t  *image,
                       void           *image_extra);


cairo_private cairo_status_t
_cairo_surface_wrapper_paint (cairo_surface_wrapper_t *wrapper,
                cairo_operator_t     op,
                const cairo_pattern_t *source,
                cairo_clip_t      *clip);

cairo_private cairo_status_t
_cairo_surface_wrapper_mask (cairo_surface_wrapper_t *wrapper,
               cairo_operator_t     op,
               const cairo_pattern_t *source,
               const cairo_pattern_t *mask,
               cairo_clip_t      *clip);

cairo_private cairo_status_t
_cairo_surface_wrapper_stroke (cairo_surface_wrapper_t *wrapper,
                 cairo_operator_t         op,
                 const cairo_pattern_t    *source,
                 cairo_path_fixed_t    *path,
                 const cairo_stroke_style_t    *stroke_style,
                 const cairo_matrix_t        *ctm,
                 const cairo_matrix_t        *ctm_inverse,
                 double             tolerance,
                 cairo_antialias_t     antialias,
                 cairo_clip_t        *clip);

cairo_private cairo_status_t
_cairo_surface_wrapper_fill_stroke (cairo_surface_wrapper_t *wrapper,
                  cairo_operator_t       fill_op,
                  const cairo_pattern_t   *fill_source,
                  cairo_fill_rule_t       fill_rule,
                  double           fill_tolerance,
                  cairo_antialias_t       fill_antialias,
                  cairo_path_fixed_t      *path,
                  cairo_operator_t       stroke_op,
                  const cairo_pattern_t   *stroke_source,
                  const cairo_stroke_style_t  *stroke_style,
                  const cairo_matrix_t      *stroke_ctm,
                  const cairo_matrix_t      *stroke_ctm_inverse,
                  double           stroke_tolerance,
                  cairo_antialias_t       stroke_antialias,
                  cairo_clip_t      *clip);

cairo_private cairo_status_t
_cairo_surface_wrapper_fill (cairo_surface_wrapper_t *wrapper,
               cairo_operator_t     op,
               const cairo_pattern_t *source,
               cairo_path_fixed_t    *path,
               cairo_fill_rule_t     fill_rule,
               double         tolerance,
               cairo_antialias_t     antialias,
               cairo_clip_t    *clip);

cairo_private cairo_status_t
_cairo_surface_wrapper_show_text_glyphs (cairo_surface_wrapper_t *wrapper,
                     cairo_operator_t       op,
                     const cairo_pattern_t      *source,
                     const char          *utf8,
                     int               utf8_len,
                     cairo_glyph_t          *glyphs,
                     int               num_glyphs,
                     const cairo_text_cluster_t *clusters,
                     int               num_clusters,
                     cairo_text_cluster_flags_t  cluster_flags,
                     cairo_scaled_font_t      *scaled_font,
                     cairo_clip_t          *clip);

cairo_private cairo_surface_t *
_cairo_surface_wrapper_create_similar (cairo_surface_wrapper_t *wrapper,
                     cairo_content_t    content,
                     int        width,
                     int        height);
cairo_private cairo_bool_t
_cairo_surface_wrapper_get_extents (cairo_surface_wrapper_t *wrapper,
                  cairo_rectangle_int_t   *extents);

cairo_private void
_cairo_surface_wrapper_get_font_options (cairo_surface_wrapper_t  *wrapper,
                     cairo_font_options_t      *options);

cairo_private cairo_surface_t *
_cairo_surface_wrapper_snapshot (cairo_surface_wrapper_t *wrapper);

cairo_private cairo_bool_t
_cairo_surface_wrapper_has_show_text_glyphs (cairo_surface_wrapper_t *wrapper);

cairo_private cairo_status_t
_cairo_surface_wrapper_flush (cairo_surface_wrapper_t *wrapper);

static inline cairo_bool_t
_cairo_surface_wrapper_is_active (cairo_surface_wrapper_t *wrapper)
{
  return wrapper->target != (cairo_surface_t *) 0;
}

CAIRO_END_DECLS

#endif /* CAIRO_SURFACE_WRAPPER_PRIVATE_H */
