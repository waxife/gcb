/**
 *  @file   doprnt.c
 *  @brief  
 *  $Id: doprnt.c,v 1.3 2013/01/28 11:19:42 ken Exp $
 *  $Author: ken $
 *  $Revision: 1.3 $
 *
 *  Copyright (c) 2011 Terawins Inc. All rights reserved.
 * 
 *  @date   2012/02/29  ken 	New file.
 *
 */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include "config.h"
#include "serial.h"
#include "doprnt.h"

/*
 * Cx51 Data Types
 *
 *        Type          Byte             Value Range
 * -------------------  ----    -----------------------------
 *   signed char           1             -128 ~          +127
 * unsigned char           1                0 ~           255
 *   signed short (int)    2           -32768 ~        +32767
 * unsigned short (int)    2                0 ~         65535
 *   signed int            2           -32768 ~        +32767
 * unsigned int            2                0 ~         65535
 *   signed long (int)     4      -2147483648 ~   +2147483647
 * unsigned long (int)     4                0 ~    4294967295
 *    float/double         4    -3.402823E+38 ~ +3.402823E+38 (max/min)
 *                              -1.175494E-38 ~ +1.175494E-38 (precision)
 */

/*
 * Because Keil C doesn't apply ANSI default argument promotions with
 * functions, which is called with a varying number of arguments of
 * varying types, the function arguments should be casted manually.
 * Accroding to the K&R C standard, default argument promotion rule
 * is that all char(s) and short(s) are widened to int(s), and all
 * float(s) are widened to double(s).
 *
 * To display value of 'char':
 *           -128 as   signed char  >> printf ("%bd\n", (char)-128);
 *            127 as   signed char  >> printf ("%bd\n", (char)127);
 *              0 as unsigned char  >> printf ("%bu\n", (char)0);
 *            255 as unsigned char  >> printf ("%bu\n", (char)255);
 *
 * To display value of 'short':
 *         -32768 as   signed short >> printf ("%hd\n", (short)-32768);
 *          32767 as   signed short >> printf ("%hd\n", (short)32767);
 *              0 as unsigned short >> printf ("%hu\n", (short)0);
 *          65535 as unsigned short >> printf ("%hu\n", (short)65535);
 *
 * To display value of 'int':
 *         -32768 as   signed int   >> printf ("%d\n",  (int)-32768);
 *          32767 as   signed int   >> printf ("%d\n",  (int)32767);
 *              0 as unsigned int   >> printf ("%u\n",  (int)0);
 *          65535 as unsigned int   >> printf ("%u\n",  (int)65535);
 *
 * To display value of 'long':
 *    -2147483648 as   signed long  >> printf ("%ld\n", (long)-2147483648);
 *     2147483647 as   signed long  >> printf ("%ld\n", (long)2147483647);
 *              0 as unsigned long  >> printf ("%lu\n", (long)0);
 *     4294967295 as unsigned long  >> printf ("%lu\n", (long)4294967295);
 */

/*
 * FIXME printf_tw() don't support float/double.
 */

typedef unsigned char uchar;
typedef unsigned long ulong;

#define LONGINT     0x01    /* long integer */
#define LONGDBL     0x02    /* long double; unimplemented */
#define SHORTINT    0x04    /* short integer */
#define ALT         0x08    /* alternate form */
#define LADJUST     0x10    /* left adjustment */
#define ZEROPAD     0x20    /* zero (as opposed to blank) pad */
#define HEXPREFIX   0x40    /* add 0x or 0X prefix */
#define BYTEINT     0x80    /* short integer */

#define ARG(signedtype) _ulong = ((flags & LONGINT) ? (signedtype long) va_arg (argp, long) : ((flags & SHORTINT) ? (signedtype short) va_arg (argp, short) : ((flags & BYTEINT)  ? (signedtype char) va_arg (argp, char) : (signedtype int) va_arg (argp, int))))

#define todigit(c)	((c) - '0')
#define tochar(c)	((c) + '0')
#define isascii(c)	((c) < 128)

#define BUFSIZE     16


#define FPREC   2
#define MOD     1000

