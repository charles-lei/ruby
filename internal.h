/**********************************************************************

  internal.h -

  $Author$
  created at: Tue May 17 11:42:20 JST 2011

  Copyright (C) 2011 Yukihiro Matsumoto

**********************************************************************/

#ifndef RUBY_INTERNAL_H
#define RUBY_INTERNAL_H 1

#include "ruby.h"
#include "ruby/encoding.h"
#include "ruby/io.h"

#if defined(__cplusplus)
extern "C" {
#if 0
} /* satisfy cc-mode */
#endif
#endif

/* likely */
#if __GNUC__ >= 3
#define LIKELY(x)   (__builtin_expect(!!(x), 1))
#define UNLIKELY(x) (__builtin_expect(!!(x), 0))
#else /* __GNUC__ >= 3 */
#define LIKELY(x)   (x)
#define UNLIKELY(x) (x)
#endif /* __GNUC__ >= 3 */

#ifndef __has_attribute
# define __has_attribute(x) 0
#endif

#if __has_attribute(unused)
#define UNINITIALIZED_VAR(x) x __attribute__((unused))
#elif defined(__GNUC__) && __GNUC__ >= 3
#define UNINITIALIZED_VAR(x) x = x
#else
#define UNINITIALIZED_VAR(x) x
#endif

#if __has_attribute(warn_unused_result)
#define WARN_UNUSED_RESULT(x) x __attribute__((warn_unused_result))
#elif defined(__GNUC__) && (__GNUC__ * 1000 + __GNUC_MINOR__) >= 3004
#define WARN_UNUSED_RESULT(x) x __attribute__((warn_unused_result))
#else
#define WARN_UNUSED_RESULT(x) x
#endif

#ifdef HAVE_VALGRIND_MEMCHECK_H
# include <valgrind/memcheck.h>
# ifndef VALGRIND_MAKE_MEM_DEFINED
#  define VALGRIND_MAKE_MEM_DEFINED(p, n) VALGRIND_MAKE_READABLE((p), (n))
# endif
# ifndef VALGRIND_MAKE_MEM_UNDEFINED
#  define VALGRIND_MAKE_MEM_UNDEFINED(p, n) VALGRIND_MAKE_WRITABLE((p), (n))
# endif
#else
# define VALGRIND_MAKE_MEM_DEFINED(p, n) 0
# define VALGRIND_MAKE_MEM_UNDEFINED(p, n) 0
#endif

#define numberof(array) ((int)(sizeof(array) / sizeof((array)[0])))

#define GCC_VERSION_SINCE(major, minor, patchlevel) \
  (defined(__GNUC__) && !defined(__INTEL_COMPILER) && \
   ((__GNUC__ > (major)) ||  \
    (__GNUC__ == (major) && __GNUC_MINOR__ > (minor)) || \
    (__GNUC__ == (major) && __GNUC_MINOR__ == (minor) && __GNUC_PATCHLEVEL__ >= (patchlevel))))

#ifndef __has_feature
# define __has_feature(x) 0
#endif

#ifndef __has_extension
# define __has_extension __has_feature
#endif

#if GCC_VERSION_SINCE(4, 6, 0) || __has_extension(c_static_assert)
# define STATIC_ASSERT(name, expr) _Static_assert(expr, #name ": " #expr)
#else
# define STATIC_ASSERT(name, expr) typedef int static_assert_##name##_check[1 - 2*!(expr)]
#endif

#define SIGNED_INTEGER_TYPE_P(int_type) (0 > ((int_type)0)-1)
#define SIGNED_INTEGER_MAX(sint_type) \
  (sint_type) \
  ((((sint_type)1) << (sizeof(sint_type) * CHAR_BIT - 2)) | \
  ((((sint_type)1) << (sizeof(sint_type) * CHAR_BIT - 2)) - 1))
#define SIGNED_INTEGER_MIN(sint_type) (-SIGNED_INTEGER_MAX(sint_type)-1)
#define UNSIGNED_INTEGER_MAX(uint_type) (~(uint_type)0)

#if SIGNEDNESS_OF_TIME_T < 0	/* signed */
# define TIMET_MAX SIGNED_INTEGER_MAX(time_t)
# define TIMET_MIN SIGNED_INTEGER_MIN(time_t)
#elif SIGNEDNESS_OF_TIME_T > 0	/* unsigned */
# define TIMET_MAX UNSIGNED_INTEGER_MAX(time_t)
# define TIMET_MIN ((time_t)0)
#endif
#define TIMET_MAX_PLUS_ONE (2*(double)(TIMET_MAX/2+1))

#define MUL_OVERFLOW_SIGNED_INTEGER_P(a, b, min, max) ( \
    (a) == 0 ? 0 : \
    (a) == -1 ? (b) < -(max) : \
    (a) > 0 ? \
      ((b) > 0 ? (max) / (a) < (b) : (min) / (a) > (b)) : \
      ((b) > 0 ? (min) / (a) < (b) : (max) / (a) > (b)))
#define MUL_OVERFLOW_FIXNUM_P(a, b) MUL_OVERFLOW_SIGNED_INTEGER_P(a, b, FIXNUM_MIN, FIXNUM_MAX)
#define MUL_OVERFLOW_LONG_P(a, b) MUL_OVERFLOW_SIGNED_INTEGER_P(a, b, LONG_MIN, LONG_MAX)
#define MUL_OVERFLOW_INT_P(a, b) MUL_OVERFLOW_SIGNED_INTEGER_P(a, b, INT_MIN, INT_MAX)

#ifndef swap16
# ifdef HAVE_BUILTIN___BUILTIN_BSWAP16
#  define swap16(x) __builtin_bswap16(x)
# endif
#endif

#ifndef swap16
# define swap16(x)      ((uint16_t)((((x)&0xFF)<<8) | (((x)>>8)&0xFF)))
#endif

#ifndef swap32
# ifdef HAVE_BUILTIN___BUILTIN_BSWAP32
#  define swap32(x) __builtin_bswap32(x)
# endif
#endif

#ifndef swap32
# define swap32(x)      ((uint32_t)((((x)&0xFF)<<24)    \
                        |(((x)>>24)&0xFF)       \
                        |(((x)&0x0000FF00)<<8)  \
                        |(((x)&0x00FF0000)>>8)  ))
#endif

#ifndef swap64
# ifdef HAVE_BUILTIN___BUILTIN_BSWAP64
#  define swap64(x) __builtin_bswap64(x)
# endif
#endif

#ifndef swap64
# ifdef HAVE_INT64_T
#  define byte_in_64bit(n) ((uint64_t)0xff << (n))
#  define swap64(x)       ((uint64_t)((((x)&byte_in_64bit(0))<<56)      \
                           |(((x)>>56)&0xFF)                    \
                           |(((x)&byte_in_64bit(8))<<40)        \
                           |(((x)&byte_in_64bit(48))>>40)       \
                           |(((x)&byte_in_64bit(16))<<24)       \
                           |(((x)&byte_in_64bit(40))>>24)       \
                           |(((x)&byte_in_64bit(24))<<8)        \
                           |(((x)&byte_in_64bit(32))>>8)))
# endif
#endif

static inline int
nlz_int(unsigned int x)
{
#if defined(HAVE_BUILTIN___BUILTIN_CLZ)
    if (x == 0) return SIZEOF_INT * CHAR_BIT;
    return __builtin_clz(x);
#else
    unsigned int y;
# if 64 < SIZEOF_INT * CHAR_BIT
    int n = 128;
# elif 32 < SIZEOF_INT * CHAR_BIT
    int n = 64;
# else
    int n = 32;
# endif
# if 64 < SIZEOF_INT * CHAR_BIT
    y = x >> 64; if (y) {n -= 64; x = y;}
# endif
# if 32 < SIZEOF_INT * CHAR_BIT
    y = x >> 32; if (y) {n -= 32; x = y;}
# endif
    y = x >> 16; if (y) {n -= 16; x = y;}
    y = x >>  8; if (y) {n -=  8; x = y;}
    y = x >>  4; if (y) {n -=  4; x = y;}
    y = x >>  2; if (y) {n -=  2; x = y;}
    y = x >>  1; if (y) {return n - 2;}
    return (int)(n - x);
#endif
}

