/*                                   HTParse:  URL parsing in the WWW Library
   **                           HTPARSE
   **
   **  This module of the WWW library contains code to parse URLs and various
   **  related things.
   **  Implemented by HTParse.c .
 */

#include "echoping.h" 
#include <ctype.h>

#ifndef HTPARSE_H
#define HTPARSE_H

/*
   **  The following are flag bits which may be ORed together to form
   **  a number to give the 'wanted' argument to HTParse.
 */
#define PARSE_ACCESS            16
#define PARSE_HOST               8
#define PARSE_PATH               4
#define PARSE_ANCHOR             2
#define PARSE_PUNCTUATION        1
#define PARSE_ALL               31

/*
   **  The following are valid mask values. The terms are the BNF names
   **  in the URL document.
 */
#define URL_XALPHAS     (unsigned char) 1
#define URL_XPALPHAS    (unsigned char) 2
#define URL_PATH        (unsigned char) 4

/*

   Macros for declarations

 */
#define PUBLIC			/* Accessible outside this module     */
#define PRIVATE static		/* Accessible only within this module */

#ifdef __STDC__
#define CONST const		/* "const" only exists in STDC */
#else
#define CONST
#endif
#define NOPARAMS (void)
#define PARAMS(parameter_list) parameter_list
#define NOARGS (void)
#define ARGS1(t,a) \
                (t a)
#define ARGS2(t,a,u,b) \
                (t a, u b)
#define ARGS3(t,a,u,b,v,c) \
                (t a, u b, v c)


/*      Parse a Name relative to another name.                  HTParse()
   **   --------------------------------------
   **
   **   This returns those parts of a name which are given (and requested)
   **   substituting bits from the related name where necessary.
   **
   ** On entry,
   **   aName           A filename given
   **      relatedName     A name relative to which aName is to be parsed
   **      wanted          A mask for the bits which are wanted.
   **
   ** On exit,
   **   returns         A pointer to a malloc'd string which MUST BE FREED
 */
extern char *HTParse PARAMS((
			       CONST char *aName,
			       CONST char *relatedName,
			       int wanted));

#ifndef TOLOWER
  /* Pyramid and Mips can't uppercase non-alpha */
#define TOLOWER(c) (isupper((unsigned char)c) ? tolower((unsigned char)c) : (c))
#define TOUPPER(c) (islower((unsigned char)c) ? toupper((unsigned char)c) : (c))
#endif /* ndef TOLOWER */

#define outofmem(file, func)\
 { fprintf(stderr,\
  "\r\n\r\n\r\n%s %s: out of memory.  Aborting...\r\n", file, func);\
   exit(-1);}

#define StrAllocCopy(dest, src) HTSACopy (&(dest), src)
#define StrAllocCat(dest, src)  HTSACat  (&(dest), src)

PUBLIC void HTSimplify ARGS1 (
			       char *, filename);

#endif /* HTPARSE_H */

/*
   end of HTParse
 */
