#include <dos.h>
#include "cpu.h"

/* Test classique des bits 12..15 du registre d'etat (flags) :
     8086/8088/80186 : ces bits valent toujours 1
     80286 (mode reel) : ces bits valent toujours 0
     80386+ (mode reel) : les bits 12..14 (IOPL, NT) sont modifiables, le bit 15 reste a 0
   Sous un gestionnaire de memoire (EMM386, QEMM, Windows) le processeur est en mode
   virtuel 8086 : le test peut alors donner "286" a tort. L'utilisateur peut dans ce
   cas forcer la vitesse avec /C:FAST. */
int detectCpuClass()
{
    unsigned orig, v;

    orig = _FLAGS;

    _FLAGS = orig & 0x0FFF; /* essaie de mettre les bits 12..15 a 0 */
    v = _FLAGS;
    if ((v & 0xF000U) == 0xF000U)
    {
        _FLAGS = orig;
        return CPU_CLASS_8086;
    }

    _FLAGS = orig | 0xF000U; /* essaie de mettre les bits 12..15 a 1 */
    v = _FLAGS;
    _FLAGS = orig;
    if ((v & 0xF000U) == 0)
        return CPU_CLASS_286;

    return CPU_CLASS_386;
}

const char *cpuClassName(int cpuClass)
{
    switch (cpuClass)
    {
    case CPU_CLASS_8086:
        return "8086/186";
    case CPU_CLASS_286:
        return "286";
    default:
        return "386+";
    }
}