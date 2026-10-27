#include <dos.h>
#include <mem.h> // Pour _fmemcpy / _fmemset
#include <conio.h>
#include <iostream.h>
#include <stdio.h>
#include "cube.h"
#include "mouse.h"
#include "renderer.h"
#include <stdlib.h>
#include <graphics.h>
#include <math.h>
#include "control.h"

double normalizeAngle(double angle);

// Tableau de 64 000 octets alloué en mémoire lointaine (far) pour éviter le dépassement de segment
unsigned char far backBuffer[320UL * 200UL];
extern unsigned _stklen = 16384U;
// Basculer en Mode 13h (320x200, 256 couleurs) via interruption BIOS 0x10
void initMode13h()
{
    _AX = 0x0013;
    geninterrupt(0x10);
}

// Restaurer le mode texte standard du DOS
void closeMode13h()
{
    _AX = 0x0003;
    geninterrupt(0x10);
}

// Copier le Back-buffer vers la mémoire vidéo (0xA0000)
void blitBuffer()
{
    // Synchronisation VSync pour éliminer tout artefact
    while (inportb(0x3DA) & 0x08)
        ;
    while (!(inportb(0x3DA) & 0x08))
        ;

    // Copie globale des 64 000 octets d'un seul bloc (64000 < 65535, tient dans le segment 64Ko)
    _fmemcpy((unsigned char far *)0xA0000000L, backBuffer, 64000);
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
    initMode13h();

    // 2. Initialisation de la souris
    int numButtons = initMouse();
    if (numButtons == 0)
    {
        closeMode13h();
        cout << "Erreur : Souris non detectee !" << endl;
        return 1;
    }

    double angleX = 30.0;
    double angleY = -45.0;

    int mouseX, mouseY, leftBtn;
    int prevMouseX = -1, prevMouseY = -1;
    int dragging = 0;
    char moveText[32];

    LayerAnim anim = {0, 0, 0, 0, 0, 12};
    int viewDrag = 0, faceDrag = 0, moveDone = 1;
    int owner, fIdx, startX = 0, startY = 0, prevX = 0, prevY = 0;
    int px, py, left, right, dx, dy, axis, layer, sign;

    while (1)
    {
        getMouseStatus(mouseX, mouseY, left, right);
        px = mouseX / 2;
        py = mouseY;

        if (kbhit())
        {
            char key = getch();
            if (key == 27)
            {
                break;
            }
        }
        // --- Clic droit : rotation de la vue ---
        if (right)
        {
            if (viewDrag)
            {
                angleY = normalizeAngle(angleY + (px - prevX) * 1.0);
                angleX = normalizeAngle(angleX + (py - prevY) * 1.0);
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
                    if (computeMove(myCube, owner, fIdx, dx, dy, angleX, angleY, axis, layer, sign))
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
        }

        _fmemset(backBuffer, 0, 64000);
        drawCube(myCube, angleX, angleY, &anim);
        sprintf(moveText, "Coups : %u", myCube.moveCount);
        drawText(4, 4, moveText, 15);
        drawCursor(px, py);
        blitBuffer();
    }

    closeMode13h();
    return 0;
}

// Ramène tout angle en degrés dans l'intervalle [0, 360[
double normalizeAngle(double angle)
{
    angle = fmod(angle, 360.0);
    if (angle < 0.0)
    {
        angle += 360.0;
    }
    return angle;
}