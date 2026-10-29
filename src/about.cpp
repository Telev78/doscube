#include <stdio.h>
#include <string.h>
#include <conio.h>
#include "renderer.h"
#include "mouse.h"
#include "lang.h"
#include "options.h"
#include "cpu.h"
#include "about.h"

/* Cadre de la fenetre */
#define DLG_X0 4
#define DLG_Y0 6
#define DLG_X1 315
#define DLG_Y1 193

/* Bouton X de fermeture (coin superieur droit de la fenetre) */
#define CLOSE_X0 297
#define CLOSE_Y0 10
#define CLOSE_X1 310
#define CLOSE_Y1 23
#define IN_CLOSE(x, y) ((x) >= CLOSE_X0 && (x) <= CLOSE_X1 && (y) >= CLOSE_Y0 && (y) <= CLOSE_Y1)

static void centerText(int y, const char *s, unsigned char color)
{
    drawText((320 - (int)strlen(s) * 8) / 2, y, s, color);
}

static void drawDialog(int closeDown)
{
    static const unsigned char titleColors[7] = {12, 41, 14, 10, 9, 15, 13};
    char letter[2];
    char line[48];
    const char *title = APP_NAME;
    int i, x, n;

    fillRect(0, 0, 319, 199, 0);                  /* fond noir        */
    fillRect(DLG_X0, DLG_Y0, DLG_X1, DLG_Y1, 15); /* cadre blanc      */
    fillRect(DLG_X0 + 2, DLG_Y0 + 2, DLG_X1 - 2, DLG_Y1 - 2, 0);
    fillRect(DLG_X0 + 4, DLG_Y0 + 4, DLG_X1 - 4, DLG_Y0 + 4, 8); /* filet interieur */
    fillRect(DLG_X0 + 4, DLG_Y1 - 4, DLG_X1 - 4, DLG_Y1 - 4, 8);
    fillRect(DLG_X0 + 4, DLG_Y0 + 4, DLG_X0 + 4, DLG_Y1 - 4, 8);
    fillRect(DLG_X1 - 4, DLG_Y0 + 4, DLG_X1 - 4, DLG_Y1 - 4, 8);

    /* Titre en grand (3x), chaque lettre dans une couleur de cube, avec ombre */
    n = (int)strlen(title);
    x = (320 - n * 24) / 2;
    letter[1] = 0;
    for (i = 0; i < n; i++)
    {
        letter[0] = title[i];
        drawTextScaled(x + i * 24 + 2, 18, letter, 8, 3);
        drawTextScaled(x + i * 24, 16, letter, titleColors[i % 7], 3);
    }

    sprintf(line, T(S_ABOUT_VERSION), APP_VERSION);
    centerText(46, line, 15);
    centerText(58, APP_AUTHOR, 7);
    fillRect(40, 72, 279, 72, 8);

    /* Rappel des commandes */
    drawText(24, 80, T(S_ABOUT_MOUSE), 14);
    drawText(40, 92, T(S_ABOUT_MOUSE1), 15);
    drawText(40, 102, T(S_ABOUT_MOUSE2), 15);

    drawText(24, 118, T(S_ABOUT_KEYS), 14);
    drawText(40, 130, T(S_ABOUT_KEYS1), 15);
    drawText(40, 140, T(S_ABOUT_KEYS2), 15);
    drawText(40, 150, T(S_ABOUT_KEYS3), 15);

    sprintf(line, "CPU: %s  Mode: %s", cpuClassName(options.cpuClass),
            options.cpuMode == CPU_SLOW ? "SLOW" : "FAST");
    centerText(164, line, 8);
    centerText(178, T(S_ABOUT_CLOSE), 7);

    drawButtonEx(CLOSE_X0, CLOSE_Y0, CLOSE_X1, CLOSE_Y1, "X", closeDown, 1);
}

void showAbout()
{
    int mx, my, left, right, px, py;
    int lastX, lastY, wasDown = 0, closeDown = 0, done = 0, key;

    drawDialog(0);
    blitRect(0, 0, 319, 199);

    /* Attend le relachement du clic qui a ouvert la fenetre */
    do
    {
        getMouseStatus(mx, my, left, right);
    } while (left || right);

    px = mx / 2;
    py = my;
    drawCursorVRAM(px, py);
    lastX = px;
    lastY = py;

    while (!done)
    {
        getMouseStatus(mx, my, left, right);
        px = mx / 2;
        py = my;

        if (kbhit())
        {
            key = getch();
            if (key == 0 || key == 224)
                getch(); /* touche etendue : ignoree */
            else if (key == 27 || key == 13 || key == ' ')
                done = 1;
            else if (key == 'l' || key == 'L')
            {
                nextLanguage();
                drawDialog(closeDown);
                blitRect(0, 0, 319, 199);
                lastX = -100; /* force le redessin du curseur */
            }
        }

        /* Clic sur le bouton X : valide au relachement s'il est encore dessus */
        if (left)
        {
            if (!wasDown)
            {
                wasDown = 1;
                closeDown = IN_CLOSE(px, py);
                if (closeDown)
                {
                    drawButtonEx(CLOSE_X0, CLOSE_Y0, CLOSE_X1, CLOSE_Y1, "X", 1, 1);
                    blitRectCursor(CLOSE_X0, CLOSE_Y0, CLOSE_X1, CLOSE_Y1, px, py);
                }
            }
        }
        else if (wasDown)
        {
            if (closeDown && IN_CLOSE(px, py))
                done = 1;
            else if (closeDown)
            {
                drawButtonEx(CLOSE_X0, CLOSE_Y0, CLOSE_X1, CLOSE_Y1, "X", 0, 1);
                blitRectCursor(CLOSE_X0, CLOSE_Y0, CLOSE_X1, CLOSE_Y1, px, py);
            }
            closeDown = 0;
            wasDown = 0;
        }

        if (px != lastX || py != lastY)
        {
            blitRect(lastX, lastY, lastX + CURSOR_W - 1, lastY + CURSOR_H - 1);
            drawCursorVRAM(px, py);
            lastX = px;
            lastY = py;
        }
    }

    /* Evite qu'un clic encore enfonce agisse sur l'ecran de jeu */
    do
    {
        getMouseStatus(mx, my, left, right);
    } while (left);

    fillRect(0, 0, 319, 199, 0);
    blitRect(0, 0, 319, 199);
}

