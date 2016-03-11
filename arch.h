/**
 * arch.h:
 *
 * Archetecture-specific integral types for Codemon project
 * CIS*2750F15 (prototypes/data definitions).
 *
 * @author   David McCaughan
 * @version  2.0, 2015/08/11
 *
 */

#ifndef ARCH_H
#define ARCH_H

#include <limits.h>


/*
 * Note that these were set specifically to match both the 64-bit OSX
 * environment, and 32-bit Linux.  int64 on Mac would suffice with long.
 */
typedef char int8;
typedef unsigned char uint8;
typedef short int16;
typedef unsigned short uint16;
typedef int int32;
typedef unsigned int uint32;
typedef long long int64;
typedef unsigned long long uint64;


#define INT8MAX    SCHAR_MAX
#define UINT8MAX   UCHAR_MAX
#define INT16MAX   SHRT_MAX
#define UINT16MAX  USHRT_MAX
#define INT32MAX   INT_MAX
#define UINT32MAX  UINT_MAX
#define INT64MAX   LLONG_MAX
#define UINT64MAX  ULLONG_MAX

#endif