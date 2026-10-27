#include <dos.h>
#include "mouse.h"

// 1. Initialisation de la souris (AX = 0x0000)
int initMouse()
{
    REGS inRegs, outRegs;
    inRegs.x.ax = 0x0000; // Fonction 0 : Initialisation du driver de souris
    int86(0x33, &inRegs, &outRegs);

    // Si AX == 0xFFFF, la souris est présente. Retourne le nombre de boutons, sinon 0.
    return (outRegs.x.ax == 0xFFFF) ? outRegs.x.bx : 0;
}

// 2. Afficher le curseur (AX = 0x0001)
void showMouseCursor()
{
    REGS inRegs, outRegs;
    inRegs.x.ax = 0x0001;
    int86(0x33, &inRegs, &outRegs);
}

// 3. Masquer le curseur (AX = 0x0002)
void hideMouseCursor()
{
    REGS inRegs, outRegs;
    inRegs.x.ax = 0x0002;
    int86(0x33, &inRegs, &outRegs);
}

// 4. Obtenir la position (X, Y) et l'état du clic (AX = 0x0003)
void getMouseStatus(int &x, int &y, int &leftButton, int &rightButton)
{
    REGS inRegs, outRegs;
    inRegs.x.ax = 0x0003;
    int86(0x33, &inRegs, &outRegs);

    x = outRegs.x.cx;                 // Coordonnée horizontale X
    y = outRegs.x.dx;                 // Coordonnée verticale Y
    leftButton = outRegs.x.bx & 0x01; // Bit 0 de BX = état du bouton gauche (1 = cliqué, 0 = relâché)
    rightButton = (outRegs.x.bx >> 1) & 0x01;
}