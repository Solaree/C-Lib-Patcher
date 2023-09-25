/*
* injector.cpp
* 9/21/2023
* Updated 9/25/2023
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
        int compileResult = system("clang++ -fPIC -shared -o libpatch.so clibpatch.cpp -target armv7a-linux-androideabi19 -static-libstdc++ && clear");
        if (compileResult != 0)
        {
            cout << "[*] Compilation of libpatch.so failed. Please install and add clang++ to the PATH variable" << endl;
        }
        return 1;
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
    const char* attachToSymbol = "_Z8attachToPKcS0_";
    typedef int (*attachTo)(const char*, const char*);
    attachTo attach = reinterpret_cast<attachTo>(dlsym(handle, attachToSymbol));
    symbolWarn(attach, attachToSymbol);

    const char* detachFromSymbol = "_Z10detachFromi";
    typedef int (*detachFrom)(int);
    detachFrom detach = reinterpret_cast<detachFrom>(dlsym(handle, detachFromSymbol));
    symbolWarn(detach, detachFromSymbol);

    const char* getAllMapsSymbol = "_Z10getAllMapsPcj";
    typedef int (*getAllMapsFunction)(char*, size_t);
    getAllMapsFunction getAllMaps = reinterpret_cast<getAllMapsFunction>(dlsym(handle, getAllMapsSymbol));
    symbolWarn(getAllMaps, getAllMapsSymbol);

    const char* getMapByNameSymbol = "_Z12getMapByNamePKc";
    typedef void (*getMapByNameFunction)(const char*);
    getMapByNameFunction getMapByName = reinterpret_cast<getMapByNameFunction>(dlsym(handle, getMapByNameSymbol));
    symbolWarn(getMapByName, getMapByNameSymbol);

    const char* getPidFromPkgNameSymbol = "_Z17getPidFromPkgNamePKc";
    typedef int (*getPidFromPkgNameFunction)(const char*);
    getPidFromPkgNameFunction getPidFromPkgName = reinterpret_cast<getPidFromPkgNameFunction>(dlsym(handle, getPidFromPkgNameSymbol));
    symbolWarn(getPidFromPkgName, getPidFromPkgNameSymbol);

    const char* putUIntSymbol = "_ZN9ArmWriter7putUIntEjPKcS1_";
    typedef int (*putUIntFunction)(int);
    putUIntFunction putUInt = reinterpret_cast<putUIntFunction>(dlsym(handle, putUIntSymbol));
    symbolWarn(putUInt, putUIntSymbol);

    const char* readUIntSymbol = "_ZN9ArmReader8readUIntEj";
    typedef int (*readUIntFunction)(int);
    readUIntFunction readUInt = reinterpret_cast<readUIntFunction>(dlsym(handle, readUIntSymbol));
    symbolWarn(readUInt, readUIntSymbol);

    int pid = getPid(target_pkg); // getting pid

    attach("com.supercell.brawlstars", "libg.so"); // attaching to pid library

    putUInt(0x00000 /* your library offset here ... */, 255 /* you patch value here ... */); // patch offset
    readUInt(0x0000 /* your library offset here ... */); // read offset

    detach(pid); // detaching from pid library

    dlclose(handle);
    return 0;
}