static inline int
nlz_long(unsigned long x)
{
#if defined(HAVE_BUILTIN___BUILTIN_CLZL)
    if (x == 0) return SIZEOF_LONG * CHAR_BIT;
    return __builtin_clzl(x);
#else
    unsigned long y;
# if 64 < SIZEOF_LONG * CHAR_BIT
    int n = 128;
# elif 32 < SIZEOF_LONG * CHAR_BIT
    int n = 64;
# else
    int n = 32;
# endif
# if 64 < SIZEOF_LONG * CHAR_BIT
    y = x >> 64; if (y) {n -= 64; x = y;}
# endif
# if 32 < SIZEOF_LONG * CHAR_BIT
    y = x >> 32; if (y) {n -= 32; x = y;}
# endif
    y = x >> 16; if (y) {n -= 16; x = y;}
    y = x >>  8; if (y) {n -=  8; x = y;}
    y = x >>  4; if (y) {n -=  4; x = y;}
    y = x >>  2; if (y) {n -=  2; x = y;}
    y = x >>  1; if (y) {return n - 2;}
    return (int)(n - x);
#endif
}

#ifdef HAVE_LONG_LONG
static inline int
nlz_long_long(unsigned LONG_LONG x)
{
#if defined(HAVE_BUILTIN___BUILTIN_CLZLL)
    if (x == 0) return SIZEOF_LONG_LONG * CHAR_BIT;
    return __builtin_clzll(x);
#else
    unsigned LONG_LONG y;
# if 64 < SIZEOF_LONG_LONG * CHAR_BIT
    int n = 128;
# elif 32 < SIZEOF_LONG_LONG * CHAR_BIT
    int n = 64;
# else
    int n = 32;
# endif
# if 64 < SIZEOF_LONG_LONG * CHAR_BIT
    y = x >> 64; if (y) {n -= 64; x = y;}
# endif
# if 32 < SIZEOF_LONG_LONG * CHAR_BIT
    y = x >> 32; if (y) {n -= 32; x = y;}
# endif
    y = x >> 16; if (y) {n -= 16; x = y;}
    y = x >>  8; if (y) {n -=  8; x = y;}
    y = x >>  4; if (y) {n -=  4; x = y;}
    y = x >>  2; if (y) {n -=  2; x = y;}
    y = x >>  1; if (y) {return n - 2;}
    return (int)(n - x);
#endif
}
#endif

#ifdef HAVE_UINT128_T
static inline int
nlz_int128(uint128_t x)
{
    uint128_t y;
    int n = 128;
    y = x >> 64; if (y) {n -= 64; x = y;}
    y = x >> 32; if (y) {n -= 32; x = y;}
    y = x >> 16; if (y) {n -= 16; x = y;}
    y = x >>  8; if (y) {n -=  8; x = y;}
    y = x >>  4; if (y) {n -=  4; x = y;}
    y = x >>  2; if (y) {n -=  2; x = y;}
    y = x >>  1; if (y) {return n - 2;}
    return (int)(n - x);
}
#endif

/*
 * This behaves different from C99 for negative arguments.
 * Note that div may overflow fixnum.
 */
static inline void
rb_divmod(long x, long y, long *divp, long *modp) {
    /* assume / and % comply C99.
     * ldiv(3) won't be inlined by GCC and clang.
     * I expect / and % are compiled as single idiv.
     */
    long div = x / y;
    long mod = x % y;
    if (y > 0 ? mod < 0 : mod > 0) {
	mod += y;
	div -= 1;
    }
    if (divp) *divp = div;
    if (modp) *modp = mod;
}

/* div() for Ruby
 * This behaves different from C99 for negative arguments.
 * Note that div may overflow fixnum
 */
static inline long
rb_div(long x, long y) {
    long div;
    rb_divmod(x, y, &div, NULL);
    return div;
}

/* mod() for Ruby
 * This behaves different from C99 for negative arguments.
 */
static inline long
rb_mod(long x, long y) {
    long mod;
    rb_divmod(x, y, NULL, &mod);
    return mod;
}

#if defined(HAVE_UINT128_T)
#   define bit_length(x) \
    (sizeof(x) <= SIZEOF_INT ? SIZEOF_INT * CHAR_BIT - nlz_int((unsigned int)(x)) : \
     sizeof(x) <= SIZEOF_LONG ? SIZEOF_LONG * CHAR_BIT - nlz_long((unsigned long)(x)) : \
     sizeof(x) <= SIZEOF_LONG_LONG ? SIZEOF_LONG_LONG * CHAR_BIT - nlz_long_long((unsigned LONG_LONG)(x)) : \
     SIZEOF_INT128_T * CHAR_BIT - nlz_int128((uint128_t)(x)))
#elif defined(HAVE_LONG_LONG)
#   define bit_length(x) \
    (sizeof(x) <= SIZEOF_INT ? SIZEOF_INT * CHAR_BIT - nlz_int((unsigned int)(x)) : \
     sizeof(x) <= SIZEOF_LONG ? SIZEOF_LONG * CHAR_BIT - nlz_long((unsigned long)(x)) : \
     SIZEOF_LONG_LONG * CHAR_BIT - nlz_long_long((unsigned LONG_LONG)(x)))
#else
#   define bit_length(x) \
    (sizeof(x) <= SIZEOF_INT ? SIZEOF_INT * CHAR_BIT - nlz_int((unsigned int)(x)) : \
     SIZEOF_LONG * CHAR_BIT - nlz_long((unsigned long)(x)))
#endif

#ifndef BDIGIT
# if SIZEOF_INT*2 <= SIZEOF_LONG_LONG
#  define BDIGIT unsigned int
#  define SIZEOF_BDIGIT SIZEOF_INT
#  define BDIGIT_DBL unsigned LONG_LONG
#  define BDIGIT_DBL_SIGNED LONG_LONG
#  define PRI_BDIGIT_PREFIX ""
#  define PRI_BDIGIT_DBL_PREFIX PRI_LL_PREFIX
# elif SIZEOF_INT*2 <= SIZEOF_LONG
#  define BDIGIT unsigned int
#  define SIZEOF_BDIGIT SIZEOF_INT
#  define BDIGIT_DBL unsigned long
#  define BDIGIT_DBL_SIGNED long
#  define PRI_BDIGIT_PREFIX ""
#  define PRI_BDIGIT_DBL_PREFIX "l"
# elif SIZEOF_SHORT*2 <= SIZEOF_LONG
#  define BDIGIT unsigned short
#  define SIZEOF_BDIGIT SIZEOF_SHORT
#  define BDIGIT_DBL unsigned long
#  define BDIGIT_DBL_SIGNED long
#  define PRI_BDIGIT_PREFIX "h"
#  define PRI_BDIGIT_DBL_PREFIX "l"
# else
#  define BDIGIT unsigned short
#  define SIZEOF_BDIGIT (SIZEOF_LONG/2)
#  define SIZEOF_ACTUAL_BDIGIT SIZEOF_LONG
#  define BDIGIT_DBL unsigned long
#  define BDIGIT_DBL_SIGNED long
#  define PRI_BDIGIT_PREFIX "h"
#  define PRI_BDIGIT_DBL_PREFIX "l"
# endif
#endif
#ifndef SIZEOF_ACTUAL_BDIGIT
# define SIZEOF_ACTUAL_BDIGIT SIZEOF_BDIGIT
#endif

#ifdef PRI_BDIGIT_PREFIX
# define PRIdBDIGIT PRI_BDIGIT_PREFIX"d"
# define PRIiBDIGIT PRI_BDIGIT_PREFIX"i"
# define PRIoBDIGIT PRI_BDIGIT_PREFIX"o"
# define PRIuBDIGIT PRI_BDIGIT_PREFIX"u"
# define PRIxBDIGIT PRI_BDIGIT_PREFIX"x"
# define PRIXBDIGIT PRI_BDIGIT_PREFIX"X"
#endif

#ifdef PRI_BDIGIT_DBL_PREFIX
# define PRIdBDIGIT_DBL PRI_BDIGIT_DBL_PREFIX"d"
# define PRIiBDIGIT_DBL PRI_BDIGIT_DBL_PREFIX"i"
# define PRIoBDIGIT_DBL PRI_BDIGIT_DBL_PREFIX"o"
# define PRIuBDIGIT_DBL PRI_BDIGIT_DBL_PREFIX"u"
# define PRIxBDIGIT_DBL PRI_BDIGIT_DBL_PREFIX"x"
# define PRIXBDIGIT_DBL PRI_BDIGIT_DBL_PREFIX"X"
#endif

