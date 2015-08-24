all: signify

SRCS=	signify.c
SRCS+=	fe25519.c sc25519.c smult_curve25519_ref.c
SRCS+=	mod_ed25519.c mod_ge25519.c
SRCS+=	crypto_api.c
SRCS+=	ohash.c readpassphrase.c sha2.c base64.c bcrypt_pbkdf.c blf.c
SRCS+=	shafile.c randombytes.c

OBJS=$(patsubst %.c,%.o,$(SRCS))

CC=gcc -D_DEFAULT_SOURCE -DHAVE_SYS_SYSCALL_H -Wnonnull
CFLAGS=-Os -march=corei7 -mtune=corei7 -pipe -ffunction-sections -fdata-sections -finline-functions
LDFLAGS=-s -static -Wl,--gc-sections
LDLIBS=

%.o: %.c
	$(CC) $(CFLAGS) -I. -c $^

signify: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -fwhole-program -o signify $^ -I. $(LDLIBS)

signify_verify: $(SRCS)
	$(CC) $(CFLAGS) $(LDFLAGS) -fwhole-program -o signify_verify $^ -I. $(LDLIBS) -DVERIFYONLY

clean:
	-rm -f *.o signify signify_verify

CPUFLAGS	:= $(shell sh -c 'grep --max-count=1 -F flags /proc/cpuinfo 2>/dev/null || echo not')
GE_LINUX_3_17	:= $(shell sh -c 'printf "3.16.9999\n`uname -r`\n" | sort --reverse -V | head -n 1 | grep -q -F "`uname -r`" && echo aye || echo not')
ifneq (,$(findstring rdrand,$(CPUFLAGS)))
	HOST_HAS_RAND = Yes
else
	ifneq (,$(findstring aye,$(GE_LINUX_3_17)))
		HOST_HAS_RAND = Yes
	endif
endif

integration-test: signify
ifndef HOST_HAS_RAND
	@echo "old host CPU; some tests have been skipped" >&2
else
	@tmpdir=`mktemp --tmpdir -d`; trap 'rm -rf "$$tmpdir"' EXIT; cd "$$tmpdir"; \
	   printf "geheim\ngeheim\n" | $(CURDIR)/signify -G -c "somecomment" -p foo.pub -s foo.sec \
	&& printf "geheim\ngeheim\n" | $(CURDIR)/signify -G -c "somecomment" -p bar.pub -s bar.sec \
	&& ! cmp foo.pub bar.pub >/dev/null && ! cmp foo.sec bar.sec >/dev/null \
	&& >&2 printf "seems to create distinct keypairs\n"

	@tmpdir=`mktemp --tmpdir -d`; trap 'rm -rf "$$tmpdir"' EXIT; cd "$$tmpdir"; \
	   printf "geheim\ngeheim\n" | $(CURDIR)/signify -G -c "somecomment" -p foo.pub -s foo.sec \
	&& printf "geheim\n" | $(CURDIR)/signify -S -x foo-signed.sig -s foo.sec -m $(CURDIR)/signify \
	&&   $(CURDIR)/signify -V -x foo-signed.sig  -p foo.pub -m $(CURDIR)/signify >/dev/null \
	&& cat foo-signed.sig | tr '[a-z]' '[b-z]a' > manipulated.sig \
	&& ! $(CURDIR)/signify -V -x manipulated.sig -p foo.pub -m $(CURDIR)/signify >/dev/null 2>&1 \
	&& >&2 printf "signing and verifying seems to work\n"
endif

	@tmpdir=`mktemp --tmpdir -d`; trap 'rm -rf "$$tmpdir"' EXIT; cd "$$tmpdir"; \
	curl --fail --silent --show-error -LR --remote-name-all \
		https://github.com/Blitznote/signify/releases/download/1.100/mark.pub \
		https://github.com/Blitznote/signify/releases/download/1.100/signify.sig \
		https://github.com/Blitznote/signify/releases/download/1.100/signify \
	&& mv signify some-binary \
	&& >&2 $(CURDIR)/signify -V -x signify.sig -p mark.pub -m some-binary

	@tmpdir=`mktemp --tmpdir -d`; trap 'rm -rf "$$tmpdir"' EXIT; cd "$$tmpdir"; \
	curl --fail --silent --show-error -LR --remote-name-all \
		http://ftp.hostserver.de/pub/OpenBSD/5.6/amd64/SHA256.sig \
		https://github.com/jpouellet/signify-osx/raw/master/src/etc/signify/openbsd-56-base.pub \
	&& $(CURDIR)/signify -Vep openbsd-56-base.pub -x SHA256.sig -m - > /dev/null \
	&& >&2 printf "compatible with OpenBSD's way of signing\n"
