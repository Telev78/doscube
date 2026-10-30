#include <dos.h>
#include <stdio.h>
#include <stdlib.h>
#include <bios.h>
#include <string.h>
#include "cube.h"
#include "mouse.h"
#include "renderer.h"
#include "lang.h"
#include "control.h"
#include "timer.h"
#include "history.h"
#include "options.h"
#include "about.h"
#include "video.h"

#define MOVE_MANUAL 0
#define MOVE_UNDO 1
#define MOVE_REDO 2
#define MOVE_SHUFFLE 3

// Taille de la pile à 16ko
extern unsigned _stklen = 16384U;

static volatile int quitRequested = 0;

// Ctrl+Break : on demande simplement la sortie propre de la boucle
int ctrlBreakHandler()
{
    quitRequested = 1;
    return 1; // 1 = continuer le programme
}

int main(int argc, char *argv[])
{
    initLanguage(argc, argv);

    int status = parseOptions(argc, argv);
    if (status == OPT_HELP)
    {
        printHelp();
        return 0;
    }
    if (status == OPT_ERROR)
    {
        printf(T(S_ERR_BAD_OPTION), badOption());
        printf("\n%s\n", T(S_HINT_HELP));
        return 1;
    }

    resolveCpuMode();

    int stepsManual, stepsHistory, stepsShuffle;
    if (options.cpuMode == CPU_SLOW)
    {
        stepsManual = 6;
        stepsHistory = 3;
        stepsShuffle = 2;
    }
    else
    {
        stepsManual = 12;
        stepsHistory = 6;
        stepsShuffle = 4;
    }

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

    LayerAnim anim = {0, 0, 0, 0, 0, 1};
    Input in;
    int px, py;
    int axis, layer, sign;
    int lastPx = -1, lastPy = -1, viewChanged = 1, redraw;
    int shuffling = 0, shuffleLeft = 0, lastAxis = -1, lastLayer = 0;
    int gameState = 0, busy; // 0 libre, 1 en jeu, 2 résolu
    unsigned finalMoves = 0;
    unsigned char textColor;
    char moveText[40];
    char timeText[16], lastTimeText[16];
    int moveKind = MOVE_MANUAL;
    int canUndo, canRedo;
    int hAxis, hLayer, hSign;
    unsigned savedCount;
    int resumeTimer;

    lastTimeText[0] = 0;
    inputInit(in);

    while (!quitRequested)
    {
        busy = (anim.active || shuffling);
        canUndo = !anim.active && !shuffling && gameState != 2 && histCanUndo();
        canRedo = !anim.active && !shuffling && gameState != 2 && histCanRedo();

        // --- Entrées : souris et clavier (voir control.cpp) ---
        inputUpdate(in, myCube, busy, canUndo, canRedo);
        px = in.px;
        py = in.py;
        if (in.wantLanguage)
            nextLanguage(); // boucle entre les langues d'interface
        if (in.redraw || in.wantLanguage)
            viewChanged = 1;

        // --- Coup demandé par un glissement sur une face ---
        if (in.moveRequested)
        {
            if (gameState == 1 && !timerRunning())
                timerStart();
            anim.active = 1;
            anim.step = 0;
            anim.steps = stepsManual;
            anim.axis = in.moveAxis;
            anim.layer = in.moveLayer;
            anim.sign = in.moveSign;
            moveKind = MOVE_MANUAL;
        }

        // --- Lancement d'un mélange ---
        if (in.wantShuffle && !anim.active && !shuffling)
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

        // --- Prochain coup du mélange ---
        if (shuffling && shuffleLeft > 0 && !anim.active)
        {
            randomMove(lastAxis, lastLayer, axis, layer, sign);
            anim.active = 1;
            anim.step = 0;
            anim.steps = stepsShuffle;
            anim.axis = axis;
            anim.layer = layer;
            anim.sign = sign;
            lastAxis = axis;
            lastLayer = layer;
            moveKind = MOVE_SHUFFLE;
            shuffleLeft--;
        }

        // --- Undo & Redo ---
        if (in.wantUndo && !anim.active && !shuffling && gameState != 2 &&
            histPeekUndo(hAxis, hLayer, hSign))
        {
            anim.active = 1;
            anim.step = 0;
            anim.steps = stepsHistory;
            anim.axis = hAxis;
            anim.layer = hLayer;
            anim.sign = hSign;
            moveKind = MOVE_UNDO;
        }
        else if (in.wantRedo && !anim.active && !shuffling && gameState != 2 &&
                 histPeekRedo(hAxis, hLayer, hSign))
        {
            anim.active = 1;
            anim.step = 0;
            anim.steps = stepsHistory;
            anim.axis = hAxis;
            anim.layer = hLayer;
            anim.sign = hSign;
            moveKind = MOVE_REDO;
        }

        // --- About & Help ---
        if (in.wantHelp && !anim.active && !shuffling)
        {
            resumeTimer = timerRunning();
            if (resumeTimer)
                timerStop(); // pause du chrono
            showAbout();
            if (resumeTimer)
                timerResume();
            viewChanged = 1;
            lastPx = -1;
            lastPy = -1; // force le redessin du curseur
            inputReset(in);
        }

        // --- confirmation avant de quitter ---
        if (in.wantQuit)
        {
            if (gameState == 1 && timerRunning())
            {
                timerStop(); // pause du chrono
                if (confirmQuit())
                    quitRequested = 1;
                else
                    timerResume();
                viewChanged = 1; // redessine le centre de l'écran
                lastPx = -1;
                lastPy = -1; // force le redessin du curseur
                inputReset(in);
            }
            else
                quitRequested = 1;
        }

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

            canUndo = !anim.active && !shuffling && gameState != 2 && histCanUndo();
            canRedo = !anim.active && !shuffling && gameState != 2 && histCanRedo();

            drawButton(BTN_X0, BTN_Y0, BTN_X1, BTN_Y1, T(S_BTN_SHUFFLE), in.btnDown == BTN_SHUFFLE);
            drawButtonEx(UNDO_X0, UNDO_Y0, UNDO_X1, UNDO_Y1, "\033", in.btnDown == BTN_UNDO, canUndo);
            drawButtonEx(REDO_X0, REDO_Y0, REDO_X1, REDO_Y1, "\032", in.btnDown == BTN_REDO, canRedo);
            drawButtonEx(HELP_X0, HELP_Y0, HELP_X1, HELP_Y1, "?", in.btnDown == BTN_HELP, 1);
            drawButtonEx(QUIT_X0, QUIT_Y0, QUIT_X1, QUIT_Y1, "X", in.btnDown == BTN_QUIT, 1);

            waitVSync();
            blitRectCursor(VIEW_X0, VIEW_Y0, VIEW_X1, VIEW_Y1, px, py);
            blitRectCursor(HUD_X0, HUD_Y0, HUD_X1, HUD_Y1, px, py);
            blitRectCursor(BTN_X0, BTN_Y0, BTN_X1, BTN_Y1, px, py);
            blitRectCursor(NAV_X0, NAV_Y0, NAV_X1, NAV_Y1, px, py);
            blitRectCursor(TOP_X0, TOP_Y0, TOP_X1, TOP_Y1, px, py);
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
