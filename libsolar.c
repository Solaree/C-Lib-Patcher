#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>

int main() {
    const char* lib = "/data/user/0/com.supercell.brawlstars/lib/libg.so";
    struct stat libstat;

    printf("[*] Starting patches...\n\n");

    int fd = open(lib, O_RDWR);
    
    if (fd == -1) {
        perror("[*] Failed to open library file\n");
        return 1;
    }

    fstat(fd, &libstat);

    printf("[*] Lib size: %lld bytes\n", (long long)libstat.st_size);
    printf("[*] Lib permissions: %o\n", libstat.st_mode & 0777);

    void* mappaddr = mmap(NULL, libstat.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    if (mappaddr == MAP_FAILED) {
        perror("[*] Failed to mmap library file\n");
        close(fd);
        return 1;
    }

    uintptr_t offsets[] = {
        0x33CC04, 0x66DCEC, 0x68DE6C, 0x493304, 0x68A718, 0x7D8858, 0x39AD0C, 0x8339F4, 0xB95D8,
        0x2174A4, 0x6708A0, 0x4672EC, 0x670808
    };

    unsigned char patchBytes[][4] = {
        {0x1D, 0x00, 0x00, 0xEB}, {0xEB, 0x00, 0x00, 0xEB}, {0x08, 0x04, 0x00, 0xEB},
        {0xE7, 0x02, 0x00, 0xEB}, {0xB6, 0x02, 0x00, 0xEB}, {0xF5, 0x00, 0x00, 0xEB},
        {0x12, 0x06, 0x00, 0xEB}, {0x1E, 0xFF, 0x2F, 0xE1}, {0x1E, 0xFF, 0x2F, 0xE1},
        {0x00, 0x00, 0x50, 0xE1}, {0x05, 0x00, 0xA0, 0xE3}, {0x00, 0x40, 0xA0, 0xE3},
        {0x02, 0x80, 0xA0, 0xE1}
    };

    for (int i = 0; i < sizeof(offsets) / sizeof(offsets[0]); i++) {
        uintptr_t start_address = (uintptr_t)mappaddr + offsets[i];
        uintptr_t page_start = start_address & ~(4095UL);

        size_t size = (start_address - page_start) + 4;

        if (mprotect((void*)page_start, size, PROT_READ | PROT_WRITE) == -1) {
            perror("[*] Failed to protect page for 'rw'\n");
            close(fd);
            munmap(mappaddr, libstat.st_size);
            return 1;
        }

        for (int j = 0; j < 4; j++) {
            *(unsigned char*)(start_address + j) = patchBytes[i][j];
        }

        if (mprotect((void*)page_start, size, PROT_READ) == -1) {
            perror("[*] Failed to protect page for 'r'\n");
            close(fd);
            munmap(mappaddr, libstat.st_size);
            return 1;
        }
    }

    printf("[*] Patched Arxan and crypto\n");

    msync(mappaddr, libstat.st_size, MS_SYNC);

    munmap(mappaddr, libstat.st_size);
    close(fd);

    printf("[*] Successfully closed file descriptor\n");
    return 0;
}