#ifndef CPU_H
#define CPU_H

/* Classes de processeur reconnues */
#define CPU_CLASS_8086 0 /* 8086, 8088, 80186, 80188 */
#define CPU_CLASS_286 1
#define CPU_CLASS_386 2 /* 80386 et suivants         */

/* Detecte la classe du processeur (test des bits 12 a 15 du registre d'etat) */
int detectCpuClass();

/* Nom affichable : "8086/186", "286" ou "386+" */
const char *cpuClassName(int cpuClass);

#endif