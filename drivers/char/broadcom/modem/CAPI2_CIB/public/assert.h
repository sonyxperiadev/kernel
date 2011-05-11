#if     ( defined(_MSC_VER) )&&(( defined(_WIN32) )|| (defined(_MAC)))
/***
*assert.h - define the assert macro
*
*       Copyright (c) 1985-1997, Microsoft Corporation. All rights reserved.
*
*Purpose:
*       Defines the assert(exp) macro.
*       [ANSI/System V]
*
*       [Public]
*
****/

#if     !defined(_WIN32) && !defined(_MAC)
#error ERROR: Only Mac or Win32 targets supported!
#endif




/* Define _CRTIMP */

#ifndef _CRTIMP
#ifdef  _DLL
#define _CRTIMP __declspec(dllimport)
#else   /* ndef _DLL */
#define _CRTIMP
#endif  /* _DLL */
#endif  /* _CRTIMP */


/* Define __cdecl for non-Microsoft compilers */

#if     ( !defined(_MSC_VER) && !defined(__cdecl) )
#define __cdecl
#endif

/* Define _CRTAPI1 (for compatibility with the NT SDK) */

#ifndef _CRTAPI1
#if	_MSC_VER >= 800 && _M_IX86 >= 300
#define _CRTAPI1 __cdecl
#else
#define _CRTAPI1
#endif
#endif

#undef  assert

#ifdef  NDEBUG

#define assert(exp)     ((void)0)

#else

#ifdef  __cplusplus
extern "C" {
#endif

_CRTIMP void __cdecl _assert(void *, void *, unsigned);

#ifdef  __cplusplus
}
#endif

#ifdef MTI_DECODE
#define __LINENUM__ __LINE__
#endif //MTI_DECODE

#define assert(exp) (void)( (exp) || (_assert(#exp, __FILE__, __LINENUM__), 0) )

#endif  /* NDEBUG */
#else //  ( defined(_MSC_VER) )
/* assert.h: ANSI 'C' (X3J11 Oct 88) library header section 4.2 */
/* Copyright (C) Codemist Ltd., 1988-1993                       */
/* Copyright 1991-1993 ARM Limited. All rights reserved.        */
/* version 0.04 */

/*
 * RCS $Revision: 1.11 $
 * Checkin $Date: 2004/03/15 13:49:00 $
 * Revising $Author: agrant $
 */

/*
 * The assert macro puts diagnostics into programs. When it is executed,
 * if its argument expression is false, it writes information about the
 * call that failed (including the text of the argument, the name of the
 * source file, and the source line number - the latter are respectively
 * the values of the preprocessing macros __FILE__ and __LINENUM__) on the
 * standard error stream. It then calls the abort function.
 * If its argument expression is true, the assert macro returns no value.
 */

/*
 * Note that <assert.h> may be included more that once in a program with
 * different setting of NDEBUG. Hence the slightly unusual first-time
 * only flag.
 */

#ifndef __assert_h
#   define __assert_h
#define _ARMABI_NORETURN __declspec(__nothrow) __declspec(__noreturn)
#   undef __CLIBNS
#   ifdef __cplusplus
        namespace std {
#           define __CLIBNS ::std::
            extern "C" {
#   else
#       define __CLIBNS
#   endif  /* __cplusplus */
    extern void abort(void);
#if __ARMCC_VERSION >= 310000
    extern void __aeabi_assert(const char *, const char *, int);	//BRCM, Let assert return
#else
    extern void __assert(const char *, const char *, int);	//BRCM, Let assert return
#endif
#   ifdef __cplusplus
            }  /* extern "C" */
        }  /* namespace std */
#   endif
#else
#   undef assert
#endif

#ifdef NDEBUG
#   define assert(ignore) ((void)0)
#else
#   if defined __OPT_SMALL_ASSERT && !defined __ASSERT_MSG && !defined __STRICT_ANSI__
#       define assert(e) ((e) ? (void)0 : __CLIBNS abort())
#   elif defined __STDC__
#     if __ARMCC_VERSION >= 310000
#       define assert(e) ((e) ? (void)0 : __CLIBNS __aeabi_assert(#e, __FILE__, __LINENUM__))
#     else
#       define assert(e) ((e) ? (void)0 : __CLIBNS __assert(#e, __FILE__, __LINENUM__))
#     endif
#   else
#     if __ARMCC_VERSION >= 310000
#       define assert(e) ((e) ? (void)0 : __CLIBNS __aeabi_assert("e", __FILE__, __LINENUM__))
#     else
#       define assert(e) ((e) ? (void)0 : __CLIBNS __assert("e", __FILE__, __LINENUM__))
#     endif
#   endif
#endif
#endif // ( defined(_MSC_VER) )
/* end of assert.h */
