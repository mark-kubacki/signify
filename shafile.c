#define _FILE_OFFSET_BITS 64
#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include "sha2.h"

char
fmt_tohex(char c)
{
	if (c>=10) {		// character
		return (char)(c - 10 + 'a');
	}
	return (char)(c+'0');	// number
}

size_t
fmt_hexdump(char *dest, const unsigned char *s, size_t len)
{
	size_t written=0, i;
	if (!dest) {
		if (len > ((size_t)-1) / 2) {
			return (size_t)-1;
		}
		return len*2;
	}
	for (i=0; i<len; ++i) {
		dest[written]	= fmt_tohex(s[i]>>4);
		dest[written+1]	= fmt_tohex(s[i]&15);
		written+=2;
	}
	return written;
}

#define MAX_MMAPSIZE (64*1024*1024)

char*
SHA512File(const char *filename, char *digest)
{
	int fd = open(filename, O_RDONLY);
	if (fd == -1) {
		return 0;
	}
	off_t chunk_start=0, flen;
	// determine the filesize
	{struct stat buf; fstat(fd, &buf); flen = buf.st_size;}

	uint8_t temp[SHA512_DIGEST_LENGTH];
	SHA2_CTX ctx;
	SHA512Init(&ctx);

	char *map;
	while (flen) {
		size_t n = (flen>MAX_MMAPSIZE ? MAX_MMAPSIZE : flen);
		map = mmap(NULL, flen, PROT_READ, MAP_SHARED, fd, chunk_start);
		if (map == MAP_FAILED) {
			close(fd);
			return 0;
		}
		SHA512Update(&ctx, (uint8_t*)map, flen);
		munmap(map, flen);
		flen-=n;
		chunk_start+=n;
	}
	SHA512Final(temp, &ctx);

	close(fd);
	digest[fmt_hexdump(digest, temp, sizeof(temp))] = 0;
	return digest;
}

// copy of SHA512File, only this time with SHA256
char*
SHA256File(const char *filename, char *digest)
{
	int fd = open(filename, O_RDONLY);
	if (fd == -1) {
		return 0;
	}
	off_t chunk_start=0, flen;
	// determine the filesize
	{struct stat buf; fstat(fd, &buf); flen = buf.st_size;}

	uint8_t temp[SHA512_DIGEST_LENGTH];
	SHA2_CTX ctx;
	SHA256Init(&ctx);

	char *map;
	while (flen) {
		size_t n = (flen>MAX_MMAPSIZE ? MAX_MMAPSIZE : flen);
		map = mmap(NULL, flen, PROT_READ, MAP_SHARED, fd, chunk_start);
		if (map == MAP_FAILED) {
			close(fd);
			return 0;
		}
		SHA256Update(&ctx, (uint8_t*)map, flen);
		munmap(map, flen);
		flen-=n;
		chunk_start+=n;
	}
	SHA256Final(temp, &ctx);

	close(fd);
	digest[fmt_hexdump(digest, temp, sizeof(temp))] = 0;
	return digest;
}