#define BIGNUM_EMBED_LEN_NUMBITS 3
#ifndef BIGNUM_EMBED_LEN_MAX
# if (SIZEOF_VALUE*3/SIZEOF_ACTUAL_BDIGIT) < (1 << BIGNUM_EMBED_LEN_NUMBITS)-1
#   define BIGNUM_EMBED_LEN_MAX (SIZEOF_VALUE*3/SIZEOF_ACTUAL_BDIGIT)
# else
#   define BIGNUM_EMBED_LEN_MAX ((1 << BIGNUM_EMBED_LEN_NUMBITS)-1)
# endif
#endif

struct RBignum {
    struct RBasic basic;
    union {
        struct {
            size_t len;
            BDIGIT *digits;
        } heap;
        BDIGIT ary[BIGNUM_EMBED_LEN_MAX];
    } as;
};
#define BIGNUM_SIGN_BIT FL_USER1
/* sign: positive:1, negative:0 */
#define BIGNUM_SIGN(b) ((RBASIC(b)->flags & BIGNUM_SIGN_BIT) != 0)
#define BIGNUM_SET_SIGN(b,sign) \
  ((sign) ? (RBASIC(b)->flags |= BIGNUM_SIGN_BIT) \
          : (RBASIC(b)->flags &= ~BIGNUM_SIGN_BIT))
#define BIGNUM_POSITIVE_P(b) BIGNUM_SIGN(b)
#define BIGNUM_NEGATIVE_P(b) (!BIGNUM_SIGN(b))
#define BIGNUM_NEGATE(b) (RBASIC(b)->flags ^= BIGNUM_SIGN_BIT)

#define BIGNUM_EMBED_FLAG FL_USER2
#define BIGNUM_EMBED_LEN_MASK (FL_USER5|FL_USER4|FL_USER3)
#define BIGNUM_EMBED_LEN_SHIFT (FL_USHIFT+BIGNUM_EMBED_LEN_NUMBITS)
#define BIGNUM_LEN(b) \
    ((RBASIC(b)->flags & BIGNUM_EMBED_FLAG) ? \
     (long)((RBASIC(b)->flags >> BIGNUM_EMBED_LEN_SHIFT) & \
            (BIGNUM_EMBED_LEN_MASK >> BIGNUM_EMBED_LEN_SHIFT)) : \
     RBIGNUM(b)->as.heap.len)
/* LSB:BIGNUM_DIGITS(b)[0], MSB:BIGNUM_DIGITS(b)[BIGNUM_LEN(b)-1] */
#define BIGNUM_DIGITS(b) \
    ((RBASIC(b)->flags & BIGNUM_EMBED_FLAG) ? \
     RBIGNUM(b)->as.ary : \
     RBIGNUM(b)->as.heap.digits)
#define BIGNUM_LENINT(b) rb_long2int(BIGNUM_LEN(b))

#define RBIGNUM(obj) (R_CAST(RBignum)(obj))

struct RRational {
    struct RBasic basic;
    const VALUE num;
    const VALUE den;
};

#define RRATIONAL(obj) (R_CAST(RRational)(obj))
#define RRATIONAL_SET_NUM(rat, n) RB_OBJ_WRITE((rat), &((struct RRational *)(rat))->num,(n))
#define RRATIONAL_SET_DEN(rat, d) RB_OBJ_WRITE((rat), &((struct RRational *)(rat))->den,(d))

struct RFloat {
    struct RBasic basic;
    double float_value;
};

#define RFLOAT(obj)  (R_CAST(RFloat)(obj))

struct RComplex {
    struct RBasic basic;
    const VALUE real;
    const VALUE imag;
};

#define RCOMPLEX(obj) (R_CAST(RComplex)(obj))

#ifdef RCOMPLEX_SET_REAL        /* shortcut macro for internal only */
#undef RCOMPLEX_SET_REAL
#undef RCOMPLEX_SET_REAL
#define RCOMPLEX_SET_REAL(cmp, r) RB_OBJ_WRITE((cmp), &((struct RComplex *)(cmp))->real,(r))
#define RCOMPLEX_SET_IMAG(cmp, i) RB_OBJ_WRITE((cmp), &((struct RComplex *)(cmp))->imag,(i))
#endif

struct RHash {
    struct RBasic basic;
    struct st_table *ntbl;      /* possibly 0 */
    int iter_lev;
    const VALUE ifnone;
};

#define RHASH(obj)   (R_CAST(RHash)(obj))

#ifdef RHASH_ITER_LEV
#undef RHASH_ITER_LEV
#undef RHASH_IFNONE
#undef RHASH_SIZE
#define RHASH_ITER_LEV(h) (RHASH(h)->iter_lev)
#define RHASH_IFNONE(h) (RHASH(h)->ifnone)
#define RHASH_SIZE(h) (RHASH(h)->ntbl ? (st_index_t)RHASH(h)->ntbl->num_entries : 0)
#endif

/* missing/setproctitle.c */
#ifndef HAVE_SETPROCTITLE
extern void ruby_init_setproctitle(int argc, char *argv[]);
#endif

/* class.c */

struct rb_deprecated_classext_struct {
    char conflict[sizeof(VALUE) * 3];
};

struct rb_subclass_entry;
typedef struct rb_subclass_entry rb_subclass_entry_t;

struct rb_subclass_entry {
    VALUE klass;
    rb_subclass_entry_t *next;
};

#if defined(HAVE_LONG_LONG)
typedef unsigned LONG_LONG rb_serial_t;
#define SERIALT2NUM ULL2NUM
#elif defined(HAVE_UINT64_T)
typedef uint64_t rb_serial_t;
#define SERIALT2NUM SIZET2NUM
#else
typedef unsigned long rb_serial_t;
#define SERIALT2NUM ULONG2NUM
#endif

struct rb_classext_struct {
    struct st_table *iv_index_tbl;
    struct st_table *iv_tbl;
    struct rb_id_table *const_tbl;
    struct rb_id_table *callable_m_tbl;
    rb_subclass_entry_t *subclasses;
    rb_subclass_entry_t **parent_subclasses;
    /**
     * In the case that this is an `ICLASS`, `module_subclasses` points to the link
     * in the module's `subclasses` list that indicates that the klass has been
     * included. Hopefully that makes sense.
     */
    rb_subclass_entry_t **module_subclasses;
    rb_serial_t class_serial;
    const VALUE origin_;
    VALUE refined_class;
    rb_alloc_func_t allocator;
};

typedef struct rb_classext_struct rb_classext_t;

#undef RClass
struct RClass {
    struct RBasic basic;
    VALUE super;
    rb_classext_t *ptr;
    struct rb_id_table *m_tbl;
};

void rb_class_subclass_add(VALUE super, VALUE klass);
void rb_class_remove_from_super_subclasses(VALUE);
int rb_singleton_class_internal_p(VALUE sklass);

#define RCLASS_EXT(c) (RCLASS(c)->ptr)
#define RCLASS_IV_TBL(c) (RCLASS_EXT(c)->iv_tbl)
#define RCLASS_CONST_TBL(c) (RCLASS_EXT(c)->const_tbl)
#define RCLASS_M_TBL(c) (RCLASS(c)->m_tbl)
#define RCLASS_CALLABLE_M_TBL(c) (RCLASS_EXT(c)->callable_m_tbl)
#define RCLASS_IV_INDEX_TBL(c) (RCLASS_EXT(c)->iv_index_tbl)
#define RCLASS_ORIGIN(c) (RCLASS_EXT(c)->origin_)
#define RCLASS_REFINED_CLASS(c) (RCLASS_EXT(c)->refined_class)
#define RCLASS_SERIAL(c) (RCLASS_EXT(c)->class_serial)

#define RICLASS_IS_ORIGIN FL_USER5

static inline void
RCLASS_SET_ORIGIN(VALUE klass, VALUE origin)
{
    RB_OBJ_WRITE(klass, &RCLASS_ORIGIN(klass), origin);
    if (klass != origin) FL_SET(origin, RICLASS_IS_ORIGIN);
}

