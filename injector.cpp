/*
* injector.cpp 
* 9/21/2023
*/

#include <dlfcn.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>

using namespace std;

template<typename FuncType>
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
        cerr << "[*] Usage: " << argv[0] << " <package>" << endl;
        return 1;
    }

    int fd = open("libpatch.so", O_RDONLY);
    if (fd >= 0)
    {
        close(fd);
    }
    else
    {
        system("clang++ -fPIC -shared -o libpatch.so clibpatch.cpp -target armv7a-linux-androideabi19 -static-libstdc++ && clear");
    }

    const char* target_pkg = argv[1];
    const char* lib_path = "libpatch.so";
    void* handle = dlopen(lib_path, RTLD_LAZY);

    if (handle)
    {
        cout << "[*] Loaded base library" << endl;
    }
    else
    {
        cerr << "[!] Failed loading base library: " << dlerror() << endl;
        return 1;
    }

    // Usage example
    const char* getPidFromPkgNameSymbol = "_Z17getPidFromPkgNamePKc";
    typedef int (*getPidFromPkgName)(const char*);
    getPidFromPkgName getPid = reinterpret_cast<getPidFromPkgName>(dlsym(handle, getPidFromPkgNameSymbol));
    symbolWarn(getPid, getPidFromPkgNameSymbol);

    int pid = getPid(target_pkg);

    const char* attachToSymbol = "_Z8attachToPKcS0_";
    typedef int (*attachTo)(int);
    attachTo attach = reinterpret_cast<attachTo>(dlsym(handle, attachToSymbol));
    symbolWarn(attach, attachToSymbol);

    attach(pid);

    const char* patchSymbol = "_ZN9ArmWriter9putValOneEj";
    typedef int (*putValOneFunction)(int);
    putValOneFunction putValOne = reinterpret_cast<putValOneFunction>(dlsym(handle, patchSymbol));
    symbolWarn(putValOne, patchSymbol);

    putValOne(0x00000 /* your library offset here ...*/);

    dlclose(handle);
    return 0;
}
