#include <dos.h>
#include "mouse.h"

// 1. Initialisation de la souris (AX = 0x0000)
int initMouse()
{
    REGS inRegs, outRegs;
    int buttons;

    inRegs.x.ax = 0x0000; // Fonction 0 : Initialisation du driver de souris
    int86(0x33, &inRegs, &outRegs);

    // Si AX == 0xFFFF, la souris est présente. Sinon retourne 0.
    if (outRegs.x.ax != 0xFFFF)
        return 0;
    buttons = outRegs.x.bx;

    // Plage explicite : X 0..639 (2 unités par pixel), Y 0..199
    inRegs.x.ax = 0x0007;
    inRegs.x.cx = 0;
    inRegs.x.dx = 639;
    int86(0x33, &inRegs, &outRegs);

    inRegs.x.ax = 0x0008;
    inRegs.x.cx = 0;
    inRegs.x.dx = 199;
    int86(0x33, &inRegs, &outRegs);

    return buttons;
}

// 2. Obtenir la position (X, Y) et l'état du clic (AX = 0x0003)
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