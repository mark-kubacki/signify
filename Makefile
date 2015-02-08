all: signify

SRCS=	signify.c
SRCS+=	fe25519.c sc25519.c smult_curve25519_ref.c
SRCS+=	mod_ed25519.c mod_ge25519.c
SRCS+=	crypto_api.c
SRCS+=	ohash.c readpassphrase.c sha2.c base64.c bcrypt_pbkdf.c blf.c
SRCS+=	shafile.c randombytes.c

OBJS=$(patsubst %.c,%.o,$(SRCS))

CC=gcc -D_DEFAULT_SOURCE -DHAVE_SYS_SYSCALL_H
CFLAGS=-Os -march=corei7 -mtune=corei7 -pipe -ffunction-sections -fdata-sections -finline-functions -funroll-loops
LDFLAGS=-s -static -Wl,--gc-sections
LDLIBS=

%.o: %.c
	$(CC) $(CFLAGS) -I. -c $^

signify: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o signify $^ -I. $(LDLIBS)

signify_verify: $(SRCS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o signify_verify $^ -I. $(LDLIBS) -DVERIFYONLY

clean:
	-rm -f *.o signify signify_verify