#ifdef DEBUG_MODE
int
printf_tw (const char *fmt, ...)
{
    const uchar *ptr;   /* format string */
    char *t;            /* buffer pointer */
    double _double;     /* double precision arguments %[eEfgG] */
    ulong _ulong;       /* integer arguments %[diouxX] */
    char cnt;           /* return value accumulator */
    char n;             /* random handy integer */
    char base;          /* base for [diouxX] conversion */
    char dprec;         /* decimal precision in [diouxX] */
    char fieldsz;       /* field size expanded by sign, etc */
    char flags;         /* flags as above */
    char prec;          /* precision from format (%.3d), or -1 */
    char realsz;        /* field size expanded by decimal precision */
    char size;          /* size of converted field or string */
    char width;         /* width from format (%8d), or 0 */
    char sign;          /* sign prefix (' ', '+', '-', or \0) */
    const char *digs;   /* digits for [diouxX] conversion */
    char buf[BUFSIZE];  /* space for %c, %[diouxX], %[eEfgG] */

    va_list argp;
    va_start(argp, fmt);
#ifdef I2C_DEBUG_MODE
	i2c_count=0x00;
#endif
    cnt = 0;
    for (ptr = (const uchar *)fmt; *ptr != '\0'; ptr++) {
        if (*ptr != '%') {
            putb_t (*ptr);
            cnt++;
            continue;
        }

        n = 0;
        flags = 0;
        dprec = 0;
        /* fpprec = 0; */
        width = 0;
        prec = -1;
        sign = '\0';

        digs = "0123456789abcdef";
rflag:
        switch (*++ptr) {
            case ' ':
                /*
                 * ``If the space and + flags both appear, the space
                 * flag will be ignored.''
                 *  -- ANSI X3J11
                 */
                if (!sign) {
                    sign = ' ';
                }
                goto rflag;

            case '#':
                flags |= ALT;
                goto rflag;
            case '*':
                /*
                 * ``A negative field width argument is taken as a
                 * - flag followed by a  positive field width.''
                 *  -- ANSI X3J11
                 * They don't exclude field widths read from args.
                 */
                if ((width = va_arg (argp, int)) >= 0) {
                    goto rflag;
                }
                width = -width;
                /* FALLTHROUGH */
            case '-':
                flags |= LADJUST;
                goto rflag;
            case '+':
                sign = '+';
                goto rflag;
            case '.':
                if (*++ptr == '*') {
                    n = va_arg (argp, int);
                } else {
                    n = 0;
                    while (isascii (*ptr) && isdigit (*ptr)) {
                        n = 10 * n + todigit (*ptr++);
                    }
                    --ptr;
                }
                prec = n < 0 ? -1 : n;
                goto rflag;
            case '0':
                /*
                 * ``Note that 0 is taken as a flag, not as the
                 * beginning of a field width.''
                 *  -- ANSI X3J11
                 */
                flags |= ZEROPAD;
                goto rflag;
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                n = 0;
                while (isascii (*ptr) && isdigit (*ptr)) {
                    n = 10 * n + todigit (*ptr++);
                }
                --ptr;
                width = n;
                goto rflag;
#if 0
            case 'L':
                flags |= LONGDBL;
                goto rflag;
#endif
            case 'h':
                flags |= SHORTINT;
                goto rflag;
            case 'l':
                flags |= LONGINT;
                goto rflag;
            case 'b':
                flags |= BYTEINT;
                goto rflag;
            case 'c':
                *(t = buf) = va_arg (argp, int);
                // egg responsc: when he printed '\0', the string was breaked
                // while other systems don't.
                if ((*t) == '\0')
                    *t = ' ';
                size = 1;
                sign = '\0';
                goto pforw;

            case 'D':
                flags |= LONGINT;
                /* FALLTHROUGH */
            case 'd':
            case 'i':
                ARG (signed);
                if ((long) _ulong < 0) {
                    _ulong = -_ulong;
                    sign = '-';
                }
                base = 10;
                goto number;
            case 'n':
                if (flags & LONGINT) {
                    *va_arg (argp, long *) = cnt;
                } else if (flags & SHORTINT) {
                    *va_arg (argp, short *) = cnt;
                } else {
                    *va_arg (argp, int *) = cnt;
                }
                break;
            case 'f':
                _double = va_arg (argp, double);
                if (_double < 0) {
                    _double = -_double;
                    sign = '-';
                }
                goto fpoint;
            case 'O':
                flags |= LONGINT;
                /* FALLTHROUGH */
            case 'o':
                ARG (unsigned);
                base = 8;
                goto nosign;
            case 'p':
                /*
                 * ``The argument shall be a pointer to void.  The
                 * value of the pointer is converted to a sequence
                 * of printable characters, in an implementation-
                 * defined manner.''
                 *  -- ANSI X3J11
                 */
                /* NOSTRICT */
                _ulong = (ulong) va_arg (argp, void *);
                width = 8;
                base = 16;
                flags |= (ZEROPAD | HEXPREFIX);
                goto nosign;
            case 's':
                if (!(t = va_arg (argp, char *))) {
                    t = "(null)";
                }
                if (prec >= 0) {
                    /*
                     * can't use strlen; can only look for the
                     * NUL in the first `prec' characters, and
                     * strlen() will go further.
                     */
                    char *p;

                    if ((p = memchr (t, 0, prec)) != NULL) {
                        size = p - t;
                        if (size > prec) {
                            size = prec;
                        }
                    } else {
                        size = prec;
                    }
                } else {
                    size = strlen (t);
                }
                sign = '\0';
                goto pforw;
            case 'U':
                flags |= LONGINT;
                /* FALLTHROUGH */
            case 'u':
                ARG (unsigned);
                base = 10;
                goto nosign;
            case 'X':
                digs = "0123456789ABCDEF";
                /* FALLTHROUGH */
            case 'x':
                ARG (unsigned);
                base = 16;
                /*
                 * leading 0x/X only if non-zero
                 */
                if (flags & ALT && _ulong != 0) {
                    flags |= HEXPREFIX;
                }

                /*
                 * unsigned conversions
                 */
nosign:
                sign = '\0';
                /*
                 * ``... diouXx conversions ... if a precision is
                 * specified, the 0 flag will be ignored.''
                 *  -- ANSI X3J11
                 */
number:
                if ((dprec = prec) >= 0) {
                    flags &= ~ZEROPAD;
                }

                /*
                 * ``The result of converting a zero value with an
                 * explicit precision of zero is no characters.''
                 *  -- ANSI X3J11
                 */
                t = buf + BUFSIZE;  /* let t point to end of buf */
                if (_ulong != 0 || prec != 0) {
                    do {
                        *--t = digs[_ulong % base];
                        _ulong /= base;
                    } while (_ulong);
                    if (flags & ALT && base == 8 && *t != '0') {
                        *--t = '0'; /* octal leading 0 */
                    }
                }
                size = buf + BUFSIZE - t;
                goto pforw;

fpoint:
                /*
                 * floating point converting
                 */
                t = buf + BUFSIZE;

                /* fraction */
                _ulong = (ulong)(_double * MOD) % MOD;
                _ulong = (_ulong / 10) + (_ulong % 10 > 5 ? 1 : 0); /* round off */
                for (n = 0; n < FPREC; n++) {
                    *--t = (_ulong % 10) + '0';
                    _ulong /= 10;
                }

                /* point */
                *--t = '.';

                /* decimal */
                _ulong = _double;
                do {
                    *--t = (_ulong % 10) + '0';
                    _ulong /= 10;
                } while (_ulong);

                size = buf + BUFSIZE - t;

pforw:
                /*
                 * All reasonable formats wind up here.  At this point,
                 * `t' points to a string which (if not flags&LADJUST)
                 * should be padded out to `width' places.  If
                 * flags&ZEROPAD, it should first be prefixed by any
                 * sign or other prefix; otherwise, it should be blank
                 * padded before the prefix is emitted.  After any
                 * left-hand padding and prefixing, emit zeroes
                 * required by a decimal [diouxX] precision, then print
                 * the string proper, then emit zeroes required by any
                 * leftover floating precision; finally, if LADJUST,
                 * pad with blanks.
                 */

                /*
                 * compute actual size, so we know how much to pad
                 * fieldsz excludes decimal prec; realsz includes it
                 */
                fieldsz = size + 0 /* fpprec */ ;
                realsz = dprec > fieldsz ? dprec : fieldsz;
                if (sign) {
                    realsz++;
                }

                if (flags & HEXPREFIX) {
                    realsz += 2;
                }

                /*
                 * right-adjusting blank padding
                 */
                if ((flags & (LADJUST | ZEROPAD)) == 0 && width) {
                    for (n = realsz; n < width; n++) {
                        putb_t (' ');
                    }
                }

                /*
                 * prefix
                 */
                if (sign) {
                    putb_t (sign);
                }
                if (flags & HEXPREFIX) {
                    putb_t ('0');
                    putb_t ('x');
                }

                /*
                 * right-adjusting zero padding
                 */
                if ((flags & (LADJUST | ZEROPAD)) == ZEROPAD) {
                    for (n = realsz; n < width; n++) {
                        putb_t ('0');
                    }
                }


                /*
                 * leading zeroes from decimal precision
                 */
                for (n = fieldsz; n < dprec; n++) {
                    putb_t ('0');
                }

                /*
                 * Check if we're doing fp - we may need to do some
                 * awkward padding for e/E and some g/G formats
                 */

                {
                    /*
                     * copy the string or number proper
                     */
                    int i;
                    for (i = 0; i < size; i++)
                        putb_t (t[i]);
                }



                /*
                 * left-adjusting padding (always blank)
                 */
                if (flags & LADJUST) {
                    for (n = realsz; n < width; n++) {
                        putb_t (' ');
                    }
                }

                /*
                 * finally, adjust cnt
                 */
                cnt += width > realsz ? width : realsz;
                break;
            case '\0':             /* "%?" prints ?, unless ? is NULL */
                goto EXIT;
            default:
                putb_t (*ptr);
                cnt++;
        }
    }

EXIT:
    va_end (argp);
	#ifdef I2C_DEBUG_MODE
	putb_t('\n');
	i2c_count=0x00;
	#endif
    return cnt;
}
#else

int
no_printf (const char *fmt, ...)
{
	const uchar *ptr = (const uchar *)fmt;
	
	return 0;
}
#endif

