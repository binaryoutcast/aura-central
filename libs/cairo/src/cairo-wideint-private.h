/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CAIRO_WIDEINT_H
#define CAIRO_WIDEINT_H

#include "cairo-wideint-type-private.h"

#include "cairo-compiler-private.h"

/*
 * 64-bit datatypes.  Two separate implementations, one using
 * built-in 64-bit signed/unsigned types another implemented
 * as a pair of 32-bit ints
 */

#define I cairo_private cairo_const

#if !HAVE_UINT64_T

cairo_uquorem64_t I
_cairo_uint64_divrem (cairo_uint64_t num, cairo_uint64_t den);

cairo_uint64_t I    _cairo_uint32_to_uint64 (uint32_t i);
#define            _cairo_uint64_to_uint32(a)  ((a).lo)
cairo_uint64_t I    _cairo_uint64_add (cairo_uint64_t a, cairo_uint64_t b);
cairo_uint64_t I    _cairo_uint64_sub (cairo_uint64_t a, cairo_uint64_t b);
cairo_uint64_t I    _cairo_uint64_mul (cairo_uint64_t a, cairo_uint64_t b);
cairo_uint64_t I    _cairo_uint32x32_64_mul (uint32_t a, uint32_t b);
cairo_uint64_t I    _cairo_uint64_lsl (cairo_uint64_t a, int shift);
cairo_uint64_t I    _cairo_uint64_rsl (cairo_uint64_t a, int shift);
cairo_uint64_t I    _cairo_uint64_rsa (cairo_uint64_t a, int shift);
int         I    _cairo_uint64_lt (cairo_uint64_t a, cairo_uint64_t b);
int         I    _cairo_uint64_cmp (cairo_uint64_t a, cairo_uint64_t b);
int         I    _cairo_uint64_eq (cairo_uint64_t a, cairo_uint64_t b);
cairo_uint64_t I    _cairo_uint64_negate (cairo_uint64_t a);
#define            _cairo_uint64_is_zero(a) ((a).hi == 0 && (a).lo == 0)
#define            _cairo_uint64_negative(a)   (((int32_t) ((a).hi)) < 0)
cairo_uint64_t I    _cairo_uint64_not (cairo_uint64_t a);

#define            _cairo_uint64_to_int64(i)   (i)
#define            _cairo_int64_to_uint64(i)   (i)

cairo_int64_t  I    _cairo_int32_to_int64(int32_t i);
#define            _cairo_int64_to_int32(a)  ((int32_t) _cairo_uint64_to_uint32(a))
#define            _cairo_int64_add(a,b)      _cairo_uint64_add (a,b)
#define            _cairo_int64_sub(a,b)      _cairo_uint64_sub (a,b)
#define            _cairo_int64_mul(a,b)      _cairo_uint64_mul (a,b)
cairo_int64_t  I    _cairo_int32x32_64_mul (int32_t a, int32_t b);
int         I    _cairo_int64_lt (cairo_int64_t a, cairo_int64_t b);
int         I    _cairo_int64_cmp (cairo_int64_t a, cairo_int64_t b);
#define            _cairo_int64_is_zero(a)      _cairo_uint64_is_zero (a)
#define            _cairo_int64_eq(a,b)      _cairo_uint64_eq (a,b)
#define            _cairo_int64_lsl(a,b)      _cairo_uint64_lsl (a,b)
#define            _cairo_int64_rsl(a,b)      _cairo_uint64_rsl (a,b)
#define            _cairo_int64_rsa(a,b)      _cairo_uint64_rsa (a,b)
#define            _cairo_int64_negate(a)      _cairo_uint64_negate(a)
#define            _cairo_int64_negative(a)  (((int32_t) ((a).hi)) < 0)
#define            _cairo_int64_not(a)      _cairo_uint64_not(a)

#else

static inline cairo_uquorem64_t
_cairo_uint64_divrem (cairo_uint64_t num, cairo_uint64_t den)
{
  cairo_uquorem64_t    qr;

  qr.quo = num / den;
  qr.rem = num % den;
  return qr;
}

#define            _cairo_uint32_to_uint64(i)  ((uint64_t) (i))
#define            _cairo_uint64_to_uint32(i)  ((uint32_t) (i))
#define            _cairo_uint64_add(a,b)      ((a) + (b))
#define            _cairo_uint64_sub(a,b)      ((a) - (b))
#define            _cairo_uint64_mul(a,b)      ((a) * (b))
#define            _cairo_uint32x32_64_mul(a,b)    ((uint64_t) (a) * (b))
#define            _cairo_uint64_lsl(a,b)      ((a) << (b))
#define            _cairo_uint64_rsl(a,b)      ((uint64_t) (a) >> (b))
#define            _cairo_uint64_rsa(a,b)      ((uint64_t) ((int64_t) (a) >> (b)))
#define            _cairo_uint64_lt(a,b)      ((a) < (b))
#define         _cairo_uint64_cmp(a,b)     ((a) == (b) ? 0 : (a) < (b) ? -1 : 1)
#define            _cairo_uint64_is_zero(a)  ((a) == 0)
#define            _cairo_uint64_eq(a,b)      ((a) == (b))
#define            _cairo_uint64_negate(a)      ((uint64_t) -((int64_t) (a)))
#define            _cairo_uint64_negative(a)   ((int64_t) (a) < 0)
#define            _cairo_uint64_not(a)      (~(a))

#define            _cairo_uint64_to_int64(i)   ((int64_t) (i))
#define            _cairo_int64_to_uint64(i)   ((uint64_t) (i))

#define            _cairo_int32_to_int64(i)  ((int64_t) (i))
#define            _cairo_int64_to_int32(i)  ((int32_t) (i))
#define            _cairo_int64_add(a,b)      ((a) + (b))
#define            _cairo_int64_sub(a,b)      ((a) - (b))
#define            _cairo_int64_mul(a,b)      ((a) * (b))
#define            _cairo_int32x32_64_mul(a,b) ((int64_t) (a) * (b))
#define            _cairo_int64_lt(a,b)      ((a) < (b))
#define         _cairo_int64_cmp(a,b)     ((a) == (b) ? 0 : (a) < (b) ? -1 : 1)
#define            _cairo_int64_is_zero(a)   ((a) == 0)
#define            _cairo_int64_eq(a,b)      ((a) == (b))
#define            _cairo_int64_lsl(a,b)      ((a) << (b))
#define            _cairo_int64_rsl(a,b)      ((int64_t) ((uint64_t) (a) >> (b)))
#define            _cairo_int64_rsa(a,b)      ((int64_t) (a) >> (b))
#define            _cairo_int64_negate(a)      (-(a))
#define            _cairo_int64_negative(a)  ((a) < 0)
#define            _cairo_int64_not(a)      (~(a))

#endif

/*
 * 64-bit comparisions derived from lt or eq
 */
#define            _cairo_uint64_le(a,b)      (!_cairo_uint64_gt(a,b))
#define            _cairo_uint64_ne(a,b)      (!_cairo_uint64_eq(a,b))
#define            _cairo_uint64_ge(a,b)      (!_cairo_uint64_lt(a,b))
#define            _cairo_uint64_gt(a,b)      _cairo_uint64_lt(b,a)

#define            _cairo_int64_le(a,b)      (!_cairo_int64_gt(a,b))
#define            _cairo_int64_ne(a,b)      (!_cairo_int64_eq(a,b))
#define            _cairo_int64_ge(a,b)      (!_cairo_int64_lt(a,b))
#define            _cairo_int64_gt(a,b)      _cairo_int64_lt(b,a)

/*
 * As the C implementation always computes both, create
 * a function which returns both for the 'native' type as well
 */

static inline cairo_quorem64_t
_cairo_int64_divrem (cairo_int64_t num, cairo_int64_t den)
{
  int            num_neg = _cairo_int64_negative (num);
  int            den_neg = _cairo_int64_negative (den);
  cairo_uquorem64_t    uqr;
  cairo_quorem64_t    qr;

  if (num_neg)
    num = _cairo_int64_negate (num);
  if (den_neg)
    den = _cairo_int64_negate (den);
  uqr = _cairo_uint64_divrem (num, den);
  if (num_neg)
    qr.rem = _cairo_int64_negate (uqr.rem);
  else
    qr.rem = uqr.rem;
  if (num_neg != den_neg)
    qr.quo = (cairo_int64_t) _cairo_int64_negate (uqr.quo);
  else
    qr.quo = (cairo_int64_t) uqr.quo;
  return qr;
}

static inline int32_t
_cairo_int64_32_div (cairo_int64_t num, int32_t den)
{
#if !HAVE_UINT64_T
  return _cairo_int64_to_int32
    (_cairo_int64_divrem (num, _cairo_int32_to_int64 (den)).quo);
#else
  return num / den;
#endif
}

/*
 * 128-bit datatypes.  Again, provide two implementations in
 * case the machine has a native 128-bit datatype.  GCC supports int128_t
 * on ia64
 */

#if !HAVE_UINT128_T

cairo_uint128_t I    _cairo_uint32_to_uint128 (uint32_t i);
cairo_uint128_t I    _cairo_uint64_to_uint128 (cairo_uint64_t i);
#define            _cairo_uint128_to_uint64(a)    ((a).lo)
#define            _cairo_uint128_to_uint32(a)    _cairo_uint64_to_uint32(_cairo_uint128_to_uint64(a))
cairo_uint128_t I    _cairo_uint128_add (cairo_uint128_t a, cairo_uint128_t b);
cairo_uint128_t I    _cairo_uint128_sub (cairo_uint128_t a, cairo_uint128_t b);
cairo_uint128_t I    _cairo_uint128_mul (cairo_uint128_t a, cairo_uint128_t b);
cairo_uint128_t I    _cairo_uint64x64_128_mul (cairo_uint64_t a, cairo_uint64_t b);
cairo_uint128_t I    _cairo_uint128_lsl (cairo_uint128_t a, int shift);
cairo_uint128_t I    _cairo_uint128_rsl (cairo_uint128_t a, int shift);
cairo_uint128_t I    _cairo_uint128_rsa (cairo_uint128_t a, int shift);
int        I    _cairo_uint128_lt (cairo_uint128_t a, cairo_uint128_t b);
int        I    _cairo_uint128_cmp (cairo_uint128_t a, cairo_uint128_t b);
int        I    _cairo_uint128_eq (cairo_uint128_t a, cairo_uint128_t b);
#define            _cairo_uint128_is_zero(a) (_cairo_uint64_is_zero ((a).hi) && _cairo_uint64_is_zero ((a).lo))
cairo_uint128_t I    _cairo_uint128_negate (cairo_uint128_t a);
#define            _cairo_uint128_negative(a)  (_cairo_uint64_negative(a.hi))
cairo_uint128_t I    _cairo_uint128_not (cairo_uint128_t a);

#define            _cairo_uint128_to_int128(i)    (i)
#define            _cairo_int128_to_uint128(i)    (i)

cairo_int128_t  I    _cairo_int32_to_int128 (int32_t i);
cairo_int128_t  I    _cairo_int64_to_int128 (cairo_int64_t i);
#define            _cairo_int128_to_int64(a)   ((cairo_int64_t) (a).lo)
#define            _cairo_int128_to_int32(a)   _cairo_int64_to_int32(_cairo_int128_to_int64(a))
#define            _cairo_int128_add(a,b)      _cairo_uint128_add(a,b)
#define            _cairo_int128_sub(a,b)      _cairo_uint128_sub(a,b)
#define            _cairo_int128_mul(a,b)      _cairo_uint128_mul(a,b)
cairo_int128_t I _cairo_int64x64_128_mul (cairo_int64_t a, cairo_int64_t b);
#define         _cairo_int64x32_128_mul(a, b) _cairo_int64x64_128_mul(a, _cairo_int32_to_int64(b))
#define            _cairo_int128_lsl(a,b)      _cairo_uint128_lsl(a,b)
#define            _cairo_int128_rsl(a,b)      _cairo_uint128_rsl(a,b)
#define            _cairo_int128_rsa(a,b)      _cairo_uint128_rsa(a,b)
int         I    _cairo_int128_lt (cairo_int128_t a, cairo_int128_t b);
int        I    _cairo_int128_cmp (cairo_int128_t a, cairo_int128_t b);
#define            _cairo_int128_is_zero(a)  _cairo_uint128_is_zero (a)
#define            _cairo_int128_eq(a,b)      _cairo_uint128_eq (a,b)
#define            _cairo_int128_negate(a)      _cairo_uint128_negate(a)
#define            _cairo_int128_negative(a)   (_cairo_uint128_negative(a))
#define            _cairo_int128_not(a)      _cairo_uint128_not(a)

#else    /* !HAVE_UINT128_T */

#define            _cairo_uint32_to_uint128(i) ((uint128_t) (i))
#define            _cairo_uint64_to_uint128(i) ((uint128_t) (i))
#define            _cairo_uint128_to_uint64(i) ((uint64_t) (i))
#define            _cairo_uint128_to_uint32(i) ((uint32_t) (i))
#define            _cairo_uint128_add(a,b)      ((a) + (b))
#define            _cairo_uint128_sub(a,b)      ((a) - (b))
#define            _cairo_uint128_mul(a,b)      ((a) * (b))
#define            _cairo_uint64x64_128_mul(a,b)    ((uint128_t) (a) * (b))
#define            _cairo_uint128_lsl(a,b)      ((a) << (b))
#define            _cairo_uint128_rsl(a,b)      ((uint128_t) (a) >> (b))
#define            _cairo_uint128_rsa(a,b)      ((uint128_t) ((int128_t) (a) >> (b)))
#define            _cairo_uint128_lt(a,b)      ((a) < (b))
#define            _cairo_uint128_cmp(a,b)      ((a) == (b) ? 0 : (a) < (b) ? -1 : 1)
#define            _cairo_uint128_is_zero(a)   ((a) == 0)
#define            _cairo_uint128_eq(a,b)      ((a) == (b))
#define            _cairo_uint128_negate(a)  ((uint128_t) -((int128_t) (a)))
#define            _cairo_uint128_negative(a)  ((int128_t) (a) < 0)
#define            _cairo_uint128_not(a)      (~(a))

#define            _cairo_uint128_to_int128(i) ((int128_t) (i))
#define            _cairo_int128_to_uint128(i) ((uint128_t) (i))

#define            _cairo_int32_to_int128(i)   ((int128_t) (i))
#define            _cairo_int64_to_int128(i)   ((int128_t) (i))
#define            _cairo_int128_to_int64(i)   ((int64_t) (i))
#define            _cairo_int128_to_int32(i)   ((int32_t) (i))
#define            _cairo_int128_add(a,b)      ((a) + (b))
#define            _cairo_int128_sub(a,b)      ((a) - (b))
#define            _cairo_int128_mul(a,b)      ((a) * (b))
#define            _cairo_int64x64_128_mul(a,b) ((int128_t) (a) * (b))
#define         _cairo_int64x32_128_mul(a, b) _cairo_int64x64_128_mul(a, _cairo_int32_to_int64(b))
#define            _cairo_int128_lt(a,b)      ((a) < (b))
#define            _cairo_int128_cmp(a,b)      ((a) == (b) ? 0 : (a) < (b) ? -1 : 1)
#define            _cairo_int128_is_zero(a)  ((a) == 0)
#define            _cairo_int128_eq(a,b)      ((a) == (b))
#define            _cairo_int128_lsl(a,b)      ((a) << (b))
#define            _cairo_int128_rsl(a,b)      ((int128_t) ((uint128_t) (a) >> (b)))
#define            _cairo_int128_rsa(a,b)      ((int128_t) (a) >> (b))
#define            _cairo_int128_negate(a)      (-(a))
#define            _cairo_int128_negative(a)   ((a) < 0)
#define            _cairo_int128_not(a)      (~(a))

#endif    /* HAVE_UINT128_T */

cairo_uquorem128_t I
_cairo_uint128_divrem (cairo_uint128_t num, cairo_uint128_t den);

cairo_quorem128_t I
_cairo_int128_divrem (cairo_int128_t num, cairo_int128_t den);

cairo_uquorem64_t I
_cairo_uint_96by64_32x64_divrem (cairo_uint128_t num,
                 cairo_uint64_t  den);

cairo_quorem64_t I
_cairo_int_96by64_32x64_divrem (cairo_int128_t num,
                cairo_int64_t  den);

#define            _cairo_uint128_le(a,b)      (!_cairo_uint128_gt(a,b))
#define            _cairo_uint128_ne(a,b)      (!_cairo_uint128_eq(a,b))
#define            _cairo_uint128_ge(a,b)      (!_cairo_uint128_lt(a,b))
#define            _cairo_uint128_gt(a,b)      _cairo_uint128_lt(b,a)

#define            _cairo_int128_le(a,b)      (!_cairo_int128_gt(a,b))
#define            _cairo_int128_ne(a,b)      (!_cairo_int128_eq(a,b))
#define            _cairo_int128_ge(a,b)      (!_cairo_int128_lt(a,b))
#define            _cairo_int128_gt(a,b)      _cairo_int128_lt(b,a)

#undef I

#endif /* CAIRO_WIDEINT_H */
