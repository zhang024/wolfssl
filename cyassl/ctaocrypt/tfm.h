/* tfm.h
 *
 * Copyright (C) 2006-2013 wolfSSL Inc.
 *
 * This file is part of CyaSSL.
 *
 * CyaSSL is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * CyaSSL is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */


/*
 * Based on public domain TomsFastMath 0.10 by Tom St Denis, tomstdenis@iahu.ca,
 * http://math.libtomcrypt.com
 */


/**
 *  Edited by Mois�s Guimar�es (moises.guimaraes@phoebus.com.br)
 *  to fit CyaSSL's needs.
 */


#ifndef CTAO_CRYPT_TFM_H
#define CTAO_CRYPT_TFM_H

#include <cyassl/ctaocrypt/types.h>
#ifndef CHAR_BIT
    #include <limits.h>
#endif


#ifdef __cplusplus
    extern "C" {
#endif

#ifndef MIN
   #define MIN(x,y) ((x)<(y)?(x):(y))
#endif

#ifndef MAX
   #define MAX(x,y) ((x)>(y)?(x):(y))
#endif


#ifndef NO_64BIT
/* autodetect x86-64 and make sure we are using 64-bit digits with x86-64 asm */
#if defined(__x86_64__)
   #if defined(TFM_X86) || defined(TFM_SSE2) || defined(TFM_ARM) 
       #error x86-64 detected, x86-32/SSE2/ARM optimizations are not valid!
   #endif
   #if !defined(TFM_X86_64) && !defined(TFM_NO_ASM)
      #define TFM_X86_64
   #endif
#endif
#if defined(TFM_X86_64)
    #if !defined(FP_64BIT)
       #define FP_64BIT
    #endif
#endif
/* use 64-bit digit even if not using asm on x86_64 */
#if defined(__x86_64__) && !defined(FP_64BIT)
    #define FP_64BIT
#endif
#endif /* NO_64BIT */

/* try to detect x86-32 */
#if defined(__i386__) && !defined(TFM_SSE2)
   #if defined(TFM_X86_64) || defined(TFM_ARM) 
       #error x86-32 detected, x86-64/ARM optimizations are not valid!
   #endif
   #if !defined(TFM_X86) && !defined(TFM_NO_ASM)
      #define TFM_X86
   #endif
#endif

/* make sure we're 32-bit for x86-32/sse/arm/ppc32 */
#if (defined(TFM_X86) || defined(TFM_SSE2) || defined(TFM_ARM) || defined(TFM_PPC32)) && defined(FP_64BIT)
   #warning x86-32, SSE2 and ARM, PPC32 optimizations require 32-bit digits (undefining)
   #undef FP_64BIT
#endif

/* multi asms? */
#ifdef TFM_X86
   #define TFM_ASM
#endif
#ifdef TFM_X86_64
   #ifdef TFM_ASM
      #error TFM_ASM already defined!
   #endif
   #define TFM_ASM
#endif
#ifdef TFM_SSE2
   #ifdef TFM_ASM
      #error TFM_ASM already defined!
   #endif
   #define TFM_ASM
#endif
#ifdef TFM_ARM
   #ifdef TFM_ASM
      #error TFM_ASM already defined!
   #endif
   #define TFM_ASM
#endif
#ifdef TFM_PPC32
   #ifdef TFM_ASM
      #error TFM_ASM already defined!
   #endif
   #define TFM_ASM
#endif
#ifdef TFM_PPC64
   #ifdef TFM_ASM
      #error TFM_ASM already defined!
   #endif
   #define TFM_ASM
#endif
#ifdef TFM_AVR32
   #ifdef TFM_ASM
      #error TFM_ASM already defined!
   #endif
   #define TFM_ASM
#endif

/* we want no asm? */
#ifdef TFM_NO_ASM
   #undef TFM_X86
   #undef TFM_X86_64
   #undef TFM_SSE2
   #undef TFM_ARM
   #undef TFM_PPC32
   #undef TFM_PPC64
   #undef TFM_AVR32
   #undef TFM_ASM   
#endif

/* ECC helpers */
#ifdef TFM_ECC192
   #ifdef FP_64BIT
       #define TFM_MUL3
       #define TFM_SQR3
   #else
       #define TFM_MUL6
       #define TFM_SQR6
   #endif
#endif

#ifdef TFM_ECC224
   #ifdef FP_64BIT
       #define TFM_MUL4
       #define TFM_SQR4
   #else
       #define TFM_MUL7
       #define TFM_SQR7
   #endif
#endif

#ifdef TFM_ECC256
   #ifdef FP_64BIT
       #define TFM_MUL4
       #define TFM_SQR4
   #else
       #define TFM_MUL8
       #define TFM_SQR8
   #endif
#endif

#ifdef TFM_ECC384
   #ifdef FP_64BIT
       #define TFM_MUL6
       #define TFM_SQR6
   #else
       #define TFM_MUL12
       #define TFM_SQR12
   #endif
#endif

#ifdef TFM_ECC521
   #ifdef FP_64BIT
       #define TFM_MUL9
       #define TFM_SQR9
   #else
       #define TFM_MUL17
       #define TFM_SQR17
   #endif
#endif


/* some default configurations.
 */
#if defined(FP_64BIT)
   /* for GCC only on supported platforms */
   typedef unsigned long long fp_digit;   /* 64bit, 128 uses mode(TI) below */
   typedef unsigned long      fp_word __attribute__ ((mode(TI)));
#else
   #if defined(_MSC_VER) || defined(__BORLANDC__) 
      typedef unsigned __int64   ulong64;
   #else
      typedef unsigned long long ulong64;
   #endif

   #ifndef NO_64BIT
      typedef unsigned int       fp_digit;
      typedef ulong64            fp_word;
   #else
      /* some procs like coldfire prefer not to place multiply into 64bit type
         even though it exists */
      typedef unsigned short     fp_digit;
      typedef unsigned int       fp_word;
   #endif
#endif

/* # of digits this is */
#define DIGIT_BIT  (int)((CHAR_BIT) * sizeof(fp_digit))

/* Max size of any number in bits.  Basically the largest size you will be
 * multiplying should be half [or smaller] of FP_MAX_SIZE-four_digit
 *
 * It defaults to 4096-bits [allowing multiplications upto 2048x2048 bits ]
 */
#ifndef FP_MAX_BITS
    #define FP_MAX_BITS           4096
#endif
#define FP_MAX_SIZE           (FP_MAX_BITS+(8*DIGIT_BIT))

/* will this lib work? */
#if (CHAR_BIT & 7)
   #error CHAR_BIT must be a multiple of eight.
#endif
#if FP_MAX_BITS % CHAR_BIT
   #error FP_MAX_BITS must be a multiple of CHAR_BIT
#endif

#define FP_MASK    (fp_digit)(-1)
#define FP_SIZE    (FP_MAX_SIZE/DIGIT_BIT)

/* signs */
#define FP_ZPOS     0
#define FP_NEG      1

/* return codes */
#define FP_OKAY     0
#define FP_VAL      1
#define FP_MEM      2

/* equalities */
#define FP_LT        -1   /* less than */
#define FP_EQ         0   /* equal to */
#define FP_GT         1   /* greater than */

/* replies */
#define FP_YES        1   /* yes response */
#define FP_NO         0   /* no response */

/* a FP type */
typedef struct {
    fp_digit dp[FP_SIZE];
    int      used, 
             sign;
} fp_int;

/* externally define this symbol to ignore the default settings, useful for changing the build from the make process */
#ifndef TFM_ALREADY_SET

/* do we want the large set of small multiplications ? 
   Enable these if you are going to be doing a lot of small (<= 16 digit) multiplications say in ECC
   Or if you're on a 64-bit machine doing RSA as a 1024-bit integer == 16 digits ;-)
 */
/* need to refactor the function */
/*#define TFM_SMALL_SET */

/* do we want huge code 
   Enable these if you are doing 20, 24, 28, 32, 48, 64 digit multiplications (useful for RSA)
   Less important on 64-bit machines as 32 digits == 2048 bits
 */
#if 0
#define TFM_MUL3
#define TFM_MUL4
#define TFM_MUL6
#define TFM_MUL7
#define TFM_MUL8
#define TFM_MUL9
#define TFM_MUL12
#define TFM_MUL17
#endif
#ifdef TFM_HUGE_SET
#define TFM_MUL20
#define TFM_MUL24
#define TFM_MUL28
#define TFM_MUL32
#if (FP_MAX_BITS >= 6144) && defined(FP_64BIT)
    #define TFM_MUL48
#endif
#if (FP_MAX_BITS >= 8192) && defined(FP_64BIT)
    #define TFM_MUL64
#endif
#endif

#if 0
#define TFM_SQR3
#define TFM_SQR4
#define TFM_SQR6
#define TFM_SQR7
#define TFM_SQR8
#define TFM_SQR9
#define TFM_SQR12
#define TFM_SQR17
#endif
#ifdef TFM_HUGE_SET
#define TFM_SQR20
#define TFM_SQR24
#define TFM_SQR28
#define TFM_SQR32
#define TFM_SQR48
#define TFM_SQR64
#endif

/* do we want some overflow checks
   Not required if you make sure your numbers are within range (e.g. by default a modulus for fp_exptmod() can only be upto 2048 bits long)
 */
/* #define TFM_CHECK */

/* Is the target a P4 Prescott
 */
/* #define TFM_PRESCOTT */

/* Do we want timing resistant fp_exptmod() ?
 * This makes it slower but also timing invariant with respect to the exponent 
 */
/* #define TFM_TIMING_RESISTANT */

#endif /* TFM_ALREADY_SET */

/* functions */

/* returns a TFM ident string useful for debugging... */
/*const char *fp_ident(void);*/

/* initialize [or zero] an fp int */
#define fp_init(a)  (void)XMEMSET((a), 0, sizeof(fp_int))
#define fp_zero(a)  fp_init(a)

/* zero/even/odd ? */
#define fp_iszero(a) (((a)->used == 0) ? FP_YES : FP_NO)
#define fp_iseven(a) (((a)->used >= 0 && (((a)->dp[0] & 1) == 0)) ? FP_YES : FP_NO)
#define fp_isodd(a)  (((a)->used > 0  && (((a)->dp[0] & 1) == 1)) ? FP_YES : FP_NO)

/* set to a small digit */
void fp_set(fp_int *a, fp_digit b);

/* copy from a to b */
#define fp_copy(a, b)  (void)(((a) != (b)) ? ((void)XMEMCPY((b), (a), sizeof(fp_int))) : (void)0)
#define fp_init_copy(a, b) fp_copy(b, a)

/* clamp digits */
#define fp_clamp(a)   { while ((a)->used && (a)->dp[(a)->used-1] == 0) --((a)->used); (a)->sign = (a)->used ? (a)->sign : FP_ZPOS; }

/* negate and absolute */
#define fp_neg(a, b)  { fp_copy(a, b); (b)->sign ^= 1; fp_clamp(b); }
#define fp_abs(a, b)  { fp_copy(a, b); (b)->sign  = 0; }

/* right shift x digits */
void fp_rshd(fp_int *a, int x);

/* right shift x bits */
void fp_rshb(fp_int *a, int x);

/* left shift x digits */
void fp_lshd(fp_int *a, int x);

/* signed comparison */
int fp_cmp(fp_int *a, fp_int *b);

/* unsigned comparison */
int fp_cmp_mag(fp_int *a, fp_int *b);

/* power of 2 operations */
void fp_div_2d(fp_int *a, int b, fp_int *c, fp_int *d);
void fp_mod_2d(fp_int *a, int b, fp_int *c);
void fp_mul_2d(fp_int *a, int b, fp_int *c);
void fp_2expt (fp_int *a, int b);
void fp_mul_2(fp_int *a, fp_int *c);
void fp_div_2(fp_int *a, fp_int *c);

/* Counts the number of lsbs which are zero before the first zero bit */
/*int fp_cnt_lsb(fp_int *a);*/

/* c = a + b */
void fp_add(fp_int *a, fp_int *b, fp_int *c);

/* c = a - b */
void fp_sub(fp_int *a, fp_int *b, fp_int *c);

/* c = a * b */
void fp_mul(fp_int *a, fp_int *b, fp_int *c);

/* b = a*a  */
void fp_sqr(fp_int *a, fp_int *b);

/* a/b => cb + d == a */
int fp_div(fp_int *a, fp_int *b, fp_int *c, fp_int *d);

/* c = a mod b, 0 <= c < b  */
int fp_mod(fp_int *a, fp_int *b, fp_int *c);

/* compare against a single digit */
int fp_cmp_d(fp_int *a, fp_digit b);

/* c = a + b */
void fp_add_d(fp_int *a, fp_digit b, fp_int *c);

/* c = a - b */
void fp_sub_d(fp_int *a, fp_digit b, fp_int *c);

/* c = a * b */
void fp_mul_d(fp_int *a, fp_digit b, fp_int *c);

/* a/b => cb + d == a */
/*int fp_div_d(fp_int *a, fp_digit b, fp_int *c, fp_digit *d);*/

/* c = a mod b, 0 <= c < b  */
/*int fp_mod_d(fp_int *a, fp_digit b, fp_digit *c);*/

/* ---> number theory <--- */
/* d = a + b (mod c) */
/*int fp_addmod(fp_int *a, fp_int *b, fp_int *c, fp_int *d);*/

/* d = a - b (mod c) */
/*int fp_submod(fp_int *a, fp_int *b, fp_int *c, fp_int *d);*/

/* d = a * b (mod c) */
int fp_mulmod(fp_int *a, fp_int *b, fp_int *c, fp_int *d);

/* c = a * a (mod b) */
int fp_sqrmod(fp_int *a, fp_int *b, fp_int *c);

/* c = 1/a (mod b) */
int fp_invmod(fp_int *a, fp_int *b, fp_int *c);

/* c = (a, b) */
/*void fp_gcd(fp_int *a, fp_int *b, fp_int *c);*/

/* c = [a, b] */
/*void fp_lcm(fp_int *a, fp_int *b, fp_int *c);*/

/* setups the montgomery reduction */
int fp_montgomery_setup(fp_int *a, fp_digit *mp);

/* computes a = B**n mod b without division or multiplication useful for
 * normalizing numbers in a Montgomery system.
 */
void fp_montgomery_calc_normalization(fp_int *a, fp_int *b);

/* computes x/R == x (mod N) via Montgomery Reduction */
void fp_montgomery_reduce(fp_int *a, fp_int *m, fp_digit mp);

/* d = a**b (mod c) */
int fp_exptmod(fp_int *a, fp_int *b, fp_int *c, fp_int *d);

/* primality stuff */

/* perform a Miller-Rabin test of a to the base b and store result in "result" */
/*void fp_prime_miller_rabin (fp_int * a, fp_int * b, int *result);*/

/* 256 trial divisions + 8 Miller-Rabins, returns FP_YES if probable prime  */
/*int fp_isprime(fp_int *a);*/

/* Primality generation flags */
/*#define TFM_PRIME_BBS      0x0001 */ /* BBS style prime */
/*#define TFM_PRIME_SAFE     0x0002 */ /* Safe prime (p-1)/2 == prime */
/*#define TFM_PRIME_2MSB_OFF 0x0004 */ /* force 2nd MSB to 0 */
/*#define TFM_PRIME_2MSB_ON  0x0008 */ /* force 2nd MSB to 1 */

/* callback for fp_prime_random, should fill dst with random bytes and return how many read [upto len] */
/*typedef int tfm_prime_callback(unsigned char *dst, int len, void *dat);*/

/*#define fp_prime_random(a, t, size, bbs, cb, dat) fp_prime_random_ex(a, t, ((size) * 8) + 1, (bbs==1)?TFM_PRIME_BBS:0, cb, dat)*/

/*int fp_prime_random_ex(fp_int *a, int t, int size, int flags, tfm_prime_callback cb, void *dat);*/

/* radix conersions */
int fp_count_bits(fp_int *a);
int fp_leading_bit(fp_int *a);

int fp_unsigned_bin_size(fp_int *a);
void fp_read_unsigned_bin(fp_int *a, unsigned char *b, int c);
void fp_to_unsigned_bin(fp_int *a, unsigned char *b);

/*int fp_signed_bin_size(fp_int *a);*/
/*void fp_read_signed_bin(fp_int *a, unsigned char *b, int c);*/
/*void fp_to_signed_bin(fp_int *a, unsigned char *b);*/

/*int fp_read_radix(fp_int *a, char *str, int radix);*/
/*int fp_toradix(fp_int *a, char *str, int radix);*/
/*int fp_toradix_n(fp_int * a, char *str, int radix, int maxlen);*/


/* VARIOUS LOW LEVEL STUFFS */
void s_fp_add(fp_int *a, fp_int *b, fp_int *c);
void s_fp_sub(fp_int *a, fp_int *b, fp_int *c);
void fp_reverse(unsigned char *s, int len);

void fp_mul_comba(fp_int *A, fp_int *B, fp_int *C);

#ifdef TFM_SMALL_SET
void fp_mul_comba_small(fp_int *A, fp_int *B, fp_int *C);
#endif

#ifdef TFM_MUL3
void fp_mul_comba3(fp_int *A, fp_int *B, fp_int *C);
#endif
#ifdef TFM_MUL4
void fp_mul_comba4(fp_int *A, fp_int *B, fp_int *C);
#endif
#ifdef TFM_MUL6
void fp_mul_comba6(fp_int *A, fp_int *B, fp_int *C);
#endif
#ifdef TFM_MUL7
void fp_mul_comba7(fp_int *A, fp_int *B, fp_int *C);
#endif
#ifdef TFM_MUL8
void fp_mul_comba8(fp_int *A, fp_int *B, fp_int *C);
#endif
#ifdef TFM_MUL9
void fp_mul_comba9(fp_int *A, fp_int *B, fp_int *C);
#endif
#ifdef TFM_MUL12
void fp_mul_comba12(fp_int *A, fp_int *B, fp_int *C);
#endif
#ifdef TFM_MUL17
void fp_mul_comba17(fp_int *A, fp_int *B, fp_int *C);
#endif

#ifdef TFM_MUL20
void fp_mul_comba20(fp_int *A, fp_int *B, fp_int *C);
#endif
#ifdef TFM_MUL24
void fp_mul_comba24(fp_int *A, fp_int *B, fp_int *C);
#endif
#ifdef TFM_MUL28
void fp_mul_comba28(fp_int *A, fp_int *B, fp_int *C);
#endif
#ifdef TFM_MUL32
void fp_mul_comba32(fp_int *A, fp_int *B, fp_int *C);
#endif
#ifdef TFM_MUL48
void fp_mul_comba48(fp_int *A, fp_int *B, fp_int *C);
#endif
#ifdef TFM_MUL64
void fp_mul_comba64(fp_int *A, fp_int *B, fp_int *C);
#endif

void fp_sqr_comba(fp_int *A, fp_int *B);

#ifdef TFM_SMALL_SET
void fp_sqr_comba_small(fp_int *A, fp_int *B);
#endif

#ifdef TFM_SQR3
void fp_sqr_comba3(fp_int *A, fp_int *B);
#endif
#ifdef TFM_SQR4
void fp_sqr_comba4(fp_int *A, fp_int *B);
#endif
#ifdef TFM_SQR6
void fp_sqr_comba6(fp_int *A, fp_int *B);
#endif
#ifdef TFM_SQR7
void fp_sqr_comba7(fp_int *A, fp_int *B);
#endif
#ifdef TFM_SQR8
void fp_sqr_comba8(fp_int *A, fp_int *B);
#endif
#ifdef TFM_SQR9
void fp_sqr_comba9(fp_int *A, fp_int *B);
#endif
#ifdef TFM_SQR12
void fp_sqr_comba12(fp_int *A, fp_int *B);
#endif
#ifdef TFM_SQR17
void fp_sqr_comba17(fp_int *A, fp_int *B);
#endif

#ifdef TFM_SQR20
void fp_sqr_comba20(fp_int *A, fp_int *B);
#endif
#ifdef TFM_SQR24
void fp_sqr_comba24(fp_int *A, fp_int *B);
#endif
#ifdef TFM_SQR28
void fp_sqr_comba28(fp_int *A, fp_int *B);
#endif
#ifdef TFM_SQR32
void fp_sqr_comba32(fp_int *A, fp_int *B);
#endif
#ifdef TFM_SQR48
void fp_sqr_comba48(fp_int *A, fp_int *B);
#endif
#ifdef TFM_SQR64
void fp_sqr_comba64(fp_int *A, fp_int *B);
#endif
/*extern const char *fp_s_rmap;*/


/**
 * Used by CyaSSL 
 */

/* Types */
    typedef fp_digit mp_digit;
    typedef fp_word  mp_word;
    typedef fp_int mp_int;

/* Constants */
    #define MP_LT   FP_LT   /* less than    */
    #define MP_EQ   FP_EQ   /* equal to     */
    #define MP_GT   FP_GT   /* greater than */
    #define MP_OKAY FP_OKAY /* ok result    */
    #define MP_NO   FP_NO   /* yes/no result */
    #define MP_YES  FP_YES  /* yes/no result */

/* Prototypes */
int  mp_init (mp_int * a);
void mp_clear (mp_int * a);
int mp_init_multi(mp_int* a, mp_int* b, mp_int* c, mp_int* d, mp_int* e, mp_int* f);

int  mp_add (mp_int * a, mp_int * b, mp_int * c);
int  mp_sub (mp_int * a, mp_int * b, mp_int * c);
int  mp_add_d (mp_int * a, mp_digit b, mp_int * c);

int  mp_mul (mp_int * a, mp_int * b, mp_int * c);
int  mp_mulmod (mp_int * a, mp_int * b, mp_int * c, mp_int * d);
int  mp_mod(mp_int *a, mp_int *b, mp_int *c);
int  mp_invmod(mp_int *a, mp_int *b, mp_int *c);
int  mp_exptmod (mp_int * G, mp_int * X, mp_int * P, mp_int * Y);

int  mp_cmp(mp_int *a, mp_int *b);
int  mp_cmp_d(mp_int *a, mp_digit b);

int  mp_unsigned_bin_size(mp_int * a);
int  mp_read_unsigned_bin (mp_int * a, const unsigned char *b, int c);
int  mp_to_unsigned_bin (mp_int * a, unsigned char *b);

int  mp_sub_d(fp_int *a, fp_digit b, fp_int *c);
int  mp_copy(fp_int* a, fp_int* b);
int  mp_isodd(mp_int* a);
int  mp_iszero(mp_int* a);
int  mp_count_bits(mp_int *a);
int  mp_leading_bit(mp_int *a);
int  mp_set_int(fp_int *a, fp_digit b);
void mp_rshb(mp_int *a, int x);

#ifdef HAVE_ECC
    int mp_read_radix(mp_int* a, const char* str, int radix);
    int mp_set(fp_int *a, fp_digit b);
    int mp_sqr(fp_int *A, fp_int *B);
    int mp_montgomery_reduce(fp_int *a, fp_int *m, fp_digit mp);
    int mp_montgomery_setup(fp_int *a, fp_digit *rho);
    int mp_div_2(fp_int * a, fp_int * b);
    int mp_init_copy(fp_int * a, fp_int * b); 
#endif

#if defined(HAVE_ECC) || defined(CYASSL_KEY_GEN)
    int mp_sqrmod(mp_int* a, mp_int* b, mp_int* c);
    int mp_montgomery_calc_normalization(mp_int *a, mp_int *b);
#endif

#ifdef CYASSL_KEY_GEN
int  mp_gcd(fp_int *a, fp_int *b, fp_int *c);
int  mp_lcm(fp_int *a, fp_int *b, fp_int *c);
int  mp_prime_is_prime(mp_int* a, int t, int* result);
#endif /* CYASSL_KEY_GEN */

CYASSL_API word32 CheckRunTimeFastMath(void);

/* If user uses RSA, DH, DSA, or ECC math lib directly then fast math FP_SIZE
   must match, return 1 if a match otherwise 0 */
#define CheckFastMathSettings() (FP_SIZE == CheckRunTimeFastMath())
#ifdef __cplusplus
   }
#endif


#endif  /* CTAO_CRYPT_TFM_H */
