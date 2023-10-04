# C-Lib-Patcher
Simple C hex-patcher for killing Arxan, crypto, patching connection and more

## Pre-requirements
Currently needed only official **LLVM** toolchain from Andoroid NDK, avilable *[here](https://developer.android.com/ndk/downloads)*

---

Injector compilation: ```make```

The patch library compilation: ```clang++ -fPIC -shared -o libpatch.so libpatch.cpp -target armv7a-linux-androideabi19 -static-libstdc++```

See examples in ```injector.cpp``` and edit patches for your own!

---

## Usage

Load binary with termux or adb shell:
```adb root``` *requires root (Magisk, SuperSU, etc.)*
```adb push injector /data/bin/injector```
```chmod 777 /data/bin/injector```
```./injector```
