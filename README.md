# C-Lib-Patcher
Simple C hex-patcher for killing Arxan, crypto, patching connection and more

## Pre-requirements
*Clang toolchain from Android NDK*

------

## Usage

```clang++ -fPIC -shared -o libpatcher.so clibpatcher.cpp -target armv7a-linux-androideabi19 -s``` - for shared library compilation on arm (minimum API is 19)