/* ------------------------------------------------------------------ */
/* Confirmation avant de quitter                                      */
/* ------------------------------------------------------------------ */

#define QD_X0 40
#define QD_Y0 66
#define QD_X1 279
#define QD_Y1 133

#define QY_X0 70
#define QY_Y0 108
#define QY_X1 139
#define QY_Y1 125
#define QN_X0 181
#define QN_Y0 108
#define QN_X1 250
#define QN_Y1 125

#define IN_QYES(x, y) ((x) >= QY_X0 && (x) <= QY_X1 && (y) >= QY_Y0 && (y) <= QY_Y1)
#define IN_QNO(x, y) ((x) >= QN_X0 && (x) <= QN_X1 && (y) >= QN_Y0 && (y) <= QN_Y1)

static void drawConfirm(int down) /* down : 0 rien, 1 Oui enfonce, 2 Non enfonce */
{
    fillRect(QD_X0, QD_Y0, QD_X1, QD_Y1, 15);
    fillRect(QD_X0 + 2, QD_Y0 + 2, QD_X1 - 2, QD_Y1 - 2, 0);
    centerText(QD_Y0 + 10, T(S_QUIT_TITLE), 14);
    centerText(QD_Y0 + 26, T(S_QUIT_MSG), 15);
    drawButtonEx(QY_X0, QY_Y0, QY_X1, QY_Y1, T(S_QUIT_YES), down == 1, 1);
    drawButtonEx(QN_X0, QN_Y0, QN_X1, QN_Y1, T(S_QUIT_NO), down == 2, 1);
}

int confirmQuit()
{
    int mx, my, left, right, px, py;
    int lastX, lastY, wasDown = 0, down = 0, result = -1, key;

    drawConfirm(0);
    blitRect(QD_X0, QD_Y0, QD_X1, QD_Y1);

    /* Attend le relachement d'un eventuel clic (celui du bouton X) */
    do
    {
        getMouseStatus(mx, my, left, right);
    } while (left || right);

    px = mx / 2;
    py = my;
    drawCursorVRAM(px, py);
    lastX = px;
    lastY = py;

    while (result < 0)
    {
        getMouseStatus(mx, my, left, right);
        px = mx / 2;
        py = my;

        if (kbhit())
        {
            key = getch();
            if (key == 0 || key == 224)
                getch();
            else if (key == 'y' || key == 'Y' || key == 'o' || key == 'O' ||
                     key == 'j' || key == 'J')
                result = 1;
            else if (key == 'n' || key == 'N' || key == 27 || key == 13 || key == ' ')
                result = 0;
            else if (key == 'l' || key == 'L')
            {
                nextLanguage();
                drawConfirm(down);
                blitRect(QD_X0, QD_Y0, QD_X1, QD_Y1);
                lastX = -100; /* force le redessin du curseur */
            }
        }

        if (left)
        {
            if (!wasDown)
            {
                wasDown = 1;
                down = IN_QYES(px, py) ? 1 : (IN_QNO(px, py) ? 2 : 0);
                if (down)
                {
                    drawConfirm(down);
                    blitRectCursor(QD_X0, QD_Y0, QD_X1, QD_Y1, px, py);
                }
            }
        }
        else if (wasDown)
        {
            if (down == 1 && IN_QYES(px, py))
                result = 1;
            else if (down == 2 && IN_QNO(px, py))
                result = 0;
            else if (down)
            {
                drawConfirm(0);
                blitRectCursor(QD_X0, QD_Y0, QD_X1, QD_Y1, px, py);
            }
            down = 0;
            wasDown = 0;
        }

        if (px != lastX || py != lastY)
        {
            blitRect(lastX, lastY, lastX + CURSOR_W - 1, lastY + CURSOR_H - 1);
            drawCursorVRAM(px, py);
            lastX = px;
            lastY = py;
        }
    }

    do
    {
        getMouseStatus(mx, my, left, right);
    } while (left);

    /* Les bandes a gauche et a droite de la zone du cube ne sont pas redessinees
       par la boucle principale : on les efface ici. Le centre sera redessine. */
    fillRect(QD_X0, QD_Y0, VIEW_X0 - 1, QD_Y1, 0);
    blitRect(QD_X0, QD_Y0, VIEW_X0 - 1, QD_Y1);
    fillRect(VIEW_X1 + 1, QD_Y0, QD_X1, QD_Y1, 0);
    blitRect(VIEW_X1 + 1, QD_Y0, QD_X1, QD_Y1);

    return result;
}