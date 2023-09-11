#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <sys/uio.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/user.h>
#include <sys/types.h>
#include <sys/ptrace.h>
#include <sys/syscall.h>
#include "clibpatch.h"

int getPidFromPackageName(const char* PACKAGE) {
    char path[512];

    DIR* dir = opendir("/proc");
    struct dirent* ent = readdir(dir);

    while ((ent = readdir(dir)) != NULL) {
        if (isdigit(*ent->d_name)) {
            snprintf(path, sizeof(path), "/proc/%s/cmdline", ent->d_name);
            FILE* cmdline = fopen(path, "r");

            if (cmdline) {
                char cmdline_content[512];
                if (fgets(cmdline_content, sizeof(cmdline_content), cmdline)) {
                    cmdline_content[strcspn(cmdline_content, "\n")] = '\0';

                    if (strcmp(cmdline_content, PACKAGE) == 0) {
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

int main() {
    const char* PACKAGE = "com.supercell.brawlstars";
    int PID = getPidFromPackageName(PACKAGE);

    uintptr_t libBase = 0;

    char libName[] = "libg.so";
    char libPath[512];

    uintptr_t OFFSETS[] = {
        0x000000
    };

    unsigned long PATCHBYTES[] = {
        putRet();
    };

    if (PID != -1) {
        printf("[*] Package Name: %s\n[*] PID: %d\n", PACKAGE, PID);
    } else {
        printf("[*] Package not found: %s\n", PACKAGE);
    }

    ptrace(PTRACE_ATTACH, PID, NULL, NULL);
    printf("[*] Attached to process\n");

    waitpid(PID, NULL, 0);
    memset(libPath, 0, sizeof(libPath));

    snprintf(libPath, sizeof(libPath), "/proc/%d/maps", PID);
    FILE* mapsFile = fopen(libPath, "r");

    if (mapsFile) {
        char line[256];

        while (fgets(line, sizeof(line), mapsFile)) {
            if (strstr(line, libName)) {
                char* dash = strchr(line, '-');
                if (dash) {
                    *dash = '\0';
                    libBase = strtoull(line, NULL, 16);
                    break;
                }
            }
        }
        fclose(mapsFile);
    }

    for (size_t i = 0; i < sizeof(OFFSETS) / sizeof(OFFSETS[0]); i++) {
        uintptr_t patchAddress = libBase + OFFSETS[i];

        ptrace(PTRACE_POKETEXT, PID, (void*)patchAddress, (void*)PATCHBYTES[i]);
    }
    printf("[*] Successfully patched\n");

    ptrace(PTRACE_DETACH, PID, NULL, NULL);
    printf("[*] Detached from process\n");
    return 0;
}
