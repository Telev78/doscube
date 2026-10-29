#include <dos.h>
#include <mem.h>
#include <conio.h>
#include <iostream.h>
#include <stdio.h>
#include <stdlib.h>
#include <graphics.h>
#include <math.h>
#include <bios.h>
#include <string.h>
#include "cube.h"
#include "mouse.h"
#include "renderer.h"
#include "lang.h"
#include "control.h"
#include "timer.h"
#include "history.h"

#define HISTORY_STEPS 6
#define MOVE_MANUAL 0
#define MOVE_UNDO 1
#define MOVE_REDO 2
#define MOVE_SHUFFLE 3
#define ANIM_STEPS 12
#define SHUFFLE_STEPS 4 // animation rapide pendant le mélange

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

int main(int argc, char *argv[])
{
    initLanguage(argc, argv);

    RubikCube myCube;
    initCube(myCube);

    // 1. Basculement en Mode 13h (320x200, 256 couleurs)
    if (!detectVGA())
    {
        printf("\n%s\n", T(S_ERR_NO_VGA));
        return 1;
    }

    if (!initMode13h())
    {
        printf("\n%s\n", T(S_ERR_MODE13));
        return 1;
    }
    atexit(closeMode13h); // restauration garantie, quelle que soit la sortie
    ctrlbrk(ctrlBreakHandler);
    initRenderer();

    timerInit();
    atexit(timerRestore);

    // 2. Initialisation de la souris
    int numButtons = initMouse();
    if (numButtons == 0)
    {
        closeMode13h();
        printf("%s\n", T(S_ERR_NO_MOUSE));
        return 1;
    }
    if (numButtons != -1 && numButtons < 2)
    {
        closeMode13h();
        printf("\n%s\n", T(S_ERR_MOUSE_BUTTONS));
        return 1;
    }

    LayerAnim anim = {0, 0, 0, 0, 0, ANIM_STEPS};
    int mouseX, mouseY, left, right, px, py, dx, dy;
    int viewDrag = 0, faceDrag = 0, moveDone = 1, btnDown = 0;
    int owner = 0, fIdx = 0, startX = 0, startY = 0, prevX = 0, prevY = 0;
    int axis, layer, sign;
    int lastPx = -1, lastPy = -1, viewChanged = 1, redraw;
    int shuffling = 0, shuffleLeft = 0, lastAxis = -1, lastLayer = 0;
    int wantShuffle = 0, gameState = 0, busy, key; // 0 libre, 1 en jeu, 2 résolu
    unsigned finalMoves = 0;
    unsigned char textColor;
    char moveText[40];
    char timeText[16], lastTimeText[16];
    int chronoUpdated;
    lastTimeText[0] = 0;
    int moveKind = MOVE_MANUAL;
    int wantUndo = 0, wantRedo = 0, canUndo, canRedo;
    int hAxis, hLayer, hSign;
    unsigned savedCount;

    while (!quitRequested)
    {
        getMouseStatus(mouseX, mouseY, left, right);
        px = mouseX / 2;
        py = mouseY;
        busy = (anim.active || shuffling);

        canUndo = !anim.active && !shuffling && gameState != 2 && histCanUndo();
        canRedo = !anim.active && !shuffling && gameState != 2 && histCanRedo();

        // --- Clavier ---
        if (kbhit())
        {
            key = getch();
            if (key == 0 || key == 224) // touche étendue : le code est dans l'octet suivant
            {
                key = getch();
                if (key == 75)
                    wantUndo = 1; // flèche gauche
                else if (key == 77)
                    wantRedo = 1; // flèche droite
            }
            else if (key == 27)
                quitRequested = 1;
            else if (key == 'm' || key == 'M')
                wantShuffle = 1;
            else if (key == 'u' || key == 'U')
                wantUndo = 1;
            else if (key == 'r' || key == 'R')
                wantRedo = 1;
            else if (key == 'l' || key == 'L')
            {
                nextLanguage();
                viewChanged = 1;
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

        // --- Clic gauche : bouton ou rotation d'une face ---
        if (left && !right)
        {
            if (!faceDrag)
            {
                faceDrag = 1;
                startX = px;
                startY = py;
                btnDown = 0;
                if (IN_BUTTON(px, py))
                    btnDown = 1;
                else if (IN_UNDO(px, py) && canUndo)
                    btnDown = 2;
                else if (IN_REDO(px, py) && canRedo)
                    btnDown = 3;

                if (btnDown)
                {
                    moveDone = 1;
                    viewChanged = 1;
                }
                else if (IN_UNDO(px, py) || IN_REDO(px, py))
                    moveDone = 1; // bouton grisé : on ignore le clic
                else
                    moveDone = busy || !pickFace(px, py, owner, fIdx);
            }
            else if (!moveDone)
            {
                dx = px - startX;
                dy = py - startY;
                if (abs(dx) + abs(dy) >= 8)
                {
                    if (!busy && computeMove(myCube, owner, fIdx, dx, dy, axis, layer, sign))
                    {
                        if (gameState == 1 && !timerRunning())
                            timerStart();
                        anim.active = 1;
                        anim.step = 0;
                        anim.steps = ANIM_STEPS;
                        anim.axis = axis;
                        anim.layer = layer;
                        anim.sign = sign;
                        moveKind = MOVE_MANUAL;
                    }
                    moveDone = 1;
                }
            }
        }
        else if (!left)
        {
            if (btnDown)
            {
                if (btnDown == 1 && IN_BUTTON(px, py))
                    wantShuffle = 1;
                else if (btnDown == 2 && IN_UNDO(px, py))
                    wantUndo = 1;
                else if (btnDown == 3 && IN_REDO(px, py))
                    wantRedo = 1;
                btnDown = 0;
                viewChanged = 1;
            }
            faceDrag = 0;
        }

        // --- Lancement d'un mélange ---
        if (wantShuffle && !busy)
        {
            srand((unsigned)biostime(0, 0));
            shuffleLeft = 30 + rand() % 11; // 30 à 40 rotations
            shuffling = 1;
            gameState = 0;
            lastAxis = -1;
            lastLayer = 0;
            viewChanged = 1;
            histClear();
        }
        wantShuffle = 0;

        // --- Prochain coup du mélange ---
        if (shuffling && shuffleLeft > 0 && !anim.active)
        {
            randomMove(lastAxis, lastLayer, axis, layer, sign);
            anim.active = 1;
            anim.step = 0;
            anim.steps = SHUFFLE_STEPS;
            anim.axis = axis;
            anim.layer = layer;
            anim.sign = sign;
            lastAxis = axis;
            lastLayer = layer;
            moveKind = MOVE_SHUFFLE;
            shuffleLeft--;
        }

        // --- Undo & Redo ---
        if (wantUndo && !anim.active && !shuffling && gameState != 2 &&
            histPeekUndo(hAxis, hLayer, hSign))
        {
            anim.active = 1;
            anim.step = 0;
            anim.steps = HISTORY_STEPS;
            anim.axis = hAxis;
            anim.layer = hLayer;
            anim.sign = hSign;
            moveKind = MOVE_UNDO;
        }
        else if (wantRedo && !anim.active && !shuffling && gameState != 2 &&
                 histPeekRedo(hAxis, hLayer, hSign))
        {
            anim.active = 1;
            anim.step = 0;
            anim.steps = HISTORY_STEPS;
            anim.axis = hAxis;
            anim.layer = hLayer;
            anim.sign = hSign;
            moveKind = MOVE_REDO;
        }
        wantUndo = 0;
        wantRedo = 0;

        // --- Avancement de l'animation ---
        if (anim.active && ++anim.step >= anim.steps)
        {
            savedCount = myCube.moveCount;
            rotateLayer(myCube, anim.axis, anim.layer, anim.sign);
            if (moveKind != MOVE_MANUAL)
                myCube.moveCount = savedCount; // annuler/refaire/mélange ne comptent pas
            anim.active = 0;
            viewChanged = 1;

            if (moveKind == MOVE_UNDO)
                histCommitUndo();
            else if (moveKind == MOVE_REDO)
                histCommitRedo();
            else if (moveKind == MOVE_MANUAL)
                histPush(anim.axis, anim.layer, anim.sign);

            if (shuffling)
            {
                if (shuffleLeft == 0)
                {
                    if (isSolved(myCube))
                        shuffleLeft = 1; // très improbable : on ajoute un coup
                    else
                    {
                        shuffling = 0;
                        myCube.moveCount = 0; // le compteur ne compte que le joueur
                        myCube.isMixed = 1;
                        gameState = 1;
                        timerReset();
                    }
                }
            }
            else if (moveKind == MOVE_MANUAL && gameState == 1 && isSolved(myCube))
            {
                gameState = 2;
                finalMoves = myCube.moveCount;
                timerStop();
                myCube.isMixed = 0;
            }
        }

        // --- Affichage ---
        redraw = (anim.active || viewChanged);
        chronoUpdated = 0;

        if (redraw)
        {
            clearRect(VIEW_X0, VIEW_Y0, VIEW_X1, VIEW_Y1);
            drawCube(myCube, &anim);

            textColor = 15;
            if (shuffling)
                strcpy(moveText, T(S_SHUFFLING));
            else if (gameState == 2)
            {
                sprintf(moveText, T(S_SOLVED), finalMoves);
                textColor = 14;
            }
            else
                sprintf(moveText, T(S_MOVES), myCube.moveCount);

            clearRect(HUD_X0, HUD_Y0, HUD_X1, HUD_Y1);
            drawText(4, 2, moveText, textColor);
            formatTime(timeText, timerElapsedMs());
            drawText(4, 14, timeText, textColor);
            drawButton(BTN_X0, BTN_Y0, BTN_X1, BTN_Y1, T(S_BTN_SHUFFLE), btnDown);

            canUndo = !anim.active && !shuffling && gameState != 2 && histCanUndo();
            canRedo = !anim.active && !shuffling && gameState != 2 && histCanRedo();

            drawButton(BTN_X0, BTN_Y0, BTN_X1, BTN_Y1, T(S_BTN_SHUFFLE), btnDown == 1);
            drawButtonEx(UNDO_X0, UNDO_Y0, UNDO_X1, UNDO_Y1, "\033", btnDown == 2, canUndo);
            drawButtonEx(REDO_X0, REDO_Y0, REDO_X1, REDO_Y1, "\032", btnDown == 3, canRedo);

            waitVSync();
            blitRectCursor(VIEW_X0, VIEW_Y0, VIEW_X1, VIEW_Y1, px, py);
            blitRectCursor(HUD_X0, HUD_Y0, HUD_X1, HUD_Y1, px, py);
            blitRectCursor(BTN_X0, BTN_Y0, BTN_X1, BTN_Y1, px, py);
            blitRectCursor(NAV_X0, NAV_Y0, NAV_X1, NAV_Y1, px, py);
            strcpy(lastTimeText, timeText);
            viewChanged = 0;
        }
        else if (timerRunning())
        {
            formatTime(timeText, timerElapsedMs());
            if (strcmp(timeText, lastTimeText) != 0)
            {
                clearRect(CHRONO_X0, CHRONO_Y0, CHRONO_X1, CHRONO_Y1);
                drawText(4, 14, timeText, 15);
                blitRectCursor(CHRONO_X0, CHRONO_Y0, CHRONO_X1, CHRONO_Y1, px, py);
                strcpy(lastTimeText, timeText);
                chronoUpdated = 1;
            }
        }
        // --- Curseur : seulement s'il a bougé ---
        if (px != lastPx || py != lastPy)
        {
            blitRect(lastPx, lastPy, lastPx + CURSOR_W - 1, lastPy + CURSOR_H - 1); // efface l'ancien
            drawCursorVRAM(px, py);                                                 // dessine le nouveau
            lastPx = px;
            lastPy = py;
        }
    }

    return 0;
}
