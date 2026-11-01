#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include "cube.h"
#include "mouse.h"
#include "renderer.h"
#include "control.h"

static int levi(int i, int j, int k) // signe de la permutation (i,j,k) de {0,1,2}
{
    return (((j - i + 3) % 3) == 1 && ((k - j + 3) % 3) == 1) ? 1 : -1;
}

int computeMove(const RubikCube &cube, int owner, int f, int dx, int dy,
                int &axis, int &layer, int &sign)
{
    int n = faceAxis[f], ns = faceSign[f];
    int pos[3], v[3], t, a = -1, b;
    double best = -1.0, sx, sy, len, d, dirSign = 1.0;

    pos[0] = cube.pieces[owner].currentPos.x;
    pos[1] = cube.pieces[owner].currentPos.y;
    pos[2] = cube.pieces[owner].currentPos.z;

    for (t = 0; t < 3; t++)
    {
        if (t == n)
            continue;
        v[0] = v[1] = v[2] = 0;
        v[t] = 1;
        projectDir(v[0], v[1], v[2], sx, sy);
        len = sqrt(sx * sx + sy * sy);
        if (len < 0.2)
            continue; // axe quasi vu de bout : direction peu fiable
        d = (dx * sx + dy * sy) / len;
        if (fabs(d) > best)
        {
            best = fabs(d);
            a = t;
            dirSign = (d > 0) ? 1.0 : -1.0;
        }
    }
    if (a < 0)
        return 0;

    b = 3 - n - a; // axe de rotation = l'autre axe tangent
    layer = pos[b];
    axis = b;
    // vitesse = w x r  =>  s = dirSign * ns * eps(b, n, a)
    sign = (int)dirSign * ns * levi(b, n, a);
    return 1;
}

void inputInit(Input &in)
{
    memset(&in, 0, sizeof(in));
    in.moveDone = 1;
}

void inputReset(Input &in)
{
    in.faceDrag = 0;
    in.btnDown = BTN_NONE;
    in.moveDone = 1;
    in.viewDrag = 0;
}

void inputUpdate(Input &in, const RubikCube &cube, int busy, int canUndo, int canRedo)
{
    int mouseX, mouseY, key, dx, dy;

    // Les évènements ne durent qu'une itération
    in.wantShuffle = 0;
    in.wantUndo = 0;
    in.wantRedo = 0;
    in.wantHelp = 0;
    in.wantQuit = 0;
    in.wantLanguage = 0;
    in.moveRequested = 0;
    in.redraw = 0;

    getMouseStatus(mouseX, mouseY, in.left, in.right);
    in.px = mouseX / 2;
    in.py = mouseY;

    // --- Clavier ---
    if (kbhit())
    {
        key = getch();
        if (key == 0 || key == 224) // touche étendue : le code est dans l'octet suivant
        {
            key = getch();
            if (key == 75) // flèche gauche
                in.wantUndo = 1;
            else if (key == 77) // flèche droite
                in.wantRedo = 1;
            else if (key == 59) // F1
                in.wantHelp = 1;
        }
        else if (key == 27) // Echap pour quitter
            in.wantQuit = 1;
        else if (key == '?') // ? pour afficher l'aide
            in.wantHelp = 1;
        else if (key == 'm' || key == 'M') // m pour mélanger le cube
            in.wantShuffle = 1;
        else if (key == 'u' || key == 'U') // undo
            in.wantUndo = 1;
        else if (key == 'r' || key == 'R') // redo
            in.wantRedo = 1;
        else if (key == 'l' || key == 'L') // boucle entre les langues d'interface
            in.wantLanguage = 1;
    }

    // --- Clic droit : rotation de la vue ---
    if (in.right)
    {
        in.moveDone = 1;
        if (in.viewDrag && (in.px != in.prevX || in.py != in.prevY))
        {
            rotateView(DRAG_X_SIGN * (in.px - in.prevX), DRAG_Y_SIGN * (in.py - in.prevY));
            in.redraw = 1;
        }
        in.viewDrag = 1;
        in.prevX = in.px;
        in.prevY = in.py;
    }
    else
        in.viewDrag = 0;

    // --- Clic gauche : bouton ou rotation d'une face ---
    if (in.left && !in.right)
    {
        if (!in.faceDrag)
        {
            in.faceDrag = 1;
            in.startX = in.px;
            in.startY = in.py;
            in.btnDown = BTN_NONE;
            if (IN_BUTTON(in.px, in.py))
                in.btnDown = BTN_SHUFFLE;
            else if (IN_UNDO(in.px, in.py) && canUndo)
                in.btnDown = BTN_UNDO;
            else if (IN_REDO(in.px, in.py) && canRedo)
                in.btnDown = BTN_REDO;
            else if (IN_HELP(in.px, in.py))
                in.btnDown = BTN_HELP;
            else if (IN_QUIT(in.px, in.py))
                in.btnDown = BTN_QUIT;

            if (in.btnDown)
            {
                in.moveDone = 1;
                in.redraw = 1;
            }
            else if (IN_UNDO(in.px, in.py) || IN_REDO(in.px, in.py))
                in.moveDone = 1; // bouton grisé : on ignore le clic
            else
                in.moveDone = busy || !pickFace(in.px, in.py, in.owner, in.fIdx);
        }
        else if (!in.moveDone)
        {
            dx = in.px - in.startX;
            dy = in.py - in.startY;
            if (abs(dx) + abs(dy) >= 8)
            {
                if (!busy && computeMove(cube, in.owner, in.fIdx, dx, dy,
                                         in.moveAxis, in.moveLayer, in.moveSign))
                    in.moveRequested = 1;
                in.moveDone = 1;
            }
        }
    }
    else if (!in.left)
    {
        if (in.btnDown)
        {
            // Relâchement : le clic est valide s'il est encore sur le même bouton
            if (in.btnDown == BTN_SHUFFLE && IN_BUTTON(in.px, in.py))
                in.wantShuffle = 1;
            else if (in.btnDown == BTN_UNDO && IN_UNDO(in.px, in.py))
                in.wantUndo = 1;
            else if (in.btnDown == BTN_REDO && IN_REDO(in.px, in.py))
                in.wantRedo = 1;
            else if (in.btnDown == BTN_HELP && IN_HELP(in.px, in.py))
                in.wantHelp = 1;
            else if (in.btnDown == BTN_QUIT && IN_QUIT(in.px, in.py))
                in.wantQuit = 1;
            in.btnDown = BTN_NONE;
            in.redraw = 1;
        }
        in.faceDrag = 0;
    }
}
