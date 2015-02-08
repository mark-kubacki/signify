/*
 * Copyright (c) 2014 W-Mark Kubacki <wmark@hurrikane.de>
 * Copyright (c) 2014-2015 Blitznote <mk@blitznote.com>
 * Licensed under the terms of the Reciprocal Public License, Version 1.5
 *   http://www.opensource.org/licenses/rpl1.5
 *
 */

#ifndef _RANDOMBYTES_H
#define _RANDOMBYTES_H

#include <sys/types.h>

// see <linux>/include/uapi/linux/random.h
#ifndef GRND_NONBLOCK
#define GRND_NONBLOCK 0x0001
#endif
#ifndef GRND_RANDOM
#define GRND_RANDOM 0x0002
#endif

int sys_getrandom_ex (const unsigned char*, size_t, const unsigned int);

int sys_getpseudorandom (unsigned char*, size_t);

int sys_getrandom (unsigned char*, size_t);

size_t rdrand_fill_array (size_t*, size_t);

int rdrand_getrandom (unsigned char*, size_t);

/* Fails if not Linux with syscall 'gentrandom' or if
 * the CPU doesn't have RDRAND instruction set.
 *
 * Kill-or-fill. Return 1 on success.
 */
int randombytes (unsigned char*, size_t);

#endif
