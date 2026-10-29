#include "cube.h"
#include <stdlib.h>

// Initialise une pièce à la position (x, y, z) : une face colorée par coordonnée à ±1
static void setupPiece(Cubie &p, int x, int y, int z)
{
    int f;
    p.originalPos.x = x;
    p.originalPos.y = y;
    p.originalPos.z = z;
    p.currentPos = p.originalPos;

    for (f = 0; f < 6; f++)
        p.faceColors[f] = COLOR_NONE;

    if (y == 1)
        p.faceColors[0] = COLOR_WHITE; // Haut
    if (y == -1)
        p.faceColors[1] = COLOR_YELLOW; // Bas
    if (x == -1)
        p.faceColors[2] = COLOR_ORANGE; // Gauche
    if (x == 1)
        p.faceColors[3] = COLOR_RED; // Droite
    if (z == 1)
        p.faceColors[4] = COLOR_GREEN; // Avant
    if (z == -1)
        p.faceColors[5] = COLOR_BLUE; // Arrière
}

void initCube(RubikCube &cube)
{
    int x, y, z;
    int index = 0;

    cube.moveCount = 0;
    cube.isMixed = 0;
    cube.startTime = 0;
    cube.elapsedTime = 0;
    cube.timerRunning = 0;

    for (x = -1; x <= 1; x++)
        for (y = -1; y <= 1; y++)
            for (z = -1; z <= 1; z++)
            {
                if (x == 0 && y == 0 && z == 0)
                    continue; // pas de pièce au centre du cube
                setupPiece(cube.pieces[index], x, y, z);
                index++;
            }
}

// --- ROTATION AUTOUR DE L'AXE X (targetX = 1 pour Droite, -1 pour Gauche) ---
void rotateX(RubikCube &cube, int targetX, int clockwise)
{
    int indices[9];
    int count = 0;
    int i, k, f, idx;
    int y, z;
    Point3D newPos[9];
    unsigned char newColors[9][6];
    unsigned char c0, c1, c4, c5;
    int actualClockwise;

    for (i = 0; i < 26; i++)
    {
        if (cube.pieces[i].currentPos.x == targetX)
        {
            indices[count++] = i;
        }
    }

    actualClockwise = (targetX == 1) ? clockwise : !clockwise;

    for (k = 0; k < count; k++)
    {
        idx = indices[k];
        y = cube.pieces[idx].currentPos.y;
        z = cube.pieces[idx].currentPos.z;

        newPos[k].x = targetX;
        if (actualClockwise)
        {
            newPos[k].y = -z;
            newPos[k].z = y;
        }
        else
        {
            newPos[k].y = z;
            newPos[k].z = -y;
        }

        for (f = 0; f < 6; f++)
        {
            newColors[k][f] = cube.pieces[idx].faceColors[f];
        }

        c0 = cube.pieces[idx].faceColors[0];
        c1 = cube.pieces[idx].faceColors[1];
        c4 = cube.pieces[idx].faceColors[4];
        c5 = cube.pieces[idx].faceColors[5];

        if (actualClockwise)
        {
            newColors[k][4] = c0; // Haut -> Avant
            newColors[k][1] = c4; // Avant -> Bas
            newColors[k][5] = c1; // Bas -> Arrière
            newColors[k][0] = c5; // Arrière -> Haut
        }
        else
        {
            newColors[k][5] = c0; // Haut -> Arrière
            newColors[k][1] = c5; // Arrière -> Bas
            newColors[k][4] = c1; // Bas -> Avant
            newColors[k][0] = c4; // Avant -> Haut
        }
    }

    for (k = 0; k < count; k++)
    {
        idx = indices[k];
        cube.pieces[idx].currentPos = newPos[k];
        for (f = 0; f < 6; f++)
        {
            cube.pieces[idx].faceColors[f] = newColors[k][f];
        }
    }

    cube.moveCount++;
}

// --- ROTATION AUTOUR DE L'AXE Y (targetY = 1 pour Haut, -1 pour Bas) ---
void rotateY(RubikCube &cube, int targetY, int clockwise)
{
    int indices[9];
    int count = 0;
    int i, k, f, idx;
    int x, z;
    Point3D newPos[9];
    unsigned char newColors[9][6];
    unsigned char c2, c3, c4, c5;
    int actualClockwise;

    for (i = 0; i < 26; i++)
    {
        if (cube.pieces[i].currentPos.y == targetY)
        {
            indices[count++] = i;
        }
    }

    actualClockwise = (targetY == 1) ? clockwise : !clockwise;

    for (k = 0; k < count; k++)
    {
        idx = indices[k];
        x = cube.pieces[idx].currentPos.x;
        z = cube.pieces[idx].currentPos.z;

        newPos[k].y = targetY;
        if (actualClockwise)
        {
            newPos[k].x = z;
            newPos[k].z = -x;
        }
        else
        {
            newPos[k].x = -z;
            newPos[k].z = x;
        }

        for (f = 0; f < 6; f++)
        {
            newColors[k][f] = cube.pieces[idx].faceColors[f];
        }

        c2 = cube.pieces[idx].faceColors[2];
        c3 = cube.pieces[idx].faceColors[3];
        c4 = cube.pieces[idx].faceColors[4];
        c5 = cube.pieces[idx].faceColors[5];

        if (actualClockwise)
        {
            newColors[k][3] = c4; // Avant -> Droite
            newColors[k][5] = c3; // Droite -> Arrière
            newColors[k][2] = c5; // Arrière -> Gauche
            newColors[k][4] = c2; // Gauche -> Avant
        }
        else
        {
            newColors[k][2] = c4; // Avant -> Gauche
            newColors[k][5] = c2; // Gauche -> Arrière
            newColors[k][3] = c5; // Arrière -> Droite
            newColors[k][4] = c3; // Droite -> Avant
        }
    }

    for (k = 0; k < count; k++)
    {
        idx = indices[k];
        cube.pieces[idx].currentPos = newPos[k];
        for (f = 0; f < 6; f++)
        {
            cube.pieces[idx].faceColors[f] = newColors[k][f];
        }
    }

    cube.moveCount++;
}

