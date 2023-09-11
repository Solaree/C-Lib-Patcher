import os; from ctypes import *

libpath = os.path.join(os.path.expanduser("~"), "Desktop", "C-Lib-Patcher", "mempatch.so")
mempatch = CDLL(libpath) # Your full path to shared library

# Example killing arxan and crypto for v36.218

PACKAGE = "com.supercell.brawlstars"
LIB = "libg.so"
OFFSETS = [
    0x33CC04, 0x66DCEC, 0x68DE6C,
    0x493304, 0x68A718, 0x7D8858,
    0x39AD0C, 0x8339F4, 0xB95D8,
    0x2174A4, 0x6708A0, 0x4672EC,
    0x670808
]
PATCHBYTES = [
    0xEB00001D, 0xEB0000EB, 0xEB000408,
    0xEB0002E7, 0xEB0002B6, 0xEB0000F5,
    0xEB000612, mempatch.putRet(), mempatch.putRet(),
    0xE1500000, 0xE3A00005, 0xE3A04000,
    0xE1A08002
]

numPatches = len(OFFSETS)

OFFSETS = (c_void_p * numPatches)(*OFFSETS)
PATCHBYTES = (c_ulong * numPatches)(*PATCHBYTES)

mempatch.mempatch(
    PACKAGE.encode('utf-8'),  # Encode the package name as bytes
    LIB.encode('utf-8'),      # Encode the library name as bytes
    OFFSETS,                  # Pass the C array of offsets
    PATCHBYTES,               # Pass the C array of patch bytes
    c_size_t(numPatches)      # Pass the number of patches as a C size_t
)