# C-Lib-Patcher
Simple C hex-patcher for killing Arxan, crypto, patching connection and more

### *Can be compiled to shared binary and put in the game, or compiled as executable and launched in internal storage*

## Pre-requirements
*Clang, Clang++ toolchain from Android NDK*

------

## Usage
```clang140 libsolar.c -o libsolar -target armv7a-linux-androideabi19 -s``` - for binary compilation

------

```clang140 -fPIC -shared libsolar.c -o libsolar -target armv7a-linux-androideabi19 -s``` - for shared library compilation
*Also can be compiled with Clang++ and '-static-libstdc++' flag*

*Current pushed version has Brawl Stars v36.218.1 Arxan hex-patching and NaCl crypto hex-patching
