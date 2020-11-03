/*
 * Copyright (c) 2015 W. Mark Kubacki <wmark@hurrikane.de>
 *
 * Placed in public domain.
 */

#ifndef _EXPLICIT_BZERO
#define _EXPLICIT_BZERO

#include <sys/types.h>
#include <string.h>

#ifndef __OpenBSD__
#define __bounded__(a,b,c)
#endif

#if ! defined(__GLIBC_PREREQ) || ! __GLIBC_PREREQ(2,25)
static __inline__ void
explicit_bzero(void *, size_t)
        __attribute__((__bounded__(__string__,2,3)));

static __inline__ void
explicit_bzero(void *buf, size_t len)
{
	memset(buf, 0, len);
}
#endif /* ! __GLIBC_PREREQ(2,25) */

#endif /* _EXPLICIT_BZERO */
