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

#include <randombytes.h>

#if __clang__
	/*
	 * http://clang.llvm.org/docs/LanguageExtensions.html#feature-checking-macros
	 * http://lists.cs.uiuc.edu/pipermail/cfe-dev/2014-December/040627.html
	 */
	#if __has_attribute( noinline ) && __has_attribute( optnone )
		#define NOOPT __attribute__ (( optnone ))
		#define NOINLINE __attribute__ (( noinline ))
	#else
		#error "require clang with noinline and optnone attributes"
	#endif
#elif __GNUC__
	/*
	 * http://gcc.gnu.org/onlinedocs/gcc/Function-Specific-Option-Pragmas.html
	 * http://gcc.gnu.org/onlinedocs/gcc/Function-Attributes.html
	 */
	#if __GNUC__ > 4 || ( __GNUC__ == 4 && __GNUC_MINOR__ >= 4 )
		#define NOOPT __attribute__ (( optimize( 0 ) ))
		#define NOINLINE __attribute__ (( noinline ))
	#else
		#error "require gcc >= 4.4"
	#endif
#else
	#error "unrecognised compiler"
	explode
#endif

/* START: Linux >= 3.18 system call 'getrandom' */
#ifndef SYS_getrandom

#if (defined(__i386)   || defined(__i386__)   || defined(_M_IX86) || \
     defined(__x86_64) || defined(__x86_64__))
#ifdef __LP64__
#define SYS_getrandom 318
#else
#define SYS_getrandom 355
#endif // __LP64__
#endif // x86 or amd64

#ifdef __aarch64__
#define SYS_getrandom 384
#endif

#endif // !defined(SYS_getrandom)

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
		} while (ret == -1 && errno == EINTR);
		if (ret < 0) return ret;

		p += ret;
		buflen -= ret;
	}

	errno = pre_errno;
	return 1;
}

int
sys_getpseudorandom (unsigned char *buf, size_t buflen)
{
	if (buflen < 0) return (-1);
	return sys_getrandom_ex(buf, buflen, GRND_NONBLOCK);
}

int
sys_getrandom (unsigned char *buf, size_t buflen)
{
	if (buflen < 0) return (-1);
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
	__cpuid(7, eax, ebx, ecx, edx);
	cached_cpu_supports_rdseed = !!(ebx & bit_RDSEED);
}


NOOPT NOINLINE size_t
rdrand_fill_array (size_t *array, size_t size)
{
	size_t successes = size;

	__asm volatile(
		"jecxz end_of_rdrand_loop%=;\n"	// jump if ecx (size) == 0

		"top_of_rdrand_loop%=:\n"
#ifdef __LP64__
		"rdrand %%rax;\n"		// Generate random value
		"jnc end_of_rdrand_loop%=;\n"	// bail on first failure
		"mov %%rax, (%1);\n "		// Store value in array
		"add $8, %1;\n "		// Move array to next spot
#else
		"rdrand %%eax;\n"
		"jnc end_of_rdrand_loop%=;\n"
		"mov %%eax, (%1);\n "
		"add $4, %1;\n "
#endif
		"loop top_of_rdrand_loop%=;\n"	// --ecx; jump if ecx > 0

		"end_of_rdrand_loop%=:\n"
		"sub %4, %0;\n"			// filled = size - remaining
		: "=r" (successes), "=r"(array)
		: "0" (successes), "1"(array), "c" (size)
#ifdef __LP64__
		: "%rax"
#else
		: "%eax"
#endif
	);

	return successes;
}

int
rdrand_getrandom (unsigned char *buf, size_t buflen)
{
	size_t num_qwords = buflen/sizeof(size_t);
	buflen = buflen % sizeof(size_t);
	size_t *p = (size_t *)buf;

	if (rdrand_fill_array(p, num_qwords) != num_qwords) {
		return (-1); // happens on rand-busy AVX-less machines
	}
	p+=num_qwords;
	// due to num < sizeof(size_t) a single size_t suffices
	if (buflen > 0) {
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
