/*
 * Copyright (c) 2015 W. Mark Kubacki <wmark@hurrikane.de>
 *
 * Placed in public domain.
 */

static __inline__ void
explicit_bzero(void *buf, size_t len)
{
	memset(buf, 0, len);
}
