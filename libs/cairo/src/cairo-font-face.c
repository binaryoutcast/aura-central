/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "cairoint.h"
#include "cairo-error-private.h"

/**
 * SECTION:cairo-font-face
 * @Title: cairo_font_face_t
 * @Short_Description: Base class for font faces
 * @See_Also: #cairo_scaled_font_t
 *
 * #cairo_font_face_t represents a particular font at a particular weight,
 * slant, and other characteristic but no size, transformation, or size.
 * 
 * Font faces are created using <firstterm>font-backend</firstterm>-specific
 * constructors, typically of the form
 * cairo_<emphasis>backend</emphasis>_font_face_create(), or implicitly
 * using the <firstterm>toy</firstterm> text API by way of 
 * cairo_select_font_face().  The resulting face can be accessed using
 * cairo_get_font_face().
 */

/* #cairo_font_face_t */

const cairo_font_face_t _cairo_font_face_nil = {
  { 0 },                /* hash_entry */
  CAIRO_STATUS_NO_MEMORY,        /* status */
  CAIRO_REFERENCE_COUNT_INVALID,    /* ref_count */
  { 0, 0, 0, NULL },            /* user_data */
  NULL
};

cairo_status_t
_cairo_font_face_set_error (cairo_font_face_t *font_face,
              cairo_status_t   status)
{
  if (status == CAIRO_STATUS_SUCCESS)
    return status;

  /* Don't overwrite an existing error. This preserves the first
   * error, which is the most significant. */
  _cairo_status_set_error (&font_face->status, status);

  return _cairo_error (status);
}

void
_cairo_font_face_init (cairo_font_face_t         *font_face,
             const cairo_font_face_backend_t *backend)
{
  CAIRO_MUTEX_INITIALIZE ();

  font_face->status = CAIRO_STATUS_SUCCESS;
  CAIRO_REFERENCE_COUNT_INIT (&font_face->ref_count, 1);
  font_face->backend = backend;

  _cairo_user_data_array_init (&font_face->user_data);
}

/**
 * cairo_font_face_reference:
 * @font_face: a #cairo_font_face_t, (may be %NULL in which case this
 * function does nothing).
 *
 * Increases the reference count on @font_face by one. This prevents
 * @font_face from being destroyed until a matching call to
 * cairo_font_face_destroy() is made.
 *
 * The number of references to a #cairo_font_face_t can be get using
 * cairo_font_face_get_reference_count().
 *
 * Return value: the referenced #cairo_font_face_t.
 **/
cairo_font_face_t *
cairo_font_face_reference (cairo_font_face_t *font_face)
{
  if (font_face == NULL ||
      CAIRO_REFERENCE_COUNT_IS_INVALID (&font_face->ref_count))
    return font_face;

  /* We would normally assert that we have a reference here but we
   * can't get away with that due to the zombie case as documented
   * in _cairo_ft_font_face_destroy. */

  _cairo_reference_count_inc (&font_face->ref_count);

  return font_face;
}
slim_hidden_def (cairo_font_face_reference);

/**
 * cairo_font_face_destroy:
 * @font_face: a #cairo_font_face_t
 *
 * Decreases the reference count on @font_face by one. If the result
 * is zero, then @font_face and all associated resources are freed.
 * See cairo_font_face_reference().
 **/
void
cairo_font_face_destroy (cairo_font_face_t *font_face)
{
  if (font_face == NULL ||
      CAIRO_REFERENCE_COUNT_IS_INVALID (&font_face->ref_count))
    return;

  assert (CAIRO_REFERENCE_COUNT_HAS_REFERENCE (&font_face->ref_count));

  if (! _cairo_reference_count_dec_and_test (&font_face->ref_count))
    return;

  if (font_face->backend->destroy)
    font_face->backend->destroy (font_face);

  /* We allow resurrection to deal with some memory management for the
   * FreeType backend where cairo_ft_font_face_t and cairo_ft_unscaled_font_t
   * need to effectively mutually reference each other
   */
  if (CAIRO_REFERENCE_COUNT_HAS_REFERENCE (&font_face->ref_count))
    return;

  _cairo_user_data_array_fini (&font_face->user_data);

  free (font_face);
}
slim_hidden_def (cairo_font_face_destroy);

/**
 * cairo_font_face_get_type:
 * @font_face: a font face
 *
 * This function returns the type of the backend used to create
 * a font face. See #cairo_font_type_t for available types.
 *
 * Return value: The type of @font_face.
 *
 * Since: 1.2
 **/
cairo_font_type_t
cairo_font_face_get_type (cairo_font_face_t *font_face)
{
  if (CAIRO_REFERENCE_COUNT_IS_INVALID (&font_face->ref_count))
    return CAIRO_FONT_TYPE_TOY;

  return font_face->backend->type;
}

/**
 * cairo_font_face_get_reference_count:
 * @font_face: a #cairo_font_face_t
 *
 * Returns the current reference count of @font_face.
 *
 * Return value: the current reference count of @font_face.  If the
 * object is a nil object, 0 will be returned.
 *
 * Since: 1.4
 **/
unsigned int
cairo_font_face_get_reference_count (cairo_font_face_t *font_face)
{
  if (font_face == NULL ||
      CAIRO_REFERENCE_COUNT_IS_INVALID (&font_face->ref_count))
    return 0;

  return CAIRO_REFERENCE_COUNT_GET_VALUE (&font_face->ref_count);
}

/**
 * cairo_font_face_status:
 * @font_face: a #cairo_font_face_t
 *
 * Checks whether an error has previously occurred for this
 * font face
 *
 * Return value: %CAIRO_STATUS_SUCCESS or another error such as
 *   %CAIRO_STATUS_NO_MEMORY.
 **/
cairo_status_t
cairo_font_face_status (cairo_font_face_t *font_face)
{
  return font_face->status;
}

/**
 * cairo_font_face_get_user_data:
 * @font_face: a #cairo_font_face_t
 * @key: the address of the #cairo_user_data_key_t the user data was
 * attached to
 *
 * Return user data previously attached to @font_face using the specified
 * key.  If no user data has been attached with the given key this
 * function returns %NULL.
 *
 * Return value: the user data previously attached or %NULL.
 **/
void *
cairo_font_face_get_user_data (cairo_font_face_t       *font_face,
                 const cairo_user_data_key_t *key)
{
  return _cairo_user_data_array_get_data (&font_face->user_data,
                      key);
}
slim_hidden_def (cairo_font_face_get_user_data);

/**
 * cairo_font_face_set_user_data:
 * @font_face: a #cairo_font_face_t
 * @key: the address of a #cairo_user_data_key_t to attach the user data to
 * @user_data: the user data to attach to the font face
 * @destroy: a #cairo_destroy_func_t which will be called when the
 * font face is destroyed or when new user data is attached using the
 * same key.
 *
 * Attach user data to @font_face.  To remove user data from a font face,
 * call this function with the key that was used to set it and %NULL
 * for @data.
 *
 * Return value: %CAIRO_STATUS_SUCCESS or %CAIRO_STATUS_NO_MEMORY if a
 * slot could not be allocated for the user data.
 **/
cairo_status_t
cairo_font_face_set_user_data (cairo_font_face_t       *font_face,
                 const cairo_user_data_key_t *key,
                 void               *user_data,
                 cairo_destroy_func_t      destroy)
{
  if (CAIRO_REFERENCE_COUNT_IS_INVALID (&font_face->ref_count))
    return font_face->status;

  return _cairo_user_data_array_set_data (&font_face->user_data,
                      key, user_data, destroy);
}
slim_hidden_def (cairo_font_face_set_user_data);

void
_cairo_unscaled_font_init (cairo_unscaled_font_t         *unscaled_font,
               const cairo_unscaled_font_backend_t *backend)
{
  CAIRO_REFERENCE_COUNT_INIT (&unscaled_font->ref_count, 1);
  unscaled_font->backend = backend;
}

cairo_unscaled_font_t *
_cairo_unscaled_font_reference (cairo_unscaled_font_t *unscaled_font)
{
  if (unscaled_font == NULL)
    return NULL;

  assert (CAIRO_REFERENCE_COUNT_HAS_REFERENCE (&unscaled_font->ref_count));

  _cairo_reference_count_inc (&unscaled_font->ref_count);

  return unscaled_font;
}

void
_cairo_unscaled_font_destroy (cairo_unscaled_font_t *unscaled_font)
{
  if (unscaled_font == NULL)
    return;

  assert (CAIRO_REFERENCE_COUNT_HAS_REFERENCE (&unscaled_font->ref_count));

  if (! _cairo_reference_count_dec_and_test (&unscaled_font->ref_count))
    return;

  unscaled_font->backend->destroy (unscaled_font);

  free (unscaled_font);
}
