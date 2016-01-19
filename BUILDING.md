# Building Signify

## Prerequisites

  * [CMake](http://www.cmake.org/download/) 2.8.8 or later

  * *GCC 4.9* or later

  * Seccomp library, `libseccomp-dev` for Ubuntu and `sys-libs/libseccomp` for Gentoo

  * (optional)[Ninja](https://ninja-build.org/), known as `ninja-build` to *apt* and `dev-util/ninja` to *portage*

  * (optional)BASH to run the tests

## Building

Using Ninja:

    mkdir build
    cd $_
    CFLAGS="-Os -march=corei7 -mtune=corei7" cmake -GNinja ..
    ninja

Using Make:

    mkdir build
    cd $_
    CFLAGS="-Os -march=corei7 -mtune=corei7" cmake ..
    make

## Variants

Define `VERIFYONLY` to get a reading-only variant which does not need any entropy.

## Running tests

Run the integration tests using `test/runner.sh` with the full path up to and including the executable as first argument.
Assuming you followed the steps outlined above with `build/signify` as result:

    test/runner.sh build/signify