// --- ROTATION AUTOUR DE L'AXE Z (targetZ = 1 pour Avant, -1 pour Arrière) ---
void rotateZ(RubikCube &cube, int targetZ, int clockwise)
{
    int indices[9];
    int count = 0;
    int i, k, f, idx;
    int x, y;
    Point3D newPos[9];
    unsigned char newColors[9][6];
    unsigned char c0, c1, c2, c3;
    int actualClockwise;

    for (i = 0; i < 26; i++)
    {
        if (cube.pieces[i].currentPos.z == targetZ)
        {
            indices[count++] = i;
        }
    }

    actualClockwise = (targetZ == 1) ? clockwise : !clockwise;

    for (k = 0; k < count; k++)
    {
        idx = indices[k];
        x = cube.pieces[idx].currentPos.x;
        y = cube.pieces[idx].currentPos.y;

        newPos[k].z = targetZ;
        if (actualClockwise)
        {
            newPos[k].x = y;
            newPos[k].y = -x;
        }
        else
        {
            newPos[k].x = -y;
            newPos[k].y = x;
        }

        for (f = 0; f < 6; f++)
        {
            newColors[k][f] = cube.pieces[idx].faceColors[f];
        }

        c0 = cube.pieces[idx].faceColors[0];
        c1 = cube.pieces[idx].faceColors[1];
        c2 = cube.pieces[idx].faceColors[2];
        c3 = cube.pieces[idx].faceColors[3];

        if (actualClockwise)
        {
            newColors[k][3] = c0; // Haut -> Droite
            newColors[k][1] = c3; // Droite -> Bas
            newColors[k][2] = c1; // Bas -> Gauche
            newColors[k][0] = c2; // Gauche -> Haut
        }
        else
        {
            newColors[k][2] = c0; // Haut -> Gauche
            newColors[k][1] = c2; // Gauche -> Bas
            newColors[k][3] = c1; // Bas -> Droite
            newColors[k][0] = c3; // Droite -> Haut
        }
    }

    for (k = 0; k < count; k++)
    {
        idx = indices[k];
        cube.pieces[idx].currentPos = newPos[k];
        for (f = 0; f < 6; f++)
        {
            cube.pieces[idx].faceColors[f] = newColors[k][f];
        }
    }

    cube.moveCount++;
}

// axis: 0=X 1=Y 2=Z ; layer: -1 ou 1 ; sign: +1 = +90° règle de la main droite autour de +axis
void rotateLayer(RubikCube &cube, int axis, int layer, int sign)
{
    int A = (axis == 2) ? (sign < 0) : (sign > 0);
    int cw = (layer == 1) ? A : !A;
    if (axis == 0)
        rotateX(cube, layer, cw);
    else if (axis == 1)
        rotateY(cube, layer, cw);
    else
        rotateZ(cube, layer, cw);
}

// 1 si chacune des 6 faces n'a qu'une seule couleur
// (fonctionne même si les tranches centrales ont déplacé les centres)
int isSolved(const RubikCube &cube)
{
    static const int fAxis[6] = {1, 1, 0, 0, 2, 2};
    static const int fSign[6] = {1, -1, -1, 1, 1, -1};
    int f, i, coord, ref, c;
    const Point3D *p;

    for (f = 0; f < 6; f++)
    {
        ref = -1;
        for (i = 0; i < 26; i++)
        {
            p = &cube.pieces[i].currentPos;
            coord = (fAxis[f] == 0) ? p->x : (fAxis[f] == 1) ? p->y
                                                             : p->z;
            if (coord != fSign[f])
                continue;
            c = cube.pieces[i].faceColors[f];
            if (c == COLOR_NONE)
                return 0;
            if (ref < 0)
                ref = c;
            else if (c != ref)
                return 0;
        }
    }
    return 1;
}

// Tire une face au hasard (couche extérieure) et un sens, en évitant de
// retomber sur la même face que le coup précédent
void randomMove(int lastAxis, int lastLayer, int &axis, int &layer, int &sign)
{
    do
    {
        axis = rand() % 3;
        layer = (rand() % 3) - 1; // -1, 0 ou 1p
    } while (axis == lastAxis && layer == lastLayer);
    sign = (rand() % 2) ? 1 : -1;
}