# C-Lib-Patcher
Simple C hex-patcher for killing Arxan, crypto, patching connection and more

## Pre-requirements
*Clang, Clang++ toolchain from Android NDK*

------

## Usage
```clang140 -o libsolar libsolar.c -target armv7a-linux-androideabi19 -s``` - for binary compilation on arm (minimum API is 19)

------

```clang140 -fPIC -shared -o libsolar.so libsolar.c -target armv7a-linux-androideabi19 -s``` - for shared library compilation on arm (minimum API is 19)

------

*Also can be compiled with Clang++ and '-static-libstdc++' flag*
