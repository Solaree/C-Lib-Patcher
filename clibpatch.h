#ifndef ASM_H
#define ASM_H

#define INSTRUCTION_SET unsigned long /* Bytecode instruction patches for arm assembly */

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

#endif