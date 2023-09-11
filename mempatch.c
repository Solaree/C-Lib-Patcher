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

unsigned long armHex(const char* hexString) {
    char* cleanHexString = malloc(strlen(hexString) + 1);
    int cleanIndex = 0;

    for (int i = 0; hexString[i] != '\0'; i++) {
        if (hexString[i] != ' ') {
            cleanHexString[cleanIndex++] = hexString[i];
        }
    }
    cleanHexString[cleanIndex] = '\0';
    INSTRUCTION_SET hexValue = strtoul(cleanHexString, NULL, 16);

    free(cleanHexString);
    return hexValue;
}

INSTRUCTION_SET putRet() {
    return armHex("1E FF 2F E1");
} /* NOP */
INSTRUCTION_SET putNop() {
    return armHex("00 F0 20 E3");
} /* BX LR */
INSTRUCTION_SET putValZero() {
    return armHex("00 00 A0 E3");
} /* MOV R0, #0 */
INSTRUCTION_SET putValOne() {
    return armHex("01 00 A0 E3");
} /* MOV R0, #1 */

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

int mempatch(const char* PACKAGE, const char* LIBNAME, uintptr_t OFFSETS[], unsigned long PATCHBYTES[], size_t numPatches) {
    // const char* PACKAGE = "com.supercell.brawlstars";
    int PID = getPidFromPackageName(PACKAGE);

    uintptr_t libBase = 0;

    char libPath[512];

    /*uintptr_t OFFSETS[] = {
        0x33CC04, 0x66DCEC, 0x68DE6C,
        0x493304, 0x68A718, 0x7D8858,
        0x39AD0C, 0x8339F4, 0xB95D8,
        0x2174A4, 0x6708A0, 0x4672EC,
        0x670808
    };

    unsigned long PATCHBYTES[] = {
        0xEB00001D, 0xEB0000EB, 0xEB000408,
        0xEB0002E7, 0xEB0002B6, 0xEB0000F5,
        0xEB000612, putRet(), putRet(),
        0xE1500000, 0xE3A00005, 0xE3A04000,
        0xE1A08002
    };*/

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
            if (strstr(line, LIBNAME)) {
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

    // size_t numPatches = sizeof(OFFSETS) / sizeof(OFFSETS[0]);

    for (size_t i = 0; i < numPatches; i++) {
        uintptr_t patchAddress = libBase + OFFSETS[i];

        ptrace(PTRACE_POKETEXT, PID, (void*)patchAddress, (void*)PATCHBYTES[i]);
    }
    printf("[*] Successfully patched\n");

    ptrace(PTRACE_DETACH, PID, NULL, NULL);
    printf("[*] Detached from process\n");
    return 0;
}