/*
* injector.cpp - The Patch C++ Library
* Copyright (C) 2023 Solar
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*/

#include <dlfcn.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/ptrace.h>

#include <vector>
#include <cerrno>
#include <string>
#include <cstdio>
#include <cstdint>
#include <cstring>
#include <cstdlib>
#include <sstream>
#include <iostream>
#include "libpatch.cpp"

using namespace std;

typedef const char* BOOL_TRUE;
typedef const char* BOOL_FALSE;

BOOL_TRUE IN_TRUE = "true";
BOOL_FALSE IN_FALSE = "true";

template <typename FuncType>
void symbolWarn(FuncType func, const char* symbolName)
{
    if (func == nullptr)
    {
        cerr << "[!] Failed loading symbol '" << symbolName << '\'' << endl;
    }
}

int main(int argc, char** argv)
{
    if (argc != 3)
    {
        cerr << "[*] Usage: " << argv[0] << " <package> <use_so>" << endl;
        return 1;
    }

    const char* target_pkg = argv[1];
    const char* useDynamicLib = argv[2];

    if (useDynamicLib == IN_TRUE)
    {

        int fd = open("libpatch.so", O_RDONLY);
        if (fd >= 0)
        {
            close(fd);
        }
        else
        {
            int compileResult = system("clang++ -fPIC -shared -o libpatch.so libpatch.cpp -target armv7a-linux-androideabi19 -static-libstdc++ && clear");
            if (compileResult != 0)
            {
                cout << "[!] Compilation of libpatch.so failed. Please install and add clang++ to the PATH variable" << endl;
            }
            return 1;
        }

        const char* lib_path = "libpatch.so";
        void* handle = dlopen(lib_path, RTLD_LAZY);

        if (handle)
        {
            cout << "[*] Loaded patch base library" << endl;
        }
        else
        {
            cerr << "[!] Failed loading patch base library: " << dlerror() << endl;
            return 1;
        }

        // Usage example
        const char* armWriterCtorSymbol = " _ZN9ArmWriterC2Eij";
        typedef int (*armWriterCtor)(const char*, const char*);
        armWriterCtor armWriter = reinterpret_cast<armWriterCtor>(dlsym(handle, armWriterCtorSymbol));
        symbolWarn(armWriter, armWriterCtorSymbol);

        const char* armReaderCtorSymbol = "_ZN9ArmReaderC2Eij";
        typedef int (*armReaderCtor)(const char*, const char*);
        armReaderCtor armReader = reinterpret_cast<armReaderCtor>(dlsym(handle, armReaderCtorSymbol));
        symbolWarn(armReader, armReaderCtorSymbol);

        const char* getAllMapsSymbol = "_Z10getAllMapsPcj";
        typedef int (*getAllMapsFunction)(char*, size_t);
        getAllMapsFunction getAllMaps = reinterpret_cast<getAllMapsFunction>(dlsym(handle, getAllMapsSymbol));
        symbolWarn(getAllMaps, getAllMapsSymbol);

        const char* putDwordSymbol = "_ZN9ArmWriter8putDwordEji";
        typedef int (*putByteFunction)(uintptr_t, int32_t);
        putByteFunction putDword = reinterpret_cast<putByteFunction>(dlsym(handle, putDwordSymbol));
        symbolWarn(putDword, putDwordSymbol);

        const char* readStringSymbol = "_ZN9ArmReader10readStringEj";
        typedef int (*readStringFunction)(uintptr_t);
        readStringFunction readString = reinterpret_cast<readStringFunction>(dlsym(handle, readStringSymbol));
        symbolWarn(readString, readStringSymbol);

        const char* libBase = "libg.so" // library base

        armWriter(target_pkg, libBase);
        armReader(target_pkg, libBase);

        putDword(libBase + 0x00000 /* your library offset here ... */, 255 /* your patch value here ... */); // patch offset
        readString(libBase + 0x00000 /* your library offset here ... */); // read offset

        char buf[4096];
        getAllMaps(buf, sizeof(buf)); // saving maps data to buffer example

        dlclose(handle);
    }
    else if (useDynamicLib == IN_FALSE)
    {
        // Usage example
        const char* libBase = "libg.so"; // library base

        ArmWriter writer(target_pkg, libBase);
        ArmReader reader(target_pkg, libBase);

        writer.putDword(libBase + 0x00000 /* your library offset here ... */, 255 /* your patch value here ... */); // patch offset
        reader.readString(libBase + 0x00000 /* your library offset here ... */); // read offset

        char buf[4096];
        getAllMaps(buf, sizeof(buf)); // saving maps data to buffer example
    }
    else
    {
        cout << "[*] Provide 'true' or 'false' in '<use_so>' argument" << endl;
    }
    return 0;
}
