#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

int main() {
    const char* lib = "/data/data/com.supercell.brawlstars/lib/libg.so";
    struct stat libstat;

    printf("[*] Starting patches...\n\n");

    int fd = open(lib, O_RDWR);

    fstat(fd, &libstat);

    printf("[*] Lib size: %lld bytes\n", (long long)libstat.st_size);
    printf("[*] Lib permissions: %o\n", libstat.st_mode & 0777);

    void* mappaddr = mmap(NULL, libstat.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    if (mappaddr == MAP_FAILED) {
        perror("[*] Failed to mmap library file\n");
        close(fd);
        return 1;
    }

    uintptr_t offsets[] = { 0x000000 };

    unsigned char patchBytes[][4] = { 0x00, 0x00, 0x00, 0x00 };

    for (int i = 0; i < sizeof(offsets) / sizeof(offsets[0]); i++) {
        uintptr_t start_address = (uintptr_t)mappaddr + offsets[i];
        uintptr_t page_start = start_address & ~(4095UL);

        size_t size = (start_address - page_start) + 4;

        mprotect((void*)page_start, size, PROT_READ | PROT_WRITE);

        for (int j = 0; j < 4; j++) {
            *(unsigned char*)(start_address + j) = patchBytes[i][j];
        }
        mprotect((void*)page_start, size, PROT_READ | PROT_EXEC);
    }

    printf("[*] Patched\n");

    msync(mappaddr, libstat.st_size, MS_SYNC);

    munmap(mappaddr, libstat.st_size);
    close(fd);

    printf("[*] Successfully closed file descriptor\n");
    return 0;
}