#undef RCLASS_SUPER
static inline VALUE
RCLASS_SUPER(VALUE klass)
{
    return RCLASS(klass)->super;
}

static inline VALUE
RCLASS_SET_SUPER(VALUE klass, VALUE super)
{
    if (super) {
	rb_class_remove_from_super_subclasses(klass);
	rb_class_subclass_add(super, klass);
    }
    RB_OBJ_WRITE(klass, &RCLASS(klass)->super, super);
    return super;
}
/* IMEMO: Internal memo object */

#ifndef IMEMO_DEBUG
#define IMEMO_DEBUG 0
#endif

struct RIMemo {
    VALUE flags;
    VALUE v0;
    VALUE v1;
    VALUE v2;
    VALUE v3;
};

enum imemo_type {
    imemo_none = 0,
    imemo_cref = 1,
    imemo_svar = 2,
    imemo_throw_data = 3,
    imemo_ifunc = 4,
    imemo_memo = 5,
    imemo_ment = 6,
    imemo_iseq = 7,
    imemo_mask = 0x07
};

static inline enum imemo_type
imemo_type(VALUE imemo)
{
    return (RBASIC(imemo)->flags >> FL_USHIFT) & imemo_mask;
}

/* FL_USER0 to FL_USER2 is for type */
#define IMEMO_FL_USHIFT (FL_USHIFT + 3)
#define IMEMO_FL_USER0 FL_USER3
#define IMEMO_FL_USER1 FL_USER4
#define IMEMO_FL_USER2 FL_USER5
#define IMEMO_FL_USER3 FL_USER6
#define IMEMO_FL_USER4 FL_USER7

/* CREF in method.h */

/* SVAR */

struct vm_svar {
    VALUE flags;
    const VALUE cref_or_me;
    const VALUE lastline;
    const VALUE backref;
    const VALUE others;
};

/* THROW_DATA */

struct vm_throw_data {
    VALUE flags;
    VALUE reserved;
    const VALUE throw_obj;
    const struct rb_control_frame_struct *catch_frame;
    VALUE throw_state;
};

#define THROW_DATA_P(err) RB_TYPE_P((err), T_IMEMO)

/* IFUNC */

struct vm_ifunc {
    VALUE flags;
    VALUE reserved;
    VALUE (*func)(ANYARGS);
    const void *data;
    ID id;
};

#define IFUNC_NEW(a, b, c) ((struct vm_ifunc *)rb_imemo_new(imemo_ifunc, (VALUE)(a), (VALUE)(b), (VALUE)(c), 0))

/* MEMO */

struct MEMO {
    VALUE flags;
    VALUE reserved;
    const VALUE v1;
    const VALUE v2;
    union {
	long cnt;
	long state;
	const VALUE value;
	VALUE (*func)(ANYARGS);
    } u3;
};

#define MEMO_V1_SET(m, v) RB_OBJ_WRITE((memo), &(memo)->v1, (v))
#define MEMO_V2_SET(m, v) RB_OBJ_WRITE((memo), &(memo)->v2, (v))

#define MEMO_CAST(m) ((struct MEMO *)m)

#define MEMO_NEW(a, b, c) ((struct MEMO *)rb_imemo_new(imemo_memo, (VALUE)(a), (VALUE)(b), (VALUE)(c), 0))

#define roomof(x, y) (((x) + (y) - 1) / (y))
#define type_roomof(x, y) roomof(sizeof(x), sizeof(y))
#define MEMO_FOR(type, value) ((type *)RARRAY_PTR(value))
#define NEW_MEMO_FOR(type, value) \
  ((value) = rb_ary_tmp_new_fill(type_roomof(type, VALUE)), MEMO_FOR(type, value))

#define STRING_P(s) (RB_TYPE_P((s), T_STRING) && CLASS_OF(s) == rb_cString)

enum {
    cmp_opt_Fixnum,
    cmp_opt_String,
    cmp_optimizable_count
};

struct cmp_opt_data {
    int opt_methods;
    int opt_inited;
};

#define CMP_OPTIMIZABLE_BIT(type) (1U << TOKEN_PASTE(cmp_opt_,type))
#define CMP_OPTIMIZABLE(data, type) \
    (((data).opt_inited & CMP_OPTIMIZABLE_BIT(type)) ? \
     ((data).opt_methods & CMP_OPTIMIZABLE_BIT(type)) : \
     (((data).opt_inited |= CMP_OPTIMIZABLE_BIT(type)), \
      rb_method_basic_definition_p(TOKEN_PASTE(rb_c,type), id_cmp) && \
      ((data).opt_methods |= CMP_OPTIMIZABLE_BIT(type))))

#define OPTIMIZED_CMP(a, b, data) \
    ((FIXNUM_P(a) && FIXNUM_P(b) && CMP_OPTIMIZABLE(data, Fixnum)) ? \
     (((long)a > (long)b) ? 1 : ((long)a < (long)b) ? -1 : 0) : \
     (STRING_P(a) && STRING_P(b) && CMP_OPTIMIZABLE(data, String)) ? \
     rb_str_cmp(a, b) : \
     rb_cmpint(rb_funcallv(a, id_cmp, 1, &b), a, b))

/* ment is in method.h */

/* global variable */

struct rb_global_entry {
    struct rb_global_variable *var;
    ID id;
};

struct rb_global_entry *rb_global_entry(ID);
VALUE rb_gvar_get(struct rb_global_entry *);
VALUE rb_gvar_set(struct rb_global_entry *, VALUE);
VALUE rb_gvar_defined(struct rb_global_entry *);

struct vtm; /* defined by timev.h */

/* array.c */
VALUE rb_ary_last(int, const VALUE *, VALUE);
void rb_ary_set_len(VALUE, long);
void rb_ary_delete_same(VALUE, VALUE);
VALUE rb_ary_tmp_new_fill(long capa);
VALUE rb_ary_at(VALUE, VALUE);
size_t rb_ary_memsize(VALUE);
#ifdef __GNUC__
#define rb_ary_new_from_args(n, ...) \
    __extension__ ({ \
	const VALUE args_to_new_ary[] = {__VA_ARGS__}; \
	if (__builtin_constant_p(n)) { \
	    STATIC_ASSERT(rb_ary_new_from_args, numberof(args_to_new_ary) == (n)); \
	} \
	rb_ary_new_from_values(numberof(args_to_new_ary), args_to_new_ary); \
    })
#endif

/* bignum.c */
extern const char ruby_digitmap[];
VALUE rb_big_fdiv(VALUE x, VALUE y);
VALUE rb_big_uminus(VALUE x);
VALUE rb_integer_float_cmp(VALUE x, VALUE y);
VALUE rb_integer_float_eq(VALUE x, VALUE y);

/* class.c */
VALUE rb_class_boot(VALUE);
VALUE rb_class_inherited(VALUE, VALUE);
VALUE rb_make_metaclass(VALUE, VALUE);
VALUE rb_include_class_new(VALUE, VALUE);
void rb_class_foreach_subclass(VALUE klass, void (*f)(VALUE, VALUE), VALUE);
void rb_class_detach_subclasses(VALUE);
void rb_class_detach_module_subclasses(VALUE);
void rb_class_remove_from_module_subclasses(VALUE);
VALUE rb_obj_methods(int argc, const VALUE *argv, VALUE obj);
VALUE rb_obj_protected_methods(int argc, const VALUE *argv, VALUE obj);
VALUE rb_obj_private_methods(int argc, const VALUE *argv, VALUE obj);
VALUE rb_obj_public_methods(int argc, const VALUE *argv, VALUE obj);
int rb_obj_basic_to_s_p(VALUE);
VALUE rb_special_singleton_class(VALUE);
VALUE rb_singleton_class_clone_and_attach(VALUE obj, VALUE attach);
VALUE rb_singleton_class_get(VALUE obj);
void Init_class_hierarchy(void);

int rb_class_has_methods(VALUE c);

/* compar.c */
VALUE rb_invcmp(VALUE, VALUE);

/* compile.c */
int rb_dvar_defined(ID);
int rb_local_defined(ID);
int rb_parse_in_eval(void);
int rb_parse_in_main(void);
const char * rb_insns_name(int i);
VALUE rb_insns_name_array(void);

