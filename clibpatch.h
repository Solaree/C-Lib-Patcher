#ifndef CLIBPATCH_H
#define CLIBPATCH_H

#define INSTRUCTION_SET unsigned long /* Bytecode instruction patches for arm assembly */

extern unsigned long armHex(const char* hexString);

extern INSTRUCTION_SET putRet();
extern INSTRUCTION_SET putNop();
extern INSTRUCTION_SET putValZero();
extern INSTRUCTION_SET putValOne();

#endif