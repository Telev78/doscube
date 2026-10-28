#include <dos.h>
#include <mem.h>
#include <conio.h>
#include <iostream.h>
#include <stdio.h>
#include <stdlib.h>
#include <graphics.h>
#include <math.h>
#include "cube.h"
#include "mouse.h"
#include "renderer.h"
#include "common.h"
#include "control.h"

// Tableau de 64 000 octets alloué en mémoire lointaine (far) pour éviter le dépassement de segment
unsigned char far backBuffer[320UL * 200UL];

// Taille de la pile à 16ko
extern unsigned _stklen = 16384U;

static int savedMode = 3;  // mode vidéo d'origine, à restaurer en sortant
static int modeActive = 0; // 1 une fois le mode 13h activé
static volatile int quitRequested = 0;

int getVideoMode()
{
    _AH = 0x0F;
    geninterrupt(0x10);
    return _AL & 0x7F;
}

// 1 si une carte VGA (ou MCGA) est présente
int detectVGA()
{
    union REGS r;
    r.x.ax = 0x1A00;
    r.x.bx = 0;
    int86(0x10, &r, &r);
    if (r.h.al != 0x1A)
        return 0; // fonction absente : EGA, CGA, MDA...
    return (r.h.bl == 0x07 || r.h.bl == 0x08 ||
            (r.h.bl >= 0x0A && r.h.bl <= 0x0C));
}

// Passe en mode 13h et vérifie que le BIOS l'a bien accepté
int initMode13h()
{
    savedMode = getVideoMode();
    _AX = 0x0013;
    geninterrupt(0x10);
    modeActive = (getVideoMode() == 0x13);
    return modeActive;
}

// Restaure le mode d'origine (sans effet si le mode 13h n'a jamais été activé)
void closeMode13h()
{
    if (modeActive)
    {
        modeActive = 0;
        _AX = savedMode; // AH = 0 (set mode), AL = ancien mode
        geninterrupt(0x10);
    }
}

// Ctrl+Break : on demande simplement la sortie propre de la boucle
int ctrlBreakHandler()
{
    quitRequested = 1;
    return 1; // 1 = continuer le programme
}

void waitVSync()
{
    while (inportb(0x3DA) & 0x08)
        ;
    while (!(inportb(0x3DA) & 0x08))
        ;
}

// Affiche une chaîne de caractères à l'écran en Mode 13h via le BIOS
void printText(int col, int row, const char *str, unsigned char color)
{
    while (*str)
    {
        // 1. Positionner le curseur (INT 10h, Fonction 0x02)
        _AH = 0x02;
        _BH = 0;   // Page vidéo 0
        _DH = row; // Ligne (0 à 24)
        _DL = col; // Colonne (0 à 39)
        geninterrupt(0x10);

        // 2. Afficher le caractère en mode télétype (INT 10h, Fonction 0x0E)
        _AH = 0x0E;
        _AL = *str;
        _BH = 0;
        _BL = color; // Indice de couleur dans la palette 256 couleurs (ex: 15 = blanc)
        geninterrupt(0x10);

        col++;
        str++;
    }
}

int main()
{
    RubikCube myCube;
    initCube(myCube);

    // 1. Basculement en Mode 13h (320x200, 256 couleurs)
    if (!detectVGA())
    {
        printf("\n%s\n", STR_ERR_NO_VGA);
        return 1;
    }

    if (!initMode13h())
    {
        printf("\n%s\n", STR_ERR_MODE13);
        return 1;
    }
    atexit(closeMode13h); // restauration garantie, quelle que soit la sortie
    ctrlbrk(ctrlBreakHandler);
    initRenderer();

    // 2. Initialisation de la souris
    int numButtons = initMouse();
    if (numButtons == 0)
    {
        closeMode13h();
        printf("%s\n", STR_ERR_NO_MOUSE);
        return 1;
    }
    if (numButtons != -1 && numButtons < 2)
    {
        closeMode13h();
        printf("\n%s\n", STR_ERR_MOUSE_BUTTONS);
        return 1;
    }

    int mouseX, mouseY, leftBtn;
    char moveText[32];

    LayerAnim anim = {0, 0, 0, 0, 0, 12};
    int viewDrag = 0, faceDrag = 0, moveDone = 1;
    int owner, fIdx, startX = 0, startY = 0, prevX = 0, prevY = 0;
    int px, py, left, right, dx, dy, axis, layer, sign;
    int lastPx = -1, lastPy = -1;
    int viewChanged = 1; // 1 = forcer le premier affichage
    int redraw;

    while (!quitRequested)
    {
        getMouseStatus(mouseX, mouseY, left, right);
        px = mouseX / 2;
        py = mouseY;

        if (kbhit())
        {
            char key = getch();
            if (key == 27)
            {
                quitRequested = 1;
            }
        }
        // --- Clic droit : rotation de la vue ---
        if (right)
        {
            if (viewDrag && (px != prevX || py != prevY))
            {
                rotateView(DRAG_X_SIGN * (px - prevX), DRAG_Y_SIGN * (py - prevY));
                viewChanged = 1;
            }
            viewDrag = 1;
            prevX = px;
            prevY = py;
        }
        else
            viewDrag = 0;

        // --- Clic gauche : rotation d'une face ---
        if (left && !right)
        {
            if (!faceDrag)
            {
                faceDrag = 1;
                startX = px;
                startY = py;
                moveDone = anim.active || !pickFace(px, py, owner, fIdx);
            }
            else if (!moveDone)
            {
                dx = px - startX;
                dy = py - startY;
                if (abs(dx) + abs(dy) >= 8)
                {
                    if (computeMove(myCube, owner, fIdx, dx, dy, axis, layer, sign))
                    {
                        anim.active = 1;
                        anim.step = 0;
                        anim.axis = axis;
                        anim.layer = layer;
                        anim.sign = sign;
                    }
                    moveDone = 1;
                }
            }
        }
        else
            faceDrag = 0;

        // --- Avancement de l'animation ---
        if (anim.active && ++anim.step >= anim.steps)
        {
            rotateLayer(myCube, anim.axis, anim.layer, anim.sign);
            anim.active = 0;
            viewChanged = 1; // afficher l'état final
        }

        redraw = (anim.active || viewChanged);

        if (redraw)
        {
            // 1. Dessin dans le back-buffer (zone cube + zone texte seulement)
            clearRect(VIEW_X0, VIEW_Y0, VIEW_X1, VIEW_Y1);
            drawCube(myCube, &anim);

            clearRect(HUD_X0, HUD_Y0, HUD_X1, HUD_Y1);
            sprintf(moveText, STR_MOVES, myCube.moveCount);
            drawText(4, 2, moveText, 15);

            // 2. Copie vers l'écran, synchronisée sur le retour vertical
            waitVSync();
            blitRect(VIEW_X0, VIEW_Y0, VIEW_X1, VIEW_Y1);
            blitRect(HUD_X0, HUD_Y0, HUD_X1, HUD_Y1);
            viewChanged = 0;
        }

        // 3. Curseur : on restaure l'ancien emplacement depuis le back-buffer,
        //    puis on dessine le nouveau directement en VRAM
        if (redraw || px != lastPx || py != lastPy)
        {
            blitRect(lastPx - 3, lastPy - 3, lastPx + 3, lastPy + 3);
            drawCursorVRAM(px, py);
            lastPx = px;
            lastPy = py;
        }
    }

    return 0;
}