/* cont.c */
VALUE rb_obj_is_fiber(VALUE);
void rb_fiber_reset_root_local_storage(VALUE);
void ruby_register_rollback_func_for_ensure(VALUE (*ensure_func)(ANYARGS), VALUE (*rollback_func)(ANYARGS));

/* debug.c */
PRINTF_ARGS(void ruby_debug_printf(const char*, ...), 1, 2);

/* dmyext.c */
void Init_enc(void);
void Init_ext(void);

/* encoding.c */
ID rb_id_encoding(void);
void rb_gc_mark_encodings(void);
rb_encoding *rb_enc_get_from_index(int index);
rb_encoding *rb_enc_check_str(VALUE str1, VALUE str2);
int rb_encdb_replicate(const char *alias, const char *orig);
int rb_encdb_alias(const char *alias, const char *orig);
int rb_encdb_dummy(const char *name);
void rb_encdb_declare(const char *name);
void rb_enc_set_base(const char *name, const char *orig);
int rb_enc_set_dummy(int index);
void rb_encdb_set_unicode(int index);
int rb_data_is_encoding(VALUE obj);

/* enum.c */
VALUE rb_f_send(int argc, VALUE *argv, VALUE recv);
VALUE rb_nmin_run(VALUE obj, VALUE num, int by, int rev, int ary);

/* error.c */
extern VALUE rb_eEAGAIN;
extern VALUE rb_eEWOULDBLOCK;
extern VALUE rb_eEINPROGRESS;
NORETURN(PRINTF_ARGS(void rb_compile_bug(const char*, int, const char*, ...), 3, 4));
NORETURN(PRINTF_ARGS(void rb_compile_bug_str(VALUE file, int line, const char *fmt, ...), 3, 4));
PRINTF_ARGS(void rb_compile_error_str(VALUE file, int line, void *enc, const char *fmt, ...), 4, 5);
VALUE rb_check_backtrace(VALUE);
NORETURN(void rb_async_bug_errno(const char *,int));
const char *rb_builtin_type_name(int t);
const char *rb_builtin_class_name(VALUE x);
PRINTF_ARGS(void rb_enc_warn(rb_encoding *enc, const char *fmt, ...), 2, 3);
PRINTF_ARGS(void rb_enc_warning(rb_encoding *enc, const char *fmt, ...), 2, 3);
PRINTF_ARGS(void rb_sys_enc_warning(rb_encoding *enc, const char *fmt, ...), 2, 3);
VALUE rb_name_err_new(VALUE mesg, VALUE recv, VALUE method);
#define rb_name_err_raise_str(mesg, recv, name) \
    rb_exc_raise(rb_name_err_new(mesg, recv, name))
#define rb_name_err_raise(mesg, recv, name) \
    rb_name_err_raise_str(rb_fstring_cstr(mesg), (recv), (name))

/* eval.c */
VALUE rb_refinement_module_get_refined_class(VALUE module);

/* eval_error.c */
void ruby_error_print(void);
VALUE rb_get_backtrace(VALUE info);

/* eval_jump.c */
void rb_call_end_proc(VALUE data);
void rb_mark_end_proc(void);

/* file.c */
VALUE rb_home_dir_of(VALUE user, VALUE result);
VALUE rb_default_home_dir(VALUE result);
VALUE rb_realpath_internal(VALUE basedir, VALUE path, int strict);
void rb_file_const(const char*, VALUE);
int rb_file_load_ok(const char *);
VALUE rb_file_expand_path_fast(VALUE, VALUE);
VALUE rb_file_expand_path_internal(VALUE, VALUE, int, int, VALUE);
VALUE rb_get_path_check_to_string(VALUE, int);
VALUE rb_get_path_check_convert(VALUE, VALUE, int);
void Init_File(void);
int ruby_is_fd_loadable(int fd);

#ifdef RUBY_FUNCTION_NAME_STRING
# if defined __GNUC__ && __GNUC__ >= 4
#   pragma GCC visibility push(default)
# endif
NORETURN(void rb_sys_fail_path_in(const char *func_name, VALUE path));
NORETURN(void rb_syserr_fail_path_in(const char *func_name, int err, VALUE path));
# if defined __GNUC__ && __GNUC__ >= 4
#   pragma GCC visibility pop
# endif
# define rb_sys_fail_path(path) rb_sys_fail_path_in(RUBY_FUNCTION_NAME_STRING, path)
# define rb_syserr_fail_path(err, path) rb_syserr_fail_path_in(RUBY_FUNCTION_NAME_STRING, (err), (path))
#else
# define rb_sys_fail_path(path) rb_sys_fail_str(path)
# define rb_syserr_fail_path(err, path) rb_syserr_fail_str((err), (path))
#endif

/* gc.c */
extern VALUE *ruby_initial_gc_stress_ptr;
extern int ruby_disable_gc;
void Init_heap(void);
void *ruby_mimmalloc(size_t size);
void ruby_mimfree(void *ptr);
void rb_objspace_set_event_hook(const rb_event_flag_t event);
#if USE_RGENGC
void rb_gc_writebarrier_remember(VALUE obj);
#else
#define rb_gc_writebarrier_remember(obj) 0
#endif
void ruby_gc_set_params(int safe_level);
void rb_copy_wb_protected_attribute(VALUE dest, VALUE obj);

#if defined(HAVE_MALLOC_USABLE_SIZE) || defined(HAVE_MALLOC_SIZE) || defined(_WIN32)
#define ruby_sized_xrealloc(ptr, new_size, old_size) ruby_xrealloc(ptr, new_size)
#define ruby_sized_xrealloc2(ptr, new_count, element_size, old_count) ruby_xrealloc(ptr, new_count, element_size)
#define ruby_sized_xfree(ptr, size) ruby_xfree(ptr)
#define SIZED_REALLOC_N(var,type,n,old_n) REALLOC_N(var, type, n)
#else
void *ruby_sized_xrealloc(void *ptr, size_t new_size, size_t old_size) RUBY_ATTR_ALLOC_SIZE((2));
void *ruby_sized_xrealloc2(void *ptr, size_t new_count, size_t element_size, size_t old_count) RUBY_ATTR_ALLOC_SIZE((2, 3));
void ruby_sized_xfree(void *x, size_t size);
#define SIZED_REALLOC_N(var,type,n,old_n) ((var)=(type*)ruby_sized_xrealloc((char*)(var), (n) * sizeof(type), (old_n) * sizeof(type)))
#endif

void rb_gc_resurrect(VALUE ptr);

/* optimized version of NEWOBJ() */
#undef NEWOBJF_OF
#undef RB_NEWOBJ_OF
#define RB_NEWOBJ_OF(obj,type,klass,flags) \
  type *(obj) = (type*)(((flags) & FL_WB_PROTECTED) ? \
			rb_wb_protected_newobj_of(klass, (flags) & ~FL_WB_PROTECTED) : \
			rb_wb_unprotected_newobj_of(klass, flags))
#define NEWOBJ_OF(obj,type,klass,flags) RB_NEWOBJ_OF(obj,type,klass,flags)

/* hash.c */
struct st_table *rb_hash_tbl_raw(VALUE hash);
VALUE rb_hash_has_key(VALUE hash, VALUE key);
VALUE rb_hash_default_value(VALUE hash, VALUE key);
VALUE rb_hash_set_default_proc(VALUE hash, VALUE proc);
long rb_objid_hash(st_index_t index);
st_table *rb_init_identtable(void);
st_table *rb_init_identtable_with_size(st_index_t size);

#define RHASH_TBL_RAW(h) rb_hash_tbl_raw(h)
VALUE rb_hash_keys(VALUE hash);
VALUE rb_hash_values(VALUE hash);
VALUE rb_hash_rehash(VALUE hash);
#define HASH_DELETED  FL_USER1
#define HASH_PROC_DEFAULT FL_USER2

/* inits.c */
void rb_call_inits(void);

/* io.c */
const char *ruby_get_inplace_mode(void);
void ruby_set_inplace_mode(const char *);
ssize_t rb_io_bufread(VALUE io, void *buf, size_t size);
void rb_stdio_set_default_encoding(void);
VALUE rb_io_flush_raw(VALUE, int);
size_t rb_io_memsize(const rb_io_t *);

/* load.c */
VALUE rb_get_load_path(void);
VALUE rb_get_expanded_load_path(void);
int rb_require_internal(VALUE fname, int safe);
NORETURN(void rb_load_fail(VALUE, const char*));

/* loadpath.c */
extern const char ruby_exec_prefix[];
extern const char ruby_initial_load_paths[];

/* localeinit.c */
int Init_enc_set_filesystem_encoding(void);

/* math.c */
VALUE rb_math_atan2(VALUE, VALUE);
VALUE rb_math_cos(VALUE);
VALUE rb_math_cosh(VALUE);
VALUE rb_math_exp(VALUE);
VALUE rb_math_hypot(VALUE, VALUE);
VALUE rb_math_log(int argc, const VALUE *argv);
VALUE rb_math_sin(VALUE);
VALUE rb_math_sinh(VALUE);
#if 0
VALUE rb_math_sqrt(VALUE);
#endif

/* newline.c */
void Init_newline(void);

/* numeric.c */
int rb_num_to_uint(VALUE val, unsigned int *ret);
VALUE ruby_num_interval_step_size(VALUE from, VALUE to, VALUE step, int excl);
int ruby_float_step(VALUE from, VALUE to, VALUE step, int excl);
double ruby_float_mod(double x, double y);
int rb_num_negative_p(VALUE);
VALUE rb_int_succ(VALUE num);
VALUE rb_int_pred(VALUE num);
VALUE rb_dbl_hash(double d);
VALUE rb_fix_plus(VALUE x, VALUE y);

#if USE_FLONUM
#define RUBY_BIT_ROTL(v, n) (((v) << (n)) | ((v) >> ((sizeof(v) * 8) - n)))
#define RUBY_BIT_ROTR(v, n) (((v) >> (n)) | ((v) << ((sizeof(v) * 8) - n)))
#endif

static inline double
rb_float_flonum_value(VALUE v)
{
#if USE_FLONUM
    if (v != (VALUE)0x8000000000000002) { /* LIKELY */
	union {
	    double d;
	    VALUE v;
	} t;

	VALUE b63 = (v >> 63);
	/* e: xx1... -> 011... */
	/*    xx0... -> 100... */
	/*      ^b63           */
	t.v = RUBY_BIT_ROTR((2 - b63) | (v & ~0x03), 3);
	return t.d;
    }
#endif
    return 0.0;
}

static inline double
rb_float_noflonum_value(VALUE v)
{
    return ((struct RFloat *)v)->float_value;
}

static inline double
rb_float_value_inline(VALUE v)
{
    if (FLONUM_P(v)) {
	return rb_float_flonum_value(v);
    }
    return rb_float_noflonum_value(v);
}

static inline VALUE
rb_float_new_inline(double d)
{
#if USE_FLONUM
    union {
	double d;
	VALUE v;
    } t;
    int bits;

    t.d = d;
    bits = (int)((VALUE)(t.v >> 60) & 0x7);
    /* bits contains 3 bits of b62..b60. */
    /* bits - 3 = */
    /*   b011 -> b000 */
    /*   b100 -> b001 */

    if (t.v != 0x3000000000000000 /* 1.72723e-77 */ &&
	!((bits-3) & ~0x01)) {
	return (RUBY_BIT_ROTL(t.v, 3) & ~(VALUE)0x01) | 0x02;
    }
    else if (t.v == (VALUE)0) {
	/* +0.0 */
	return 0x8000000000000002;
    }
    /* out of range */
#endif
    return rb_float_new_in_heap(d);
}

#define rb_float_value(v) rb_float_value_inline(v)
#define rb_float_new(d)   rb_float_new_inline(d)

/* object.c */
void rb_obj_copy_ivar(VALUE dest, VALUE obj);
VALUE rb_obj_equal(VALUE obj1, VALUE obj2);
VALUE rb_class_search_ancestor(VALUE klass, VALUE super);
NORETURN(void rb_undefined_alloc(VALUE klass));
double rb_num_to_dbl(VALUE val);
VALUE rb_obj_dig(int argc, VALUE *argv, VALUE self, VALUE notfound);

struct RBasicRaw {
    VALUE flags;
    VALUE klass;
};

#define RBASIC_CLEAR_CLASS(obj)        (((struct RBasicRaw *)((VALUE)(obj)))->klass = 0)
#define RBASIC_SET_CLASS_RAW(obj, cls) (((struct RBasicRaw *)((VALUE)(obj)))->klass = (cls))
#define RBASIC_SET_CLASS(obj, cls)     do { \
    VALUE _obj_ = (obj); \
    RB_OBJ_WRITE(_obj_, &((struct RBasicRaw *)(_obj_))->klass, cls); \
} while (0)

/* parse.y */
#ifndef USE_SYMBOL_GC
#define USE_SYMBOL_GC 1
#endif
VALUE rb_parser_get_yydebug(VALUE);
VALUE rb_parser_set_yydebug(VALUE, VALUE);
int rb_is_const_name(VALUE name);
int rb_is_class_name(VALUE name);
int rb_is_global_name(VALUE name);
int rb_is_instance_name(VALUE name);
int rb_is_attrset_name(VALUE name);
int rb_is_local_name(VALUE name);
int rb_is_method_name(VALUE name);
int rb_is_junk_name(VALUE name);
int rb_is_const_sym(VALUE sym);
int rb_is_class_sym(VALUE sym);
int rb_is_global_sym(VALUE sym);
int rb_is_instance_sym(VALUE sym);
int rb_is_attrset_sym(VALUE sym);
int rb_is_local_sym(VALUE sym);
int rb_is_method_sym(VALUE sym);
int rb_is_junk_sym(VALUE sym);
ID rb_make_internal_id(void);
void rb_gc_free_dsymbol(VALUE);
ID rb_id_attrget(ID id);

/* proc.c */
VALUE rb_proc_location(VALUE self);
st_index_t rb_hash_proc(st_index_t hash, VALUE proc);
int rb_block_arity(void);
VALUE rb_func_proc_new(rb_block_call_func_t func, VALUE val);
VALUE rb_func_lambda_new(rb_block_call_func_t func, VALUE val);

/* process.c */
#define RB_MAX_GROUPS (65536)

struct rb_execarg {
    union {
        struct {
            VALUE shell_script;
        } sh;
        struct {
            VALUE command_name;
            VALUE command_abspath; /* full path string or nil */
            VALUE argv_str;
            VALUE argv_buf;
        } cmd;
    } invoke;
    VALUE redirect_fds;
    VALUE envp_str;
    VALUE envp_buf;
    VALUE dup2_tmpbuf;
    unsigned use_shell : 1;
    unsigned pgroup_given : 1;
    unsigned umask_given : 1;
    unsigned unsetenv_others_given : 1;
    unsigned unsetenv_others_do : 1;
    unsigned close_others_given : 1;
    unsigned close_others_do : 1;
    unsigned chdir_given : 1;
    unsigned new_pgroup_given : 1;
    unsigned new_pgroup_flag : 1;
    unsigned uid_given : 1;
    unsigned gid_given : 1;
    rb_pid_t pgroup_pgid; /* asis(-1), new pgroup(0), specified pgroup (0<V). */
    VALUE rlimit_limits; /* Qfalse or [[rtype, softlim, hardlim], ...] */
    mode_t umask_mask;
    rb_uid_t uid;
    rb_gid_t gid;
    int close_others_maxhint;
    VALUE fd_dup2;
    VALUE fd_close;
    VALUE fd_open;
    VALUE fd_dup2_child;
    VALUE env_modification; /* Qfalse or [[k1,v1], ...] */
    VALUE chdir_dir;
};

/* argv_str contains extra two elements.
 * The beginning one is for /bin/sh used by exec_with_sh.
 * The last one for terminating NULL used by execve.
 * See rb_exec_fillarg() in process.c. */
#define ARGVSTR2ARGC(argv_str) (RSTRING_LEN(argv_str) / sizeof(char *) - 2)
#define ARGVSTR2ARGV(argv_str) ((char **)RSTRING_PTR(argv_str) + 1)

rb_pid_t rb_fork_ruby(int *status);
void rb_last_status_clear(void);

/* rational.c */
VALUE rb_lcm(VALUE x, VALUE y);
VALUE rb_rational_reciprocal(VALUE x);
VALUE rb_cstr_to_rat(const char *, int);

/* re.c */
VALUE rb_reg_compile(VALUE str, int options, const char *sourcefile, int sourceline);
VALUE rb_reg_check_preprocess(VALUE);
long rb_reg_search0(VALUE, VALUE, long, int, int);
void rb_backref_set_string(VALUE string, long pos, long len);
int rb_match_count(VALUE match);
int rb_match_nth_defined(int nth, VALUE match);

/* signal.c */
extern int ruby_enable_coredump;
int rb_get_next_signal(void);
int rb_sigaltstack_size(void);

/* strftime.c */
#ifdef RUBY_ENCODING_H
size_t rb_strftime_timespec(char *s, size_t maxsize, const char *format, rb_encoding *enc,
	const struct vtm *vtm, struct timespec *ts, int gmt);
size_t rb_strftime(char *s, size_t maxsize, const char *format, rb_encoding *enc,
            const struct vtm *vtm, VALUE timev, int gmt);
#endif

/* string.c */
void Init_frozen_strings(void);
VALUE rb_fstring(VALUE);
VALUE rb_fstring_new(const char *ptr, long len);
#define rb_fstring_lit(str) rb_fstring_new((str), rb_strlen_lit(str))
#define rb_fstring_literal(str) rb_fstring_lit(str)
VALUE rb_fstring_cstr(const char *str);
#if defined(__GNUC__) && !defined(__PCC__)
#define rb_fstring_cstr(str) __extension__ (	\
{						\
    (__builtin_constant_p(str)) ?		\
	rb_fstring_new((str), (long)strlen(str)) : \
	rb_fstring_cstr(str); \
})
#endif
#ifdef RUBY_ENCODING_H
VALUE rb_fstring_enc_new(const char *ptr, long len, rb_encoding *enc);
#define rb_fstring_enc_lit(str, enc) rb_fstring_enc_new((str), rb_strlen_lit(str), (enc))
#define rb_fstring_enc_literal(str, enc) rb_fstring_enc_lit(str, enc)
VALUE rb_fstring_enc_cstr(const char *ptr, rb_encoding *enc);
#if defined(__GNUC__) && !defined(__PCC__)
#define rb_fstring_enc_cstr(str, enc) __extension__ ( \
{						\
    (__builtin_constant_p(str)) ?		\
	rb_fstring_enc_new((str), (long)strlen(str), (enc)) : \
	rb_fstring_enc_cstr(str, enc); \
})
#endif
#endif
int rb_str_buf_cat_escaped_char(VALUE result, unsigned int c, int unicode_p);
int rb_str_symname_p(VALUE);
VALUE rb_str_quote_unprintable(VALUE);
VALUE rb_id_quote_unprintable(ID);
#define QUOTE(str) rb_str_quote_unprintable(str)
#define QUOTE_ID(id) rb_id_quote_unprintable(id)
void rb_str_fill_terminator(VALUE str, const int termlen);
VALUE rb_str_locktmp_ensure(VALUE str, VALUE (*func)(VALUE), VALUE arg);
#ifdef RUBY_ENCODING_H
VALUE rb_external_str_with_enc(VALUE str, rb_encoding *eenc);
VALUE rb_str_cat_conv_enc_opts(VALUE newstr, long ofs, const char *ptr, long len,
			       rb_encoding *from, int ecflags, VALUE ecopts);
#endif
#define STR_NOEMBED      FL_USER1
#define STR_SHARED       FL_USER2 /* = ELTS_SHARED */
#define STR_EMBED_P(str) (!FL_TEST_RAW((str), STR_NOEMBED))
#define STR_SHARED_P(s)  FL_ALL_RAW((s), STR_NOEMBED|ELTS_SHARED)
#define is_ascii_string(str) (rb_enc_str_coderange(str) == ENC_CODERANGE_7BIT)
#define is_broken_string(str) (rb_enc_str_coderange(str) == ENC_CODERANGE_BROKEN)
size_t rb_str_memsize(VALUE);
VALUE rb_sym_proc_call(VALUE args, VALUE sym, int argc, const VALUE *argv, VALUE passed_proc);
VALUE rb_sym_to_proc(VALUE sym);

/* symbol.c */
#ifdef RUBY_ENCODING_H
VALUE rb_sym_intern(const char *ptr, long len, rb_encoding *enc);
VALUE rb_sym_intern_cstr(const char *ptr, rb_encoding *enc);
#ifdef __GNUC__
#define rb_sym_intern_cstr(ptr, enc) __extension__ ( \
{						\
    (__builtin_constant_p(ptr)) ?		\
	rb_sym_intern((ptr), (long)strlen(ptr), (enc)) : \
	rb_sym_intern_cstr((ptr), (enc)); \
})
#endif
#endif
VALUE rb_sym_intern_ascii(const char *ptr, long len);
VALUE rb_sym_intern_ascii_cstr(const char *ptr);
#ifdef __GNUC__
#define rb_sym_intern_ascii_cstr(ptr) __extension__ ( \
{						\
    (__builtin_constant_p(ptr)) ?		\
	rb_sym_intern_ascii((ptr), (long)strlen(ptr)) : \
	rb_sym_intern_ascii_cstr(ptr); \
})
#endif

/* struct.c */
VALUE rb_struct_init_copy(VALUE copy, VALUE s);
VALUE rb_struct_lookup(VALUE s, VALUE idx);

/* time.c */
struct timeval rb_time_timeval(VALUE);

/* thread.c */
VALUE rb_obj_is_mutex(VALUE obj);
VALUE rb_suppress_tracing(VALUE (*func)(VALUE), VALUE arg);
void rb_thread_execute_interrupts(VALUE th);
void rb_clear_trace_func(void);
VALUE rb_get_coverages(void);
VALUE rb_thread_shield_new(void);
VALUE rb_thread_shield_wait(VALUE self);
VALUE rb_thread_shield_release(VALUE self);
VALUE rb_thread_shield_destroy(VALUE self);
int rb_thread_to_be_killed(VALUE thread);
void rb_mutex_allow_trap(VALUE self, int val);
VALUE rb_uninterruptible(VALUE (*b_proc)(ANYARGS), VALUE data);
VALUE rb_mutex_owned_p(VALUE self);
void ruby_kill(rb_pid_t pid, int sig);

/* thread_pthread.c, thread_win32.c */
void Init_native_thread(void);
int rb_divert_reserved_fd(int fd);

/* transcode.c */
extern VALUE rb_cEncodingConverter;
size_t rb_econv_memsize(rb_econv_t *);

/* us_ascii.c */
extern rb_encoding OnigEncodingUS_ASCII;

/* util.c */
char *ruby_dtoa(double d_, int mode, int ndigits, int *decpt, int *sign, char **rve);
char *ruby_hdtoa(double d, const char *xdigs, int ndigits, int *decpt, int *sign, char **rve);

/* utf_8.c */
extern rb_encoding OnigEncodingUTF_8;

/* variable.c */
void rb_gc_mark_global_tbl(void);
size_t rb_generic_ivar_memsize(VALUE);
VALUE rb_search_class_path(VALUE);
VALUE rb_attr_delete(VALUE, ID);
VALUE rb_ivar_lookup(VALUE obj, ID id, VALUE undef);
void rb_autoload_str(VALUE mod, ID id, VALUE file);

/* version.c */
extern const char ruby_engine[];

/* vm_insnhelper.h */
rb_serial_t rb_next_class_serial(void);

/* vm.c */
VALUE rb_obj_is_thread(VALUE obj);
void rb_vm_mark(void *ptr);
void Init_BareVM(void);
void Init_vm_objects(void);
VALUE rb_vm_top_self(void);
void rb_thread_recycle_stack_release(VALUE *);
void rb_vm_change_state(void);
void rb_vm_inc_const_missing_count(void);
void rb_thread_mark(void *th);
const void **rb_vm_get_insns_address_table(void);
VALUE rb_sourcefilename(void);
VALUE rb_source_location(int *pline);
const char *rb_source_loc(int *pline);
void rb_vm_pop_cfunc_frame(void);
int rb_vm_add_root_module(ID id, VALUE module);
void rb_vm_check_redefinition_by_prepend(VALUE klass);
VALUE rb_yield_refine_block(VALUE refinement, VALUE refinements);
VALUE ruby_vm_sysstack_error_copy(void);

/* vm_dump.c */
void rb_print_backtrace(void);

/* vm_eval.c */
void Init_vm_eval(void);
VALUE rb_current_realfilepath(void);
VALUE rb_check_block_call(VALUE, ID, int, const VALUE *, rb_block_call_func_t, VALUE);
typedef void rb_check_funcall_hook(int, VALUE, ID, int, const VALUE *, VALUE);
VALUE rb_check_funcall_with_hook(VALUE recv, ID mid, int argc, const VALUE *argv,
				 rb_check_funcall_hook *hook, VALUE arg);
VALUE rb_check_funcall_default(VALUE, ID, int, const VALUE *, VALUE);
VALUE rb_catch_protect(VALUE t, rb_block_call_func *func, VALUE data, int *stateptr);
VALUE rb_yield_1(VALUE val);

/* vm_insnhelper.c */
VALUE rb_equal_opt(VALUE obj1, VALUE obj2);

/* vm_method.c */
void Init_eval_method(void);
int rb_method_defined_by(VALUE obj, ID mid, VALUE (*cfunc)(ANYARGS));

/* miniprelude.c, prelude.c */
void Init_prelude(void);

/* vm_backtrace.c */
void Init_vm_backtrace(void);
VALUE rb_vm_thread_backtrace(int argc, const VALUE *argv, VALUE thval);
VALUE rb_vm_thread_backtrace_locations(int argc, const VALUE *argv, VALUE thval);

VALUE rb_make_backtrace(void);
void rb_backtrace_print_as_bugreport(void);
int rb_backtrace_p(VALUE obj);
VALUE rb_backtrace_to_str_ary(VALUE obj);
VALUE rb_backtrace_to_location_ary(VALUE obj);
void rb_backtrace_print_to(VALUE output);
VALUE rb_vm_backtrace_object(void);

RUBY_SYMBOL_EXPORT_BEGIN
const char *rb_objspace_data_type_name(VALUE obj);

/* Temporary.  This API will be removed (renamed). */
VALUE rb_thread_io_blocking_region(rb_blocking_function_t *func, void *data1, int fd);

/* bignum.c (export) */
VALUE rb_big_mul_normal(VALUE x, VALUE y);
VALUE rb_big_mul_balance(VALUE x, VALUE y);
VALUE rb_big_mul_karatsuba(VALUE x, VALUE y);
VALUE rb_big_mul_toom3(VALUE x, VALUE y);
VALUE rb_big_sq_fast(VALUE x);
VALUE rb_big_divrem_normal(VALUE x, VALUE y);
VALUE rb_big2str_poweroftwo(VALUE x, int base);
VALUE rb_big2str_generic(VALUE x, int base);
VALUE rb_str2big_poweroftwo(VALUE arg, int base, int badcheck);
VALUE rb_str2big_normal(VALUE arg, int base, int badcheck);
VALUE rb_str2big_karatsuba(VALUE arg, int base, int badcheck);
#if defined(HAVE_LIBGMP) && defined(HAVE_GMP_H)
VALUE rb_big_mul_gmp(VALUE x, VALUE y);
VALUE rb_big_divrem_gmp(VALUE x, VALUE y);
VALUE rb_big2str_gmp(VALUE x, int base);
VALUE rb_str2big_gmp(VALUE arg, int base, int badcheck);
#endif

/* error.c (export) */
int rb_bug_reporter_add(void (*func)(FILE *, void *), void *data);

/* file.c (export) */
#ifdef HAVE_READLINK
VALUE rb_readlink(VALUE path, rb_encoding *enc);
#endif
#ifdef __APPLE__
VALUE rb_str_normalize_ospath(const char *ptr, long len);
#endif

/* hash.c (export) */
VALUE rb_hash_delete_entry(VALUE hash, VALUE key);
VALUE rb_ident_hash_new(void);

/* io.c (export) */
void rb_maygvl_fd_fix_cloexec(int fd);
int rb_gc_for_fd(int err);
void rb_write_error_str(VALUE mesg);

/* numeric.c (export) */
VALUE rb_int_positive_pow(long x, unsigned long y);

/* process.c (export) */
int rb_exec_async_signal_safe(const struct rb_execarg *e, char *errmsg, size_t errmsg_buflen);
rb_pid_t rb_fork_async_signal_safe(int *status, int (*chfunc)(void*, char *, size_t), void *charg, VALUE fds, char *errmsg, size_t errmsg_buflen);
VALUE rb_execarg_new(int argc, const VALUE *argv, int accept_shell);
struct rb_execarg *rb_execarg_get(VALUE execarg_obj); /* dangerous.  needs GC guard. */
VALUE rb_execarg_init(int argc, const VALUE *argv, int accept_shell, VALUE execarg_obj);
int rb_execarg_addopt(VALUE execarg_obj, VALUE key, VALUE val);
void rb_execarg_parent_start(VALUE execarg_obj);
void rb_execarg_parent_end(VALUE execarg_obj);
int rb_execarg_run_options(const struct rb_execarg *e, struct rb_execarg *s, char* errmsg, size_t errmsg_buflen);
VALUE rb_execarg_extract_options(VALUE execarg_obj, VALUE opthash);
void rb_execarg_setenv(VALUE execarg_obj, VALUE env);

/* rational.c (export) */
VALUE rb_gcd_normal(VALUE self, VALUE other);
#if defined(HAVE_LIBGMP) && defined(HAVE_GMP_H)
VALUE rb_gcd_gmp(VALUE x, VALUE y);
#endif

/* string.c (export) */
#ifdef RUBY_ENCODING_H
/* internal use */
VALUE rb_setup_fake_str(struct RString *fake_str, const char *name, long len, rb_encoding *enc);
#endif

/* thread.c (export) */
int ruby_thread_has_gvl_p(void); /* for ext/fiddle/closure.c */

/* util.c (export) */
extern const signed char ruby_digit36_to_number_table[];
extern const char ruby_hexdigits[];
extern unsigned long ruby_scan_digits(const char *str, ssize_t len, int base, size_t *retlen, int *overflow);

/* variable.c (export) */
void rb_mark_generic_ivar(VALUE);
VALUE rb_const_missing(VALUE klass, VALUE name);
int rb_class_ivar_set(VALUE klass, ID vid, VALUE value);
st_table *rb_st_copy(VALUE obj, struct st_table *orig_tbl);

/* gc.c (export) */
VALUE rb_wb_protected_newobj_of(VALUE, VALUE);
VALUE rb_wb_unprotected_newobj_of(VALUE, VALUE);

size_t rb_obj_memsize_of(VALUE);
void rb_gc_verify_internal_consistency(void);

#define RB_OBJ_GC_FLAGS_MAX 5
size_t rb_obj_gc_flags(VALUE, ID[], size_t);
void rb_gc_mark_values(long n, const VALUE *values);

#if IMEMO_DEBUG
VALUE rb_imemo_new_debug(enum imemo_type type, VALUE v1, VALUE v2, VALUE v3, VALUE v0, const char *file, int line);
#define rb_imemo_new(type, v1, v2, v3, v0) rb_imemo_new_debug(type, v1, v2, v3, v0, __FILE__, __LINE__)
#else
VALUE rb_imemo_new(enum imemo_type type, VALUE v1, VALUE v2, VALUE v3, VALUE v0);
#endif

RUBY_SYMBOL_EXPORT_END

#define RUBY_DTRACE_CREATE_HOOK(name, arg) \
    RUBY_DTRACE_HOOK(name##_CREATE, arg)
#define RUBY_DTRACE_HOOK(name, arg) \
do { \
    if (UNLIKELY(RUBY_DTRACE_##name##_ENABLED())) { \
	int dtrace_line; \
	const char *dtrace_file = rb_source_loc(&dtrace_line); \
	if (!dtrace_file) dtrace_file = ""; \
	RUBY_DTRACE_##name(arg, dtrace_file, dtrace_line); \
    } \
} while (0)

#if defined(__cplusplus)
#if 0
{ /* satisfy cc-mode */
#endif
}  /* extern "C" { */
#endif

#endif /* RUBY_INTERNAL_H */
