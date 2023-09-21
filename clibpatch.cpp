/*
* clibpatch.cpp 
* 9/14/2023
*/

#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <cerrno>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

using namespace std;

class ArmWriter
{
public:
    ArmWriter(pid_t pid, uintptr_t base);

    void putBytes(uintptr_t offset, const char* bytes);
    void putStaticBytes(const char* libpath, uintptr_t offset, const char* bytes);
    void putRet(uintptr_t offset);
    void putNop(uintptr_t offset);
    void putValZero(uintptr_t offset);
    void putValOne(uintptr_t offset);
    void putUInt(uintptr_t offset, const char* high, const char* little);
    void putStringR0(uintptr_t strOffset, uintptr_t loadOffset, const char* off1, const char* off2, const char* retval);
    void putStringR1(uintptr_t strOffset, uintptr_t loadOffset, const char* off1, const char* off2, const char* retval);
    void protect(uintptr_t off_start, size_t size, const char* perms);

private:
    pid_t PID;
    uintptr_t libBase;
};
ArmWriter::ArmWriter(pid_t pid, uintptr_t base) : PID(pid), libBase(base)
{
}
void ArmWriter::putBytes(uintptr_t offset, const char* bytes)
{
    uintptr_t patchAddress = libBase + offset;
    ptrace(PTRACE_POKETEXT, PID, (void*)patchAddress, (void*)strtoul(bytes, NULL, 16));
}
void ArmWriter::putStaticBytes(const char* libpath, uintptr_t offset, const char* bytes)
{
    int fd = open(libpath, O_RDWR);
    struct stat libstat;
    fstat(fd, &libstat);
    void* mappaddr = mmap(NULL, libstat.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (mappaddr == MAP_FAILED)
    {
        perror("[!] Failed to mmap library file\n");
        close(fd);
        return;
    }
    uintptr_t start_address = (uintptr_t)mappaddr + offset;
    uintptr_t page_start = start_address & ~(4095UL);
    size_t size = (start_address - page_start) + 4;
    protect(page_start, size, "rw");
    for (int i = 0; i < 4; i++)
    {
        *(unsigned char*)(start_address + i) = strtol(bytes + (i * 2), NULL, 16);
    }
    protect(page_start, size, "rx");
    msync(mappaddr, libstat.st_size, MS_SYNC);
    munmap(mappaddr, libstat.st_size);
    close(fd);
}
void ArmWriter::putRet(uintptr_t offset)
{
    putBytes(offset, "1E FF 2F E1"); // BX LR
}
void ArmWriter::putNop(uintptr_t offset)
{
    putBytes(offset, "00 00 00 EA"); // NOP
}
void ArmWriter::putValZero(uintptr_t offset)
{
    putBytes(offset, "00 00 A0 E3"); // MOV R0, #0
}
void ArmWriter::putValOne(uintptr_t offset)
{
    putBytes(offset, "01 00 A0 E3"); // MOV R0, #1
}
void ArmWriter::putUInt(uintptr_t offset, const char* high, const char* little)
{
    string instr = string(high) + " " + little + "A0 E3";
    putBytes(offset, instr.c_str());
}
void ArmWriter::putStringR0(uintptr_t strOffset, uintptr_t loadOffset, const char* off1, const char* off2, const char* retval)
{
    putBytes(strOffset, retval);
    putBytes(loadOffset, (string(off1) + " " + off2 + "9F E5").c_str());
    putBytes(loadOffset + 4, "00 00 8F E0"); // ADD R0, PC, R0
}
void ArmWriter::putStringR1(uintptr_t strOffset, uintptr_t loadOffset, const char* off1, const char* off2, const char* retval)
{
    putBytes(strOffset, retval);
    putBytes(loadOffset, (string(off1) + " " + off2 + "9F E5").c_str());
    putBytes(loadOffset + 4, "01 10 8F E0"); // ADD R1, PC, R1
}
void ArmWriter::protect(uintptr_t off_start, size_t size, const char* perms)
{
    int prot = 0;
    if (strcmp(perms, "rwx") == 0 || strcmp(perms, "777") == 0)
    {
        prot = PROT_READ | PROT_WRITE | PROT_EXEC;
    }
    else if (strcmp(perms, "rw") == 0 || strcmp(perms, "766") == 0)
    {
        prot = PROT_READ | PROT_WRITE;
    }
    else if (strcmp(perms, "rx") == 0 || strcmp(perms, "755") == 0)
    {
        prot = PROT_READ | PROT_EXEC;
    }
    else if (strcmp(perms, "r") == 0 || strcmp(perms, "444") == 0)
    {
        prot = PROT_READ;
    }
    else
    {
        cerr << "[!] ArmWriter::protect() ERROR: provide 'rwx' (777), 'rw' (766), 'rx' (755) or 'r' (444)" << endl;
        return;
    }
}
int getAllMaps(char* buf, size_t bufSize)
{
    int fd = open("/proc/self/maps", O_RDONLY);
    ssize_t bytesRead = read(fd, buf, bufSize - 1);
    buf[bytesRead] = '\0';
    close(fd);
    return bytesRead;
}
void getMapByName(const char* lib)
{
    char buf[4096];
    ssize_t bytesRead = getAllMaps(buf, sizeof(buf));
    char* line = buf;
    while (*line != '\0')
    {
        if (strstr(line, lib) != nullptr)
        {
            cout << "[*] Lib loaded in /proc/self/maps" << endl;
            break;
        }
        line = strchr(line, '\n');
        if (line == nullptr)
        {
            break;
        }
        line++;
    }
}
void getProcesses()
{
    DIR* dir = opendir("/data/app");
    dirent* entry;
    while ((entry = readdir(dir)))
    {
        cout << "[*] Base apk process: " << entry->d_name << endl;
    }
    closedir(dir);
}
int getLoginUid()
{
    int fd = open("/proc/self/loginuid", O_RDONLY);
    char buf[4096];
    ssize_t bytesRead = read(fd, buf, sizeof(buf) - 1);
    buf[bytesRead] = '\0';
    close(fd);
    return atoi(buf);
}
int getPidFromPkgName(const char* PACKAGE)
{
    char path[512];
    DIR* dir = opendir("/proc");
    struct dirent* ent;
    while ((ent = readdir(dir)) != nullptr)
    {
        if (isdigit(*ent->d_name))
        {
            snprintf(path, sizeof(path), "/proc/%s/cmdline", ent->d_name);
            FILE* cmdline = fopen(path, "r");
            if (cmdline)
            {
                char cmdline_content[512];
                if (fgets(cmdline_content, sizeof(cmdline_content), cmdline))
                {
                    cmdline_content[strcspn(cmdline_content, "\n")] = '\0';
                    if (strcmp(cmdline_content, PACKAGE) == 0)
                    {
                        fclose(cmdline);
                        closedir(dir);
                        return atoi(ent->d_name);
                    }
                }
                fclose(cmdline);
            }
        }
    }
    closedir(dir);
    return -1;
}
int attachTo(const char* PKG, const char* baselib)
{
    int PID = getPidFromPkgName(PKG);
    uintptr_t libBase = 0;
    char libPath[512];
    if (PID != -1)
    {
        cout << "[*] Package Name: " << PKG << endl << "[*] PID: " << PID << endl;
    }
    else
    {
        cout << "[*] Package not found: " << PKG << endl;
    }
    ptrace(PTRACE_ATTACH, PID, nullptr, nullptr);
    cout << "[*] Attached to process " << PKG << endl;
    waitpid(PID, nullptr, 0);
    memset(libPath, 0, sizeof(libPath));
    snprintf(libPath, sizeof(libPath), "/proc/self/maps");
    FILE* mapsFile = fopen(libPath, "r");
    if (mapsFile)
    {
        char line[256];
        while (fgets(line, sizeof(line), mapsFile))
        {
            if (strstr(line, baselib))
            {
                char* dash = strchr(line, '-');
                if (dash)
                {
                    *dash = '\0';
                    libBase = strtoull(line, nullptr, 16);
                    break;
                }
            }
        }
        fclose(mapsFile);
    }
    return PID;
}
int detachFrom(int PID)
{
    ptrace(PTRACE_DETACH, PID, nullptr, nullptr);
    cout << "[*] Detached from process" << endl;
    return 0;
}
