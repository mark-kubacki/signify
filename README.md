Signify ported from OpenBSD to Linux
=====================================
[![Build Status](https://drone.io/github.com/Blitznote/signify/status.png)](https://drone.io/github.com/Blitznote/signify/latest)

with these differences:
 * Randomness is provided by syscall 'getrandom' to Linux (>= 3.18)
 * or RDRAND.
 * Compiles to a static binary of less than 180KiB.

The resulting binary is licensed under the same terms as OpenBSD's *signify*.
For the sources — please see the source files for their licensing terms,
and if you are permitted to use them in another work.

Usage
------

You still need to distribute your signing keys separately,
by a different channel, to be trustworthy.

In this example the binary file *signify* gets signed and verified:
```bash
# generate a keypair
$ signify -G -c "W. Mark Kubacki <wmark@hurrikane.de>" -p mark.pub -s mark.sec

# sign (usually a small file, like the output of: sha512sum …)
$ signify -S -x signify.sig -s mark.sec -m signify

# verify
$ signify -V -x signify.sig -p mark.pub -m signify
Signature Verified
```

Let X be the message (»file«), if X{,.sig} exist the signature file gets found automatically and you can write:
```bash
$ signify -V -p mark.pub -m X
Signature Verified
```
