# C-Lib-Patcher
Simple C hex-patcher for killing Arxan, crypto, patching connection and more

## Pre-requirements
*Clang toolchain from Android NDK*

------

## Usage

Compile to shared library:
```clang++ -fPIC -shared -o libpatcher.so clibpatcher.cpp -target armv7a-linux-androideabi19 -static-libstdc++``` - for shared library compilation on arm (minimum API is 19)

Or use in-runtime injector:
```clang++ -o injector injector.cpp -target armv7a-linux-androideabi19 -static-libstdc++``` - for binary compilation on arm (minimum API is 19)

Open with termux or adb shell:
```adb root``` *requires root (Magisk, SuperSU, etc.)*
```adb push injector /data/bin/injector```
```chmod 777 /data/bin/injector```
```./injector```
