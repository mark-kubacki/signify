/*
 * Copyright (c) 2014 W-Mark Kubacki <wmark@hurrikane.de>
 * Copyright (c) 2014-2015 Blitznote <mk@blitznote.com>
 * Licensed under the terms of the Reciprocal Public License, Version 1.5
 *   http://www.opensource.org/licenses/rpl1.5
 *
 */

#ifndef _RANDOMBYTES_C
#define _RANDOMBYTES_C

#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <sys/syscall.h>
#ifdef HAVE_SYS_SYSCTL_H
#include <sys/sysctl.h>
#endif
#include "cpuid.h"
#ifndef bit_RDRND
#define bit_RDRND (1 << 30)
#endif
#ifndef bit_RDSEED
#define bit_RDSEED (1 << 18)
#endif

#if !defined(likely)
#define likely(x)       __builtin_expect(!!(x), 1)
#define unlikely(x)     __builtin_expect(!!(x), 0)
#endif

#include <randombytes.h>

#ifdef SYS_getrandom
// see <linux>/include/uapi/linux/random.h
#ifndef GRND_NONBLOCK
#define GRND_NONBLOCK 0x0001
#endif
#ifndef GRND_RANDOM
#define GRND_RANDOM 0x0002
#endif

int
sys_getrandom_ex (const unsigned char *buf, size_t buflen, const unsigned int flags)
{
	int pre_errno = errno;
	unsigned char *p = (unsigned char *)buf;

	while (buflen) {
		int ret = 0; // ret < 0 is an error, else: number of bytes returned
		int chunk = buflen > 256 ? 256 : buflen; // min(256, buflen);

		do {
			ret = syscall(SYS_getrandom, p, chunk, flags);
		} while (unlikely(ret == -1 && errno == EINTR));
		if (unlikely(ret < 0)) return ret;

		p += ret;
		buflen -= ret;
	}

	errno = pre_errno;
	return 1;
}

int
sys_getpseudorandom (unsigned char *buf, size_t buflen)
{
	if (unlikely(buflen < 0)) return (-1);
	return sys_getrandom_ex(buf, buflen, GRND_NONBLOCK);
}

int
sys_getrandom (unsigned char *buf, size_t buflen)
{
	if (unlikely(buflen < 0)) return (-1);
	return sys_getrandom_ex(buf, buflen, GRND_RANDOM);
}

#endif // defined(SYS_getrandom)


/* START: RDRAND and RDSEED support */
#ifdef CPU_RAND
static int cached_cpu_supports_rdrand;
static int cached_cpu_supports_rdseed;

void __attribute__((constructor))
init_cpu_support_flag(void)
{
	unsigned int eax=0, ebx=0, ecx=0, edx=0;
	__cpuid(1, eax, ebx, ecx, edx);
	cached_cpu_supports_rdrand = !!(ecx & bit_RDRND);
	eax=0, ebx=0, ecx=0, edx=0;
	__cpuid_count(7, 0, eax, ebx, ecx, edx);
	cached_cpu_supports_rdseed = !!(ebx & bit_RDSEED);
}

/* Fills the 'array' of size 'size' with random numbers.
 * Is not guaranteed to fill every cell (due to unavailable entropy)
 * but at least returns the count of filled cells so you could retry.
 */
size_t
rdrand_fill_array (size_t *array, size_t size)
{
	if (unlikely(size == 0)) {
		return 0;
	}
	size_t total = size;

	do {
		size_t scratch;
	#if defined(__clang__) || defined(__INTEL_COMPILER)
		int ok;
		asm volatile("rdrand %1\n"
			"setc %0"
			: "=qm"(ok), "=a"(scratch));
		if (unlikely(!ok)) break;
	#else /* GCC */
		asm volatile("rdrand %0" : "=a"(scratch));
		asm goto("jnc %l0" :::: end);
	#endif
		*array = scratch;
		++array;
		--size;
	} while(likely(size > 0));
end:
	return (total - size);
}

int
rdrand_getrandom (unsigned char *buf, size_t buflen)
{
	size_t num_qwords = buflen/sizeof(size_t);
	buflen = buflen % sizeof(size_t);
	size_t *p = (size_t *)buf;

	if (unlikely(rdrand_fill_array(p, num_qwords) != num_qwords)) {
		return (-1); // happens on rand-busy AVX-less machines
	}
	p+=num_qwords;
	// due to num < sizeof(size_t) a single size_t suffices
	if (unlikely(buflen > 0)) {
		// use num_qwords as scratch space
		if (rdrand_fill_array(&num_qwords, 1) != 1) {
			return (-2);
		}
		memcpy((void *)p, &num_qwords, buflen);
	}

	return 1;
}

#endif // defined(CPU_RAND)


/* Fails if not Linux with syscall 'gentrandom' or if
 * the CPU doesn't have RDRAND instruction set.
 *
 * Kill-or-fill. Return 1 on success.
 */
int
randombytes (unsigned char *buf, size_t buflen)
{
#ifdef SYS_getrandom
	if (sys_getpseudorandom(buf, buflen) == 1) {
		return 1;
	}
#else
#warning "unknown syscall number: SYS_getrandom"
#endif

#ifdef CPU_RAND
	if (cached_cpu_supports_rdrand && rdrand_getrandom(buf, buflen) == 1) {
		return 1;
	}
#else
#warning "architecture without built-in random number generator"
#endif

	return (-1);
}

#endif
