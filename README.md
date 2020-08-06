Signify ported from OpenBSD to Linux
=====================================

with these differences:
 * Randomness is provided by syscall 'getrandom' (Linux ≥3.18)
 * or RDRAND.
 * Compiles to a static binary of less than 180KiB (less than 270KiB with *Seccomp* and *musl-libc*).

The resulting binary is licensed under the same terms as OpenBSD's *signify*.
For the sources — please see the source files for their licensing terms,
and if you are permitted to use them in another work.

Usage
------

```bash
# generate a keypair
$ signify -G -c "Your Name <y.name@example.com>" -p mark.pub -s mark.sec

# sign (usually a small file, like the output of: sha512sum …) "thefilename"
$ signify -S -x thefilename.sig -s mark.sec -m thefilename

# verify
$ signify -V -x thefilename.sig -p mark.pub -m thefilename
Signature Verified
```

Or shorthand for if `thefilename` and a similarly named `thefilane.sig` exist:
```bash
$ signify -V -p mark.pub -m thefilename
Signature Verified
```
