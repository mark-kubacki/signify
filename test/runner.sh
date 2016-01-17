#!/bin/bash

set -e -o pipefail

if [[ ! -e "$1" ]]; then
  >&2 printf "Usage: $0 <path-to/signify>\n"
  exit 1
fi
EXE="$(realpath "$1")"

declare -a on_exit_items

on_exit() {
  for i in "${on_exit_items[@]}"; do
    eval $i
  done
}

add_on_exit() {
  local n=${#on_exit_items[*]}
  on_exit_items[$n]="$*"
  if [[ $n -eq 0 ]]; then
    trap on_exit EXIT
  fi
}

if grep --max-count=1 -F flags /proc/cpuinfo 2>/dev/null | grep -q -F 'rdrand'; then
  host_cpu_has_rdrand=true
fi
if printf "3.16.9999\n$(uname -r)\n" | sort --reverse -V | head -n 1 | grep -q -F "$(uname -r)"; then
  host_is_ge_linux_3_17=true
fi

# Something is broken if we always get the same keypair.
test_creates_distinct_keypairs() {
  local WORKDIR=$(mktemp -d -t signify.XXXXXX)
  add_on_exit "rm -rf '$WORKDIR'"
  cd $WORKDIR

  printf "geheim\ngeheim\n" | "$EXE" -G -c "somecomment" -p foo.pub -s foo.sec
  printf "geheim\ngeheim\n" | "$EXE" -G -c "somecomment" -p bar.pub -s bar.sec

  ! >/dev/null cmp foo.pub bar.pub
  ! >/dev/null cmp foo.sec bar.sec
}

# Resembles a typical usage.
test_sign_and_verify() {
  local WORKDIR=$(mktemp -d -t signify.XXXXXX)
  add_on_exit "rm -rf '$WORKDIR'"
  cd $WORKDIR

  # generate keys
  printf "geheim\ngeheim\n" | "$EXE" -G -c "somecomment" -p foo.pub -s foo.sec
  # sign something - we use exemplarily use the binary as message to be signed
  printf "geheim\n" | "$EXE" -S -x foo-signed.sig -s foo.sec -m "$EXE"
  # verify
  >/dev/null 2>&1 "$EXE" -V -x foo-signed.sig -p foo.pub -m "$EXE"

  # now tamper with the signature
  cat foo-signed.sig | tr '[a-z]' '[b-z]a' > manipulated.sig
  # … which is expected to NOT verify
  ! >/dev/null 2>&1 "$EXE" -V -x manipulated.sig -p foo.pub -m "$EXE"
}

# Passes if we accept someone else's input.
test_verify_real_example() {
  local WORKDIR=$(mktemp -d -t signify.XXXXXX)
  add_on_exit "rm -rf '$WORKDIR'"
  cd $WORKDIR

  curl --fail --silent --show-error --location --remote-name-all \
    https://github.com/Blitznote/signify/releases/download/1.100/{mark.pub,signify.sig,signify}
  mv signify some-binary

  >/dev/null 2>&1 "$EXE" -V -x signify.sig -p mark.pub -m some-binary
}

# Does our format and algorithm equal that of OpenBSD's signify?
test_is_compatible_with_openbsds() {
  local WORKDIR=$(mktemp -d -t signify.XXXXXX)
  add_on_exit "rm -rf '$WORKDIR'"
  cd $WORKDIR

  curl --fail --silent --show-error --location --remote-name-all \
    http://ftp.hostserver.de/pub/OpenBSD/5.6/amd64/SHA256.sig \
    https://github.com/jpouellet/signify-osx/raw/master/src/etc/signify/openbsd-56-base.pub

  >/dev/null "$EXE" -Vep openbsd-56-base.pub -x SHA256.sig -m -
}

if [ "${host_cpu_has_rdrand}" = true ] || \
   [ "${host_is_ge_linux_3_17}" = true ]; then
  test_creates_distinct_keypairs; >&2 printf "#"
  test_sign_and_verify; >&2 printf "#"
else
  >&2 printf "old host CPU; some tests have been skipped\n"
fi

test_verify_real_example; >&2 printf "#"
test_is_compatible_with_openbsds; >&2 printf "#"
