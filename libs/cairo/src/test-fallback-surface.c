/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/* This isn't a "real" surface, but just something to be used by the
 * test suite to test a mythical backend that uses nothing but
 * fallbacks.
 *
 * The defining feature of this backend is that it has as many %NULL
 * backend function entries as possible. The ones that aren't %NULL are
 * simply those that must be implemented to have working fallbacks.
 * (Except for create_similar---fallbacks would work fine without
 * that---I implemented it here in order to create as many surfaces as
 * possible of type test_fallback_surface_t during the test suite
 * run).
 *
 * It's possible that this code might serve as a good starting point
 * for someone working on bringing up a new backend, starting with the
 * minimal all-fallbacks approach and working up gradually from
 * there.
 */

#include "cairoint.h"

#include "test-fallback-surface.h"
#include "cairo-error-private.h"

typedef struct _test_fallback_surface {
  cairo_surface_t base;

  /* This is a cairo_image_surface to hold the actual contents. */
  cairo_surface_t *backing;
} test_fallback_surface_t;

static const cairo_surface_backend_t test_fallback_surface_backend;

slim_hidden_proto (_cairo_test_fallback_surface_create);

cairo_surface_t *
_cairo_test_fallback_surface_create (cairo_content_t    content,
                 int        width,
                 int        height)
{
  test_fallback_surface_t *surface;
  cairo_surface_t *backing;

  backing = _cairo_image_surface_create_with_content (content, width, height);
  if (cairo_surface_status (backing))
    return backing;

  surface = malloc (sizeof (test_fallback_surface_t));
  if (unlikely (surface == NULL)) {
    cairo_surface_destroy (backing);
    return _cairo_surface_create_in_error (_cairo_error (CAIRO_STATUS_NO_MEMORY));
  }

  _cairo_surface_init (&surface->base,
             &test_fallback_surface_backend,
             NULL, /* device */
             content);

  surface->backing = backing;

  return &surface->base;
}
slim_hidden_def (_cairo_test_fallback_surface_create);

static cairo_surface_t *
_test_fallback_surface_create_similar (void        *abstract_surface,
                     cairo_content_t     content,
                     int         width,
                     int         height)
{
  assert (CAIRO_CONTENT_VALID (content));

  return _cairo_test_fallback_surface_create (content,
                      width, height);
}

static cairo_status_t
_test_fallback_surface_finish (void *abstract_surface)
{
  test_fallback_surface_t *surface = abstract_surface;

  cairo_surface_destroy (surface->backing);

  return CAIRO_STATUS_SUCCESS;
}

static cairo_status_t
_test_fallback_surface_acquire_source_image (void       *abstract_surface,
                       cairo_image_surface_t **image_out,
                       void         **image_extra)
{
  test_fallback_surface_t *surface = abstract_surface;

  return _cairo_surface_acquire_source_image (surface->backing,
                        image_out, image_extra);
}

static void
_test_fallback_surface_release_source_image (void       *abstract_surface,
                       cairo_image_surface_t    *image,
                       void          *image_extra)
{
  test_fallback_surface_t *surface = abstract_surface;

  _cairo_surface_release_source_image (surface->backing,
                     image, image_extra);
}

static cairo_status_t
_test_fallback_surface_acquire_dest_image (void               *abstract_surface,
                       cairo_rectangle_int_t   *interest_rect,
                       cairo_image_surface_t  **image_out,
                       cairo_rectangle_int_t   *image_rect_out,
                       void              **image_extra)
{
  test_fallback_surface_t *surface = abstract_surface;

  return _cairo_surface_acquire_dest_image (surface->backing,
                        interest_rect,
                        image_out,
                        image_rect_out,
                        image_extra);
}

static void
_test_fallback_surface_release_dest_image (void               *abstract_surface,
                       cairo_rectangle_int_t   *interest_rect,
                       cairo_image_surface_t   *image,
                       cairo_rectangle_int_t   *image_rect,
                       void               *image_extra)
{
  test_fallback_surface_t *surface = abstract_surface;

  _cairo_surface_release_dest_image (surface->backing,
                     interest_rect,
                     image,
                     image_rect,
                     image_extra);
}

static cairo_status_t
_test_fallback_surface_clone_similar (void          *abstract_surface,
                    cairo_surface_t   *src,
                    int          src_x,
                    int          src_y,
                    int          width,
                    int          height,
                    int         *clone_offset_x,
                    int         *clone_offset_y,
                    cairo_surface_t  **clone_out)
{
  test_fallback_surface_t *surface = abstract_surface;

  if (src->backend == surface->base.backend) {
    *clone_offset_x = 0;
    *clone_offset_y = 0;
    *clone_out = cairo_surface_reference (src);

    return CAIRO_STATUS_SUCCESS;
  }

  return CAIRO_INT_STATUS_UNSUPPORTED;
}

static cairo_bool_t
_test_fallback_surface_get_extents (void          *abstract_surface,
                  cairo_rectangle_int_t *rectangle)
{
  test_fallback_surface_t *surface = abstract_surface;

  return _cairo_surface_get_extents (surface->backing, rectangle);
}

static const cairo_surface_backend_t test_fallback_surface_backend = {
  CAIRO_INTERNAL_SURFACE_TYPE_TEST_FALLBACK,
  _test_fallback_surface_create_similar,
  _test_fallback_surface_finish,
  _test_fallback_surface_acquire_source_image,
  _test_fallback_surface_release_source_image,
  _test_fallback_surface_acquire_dest_image,
  _test_fallback_surface_release_dest_image,
  _test_fallback_surface_clone_similar,
  NULL, /* composite */
  NULL, /* fill_rectangles */
  NULL, /* composite_trapezoids */
  NULL, /* create_span_renderer */
  NULL, /* check_span_renderer */
  NULL, /* copy_page */
  NULL, /* show_page */
  _test_fallback_surface_get_extents,
  NULL, /* old_show_glyphs */
  NULL, /* get_font_options */
  NULL, /* flush */
  NULL, /* mark_dirty_rectangle */
  NULL, /* scaled_font_fini */
  NULL, /* scaled_glyph_fini */
  NULL, /* paint */
  NULL, /* mask */
  NULL, /* stroke */
  NULL, /* fill */
  NULL, /* show_glyphs */
  NULL  /* snapshot */
};
