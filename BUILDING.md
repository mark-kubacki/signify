# Building Signify

## Prerequisites

  * [CMake](http://www.cmake.org/download/) 2.8.8 or later

  * *GCC 4.9* or later

  * Seccomp library, `libseccomp-dev` for Ubuntu and `sys-libs/libseccomp` for Gentoo

  * (optional) [Ninja](https://ninja-build.org/), known as `ninja-build` to *apt* and `dev-util/ninja` to *portage*

  * (optional) BASH to run the tests

## Building

Using Ninja:

    mkdir build
    cd $_
    CFLAGS="-Os -march=westmere -mtune=intel" cmake -GNinja ..
    ninja

Using Make:

    mkdir build
    cd $_
    CFLAGS="-Os -march=silvermont -mno-movbe -mtune=intel" cmake ..
    make

The latter *CFLFAGS* will match Intel's *Ivy Bridge* architecture sans *AVX* as well as *Silvermont*, and later.
AMD, including *Zen v1*, has a more diverse instruction set portfolio, and a common denominator looks more verbose. Sorry.

## Variants

Define `VERIFYONLY` to get a reading-only variant which does not need any entropy.

## Running tests

Run the integration tests using `test/runner.sh` with the full path up to and including the executable as first argument.
Assuming you followed the steps outlined above with `build/signify` as result:

    test/runner.sh build/signify
