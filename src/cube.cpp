#include "cube.h"

// Fonction d'initialisation complète du Rubik's Cube résolu
void initCube(RubikCube &cube)
{
    cube.moveCount = 0;
    cube.isMixed = 0;
    cube.startTime = 0;
    cube.elapsedTime = 0;
    cube.timerRunning = 0;

    int index = 0;

    // 1. Initialisation des 8 coins (8 pièces)
    int cornerCoords[8][3] = {
        {-1, 1, 1},  // 0: Haut-Gauche-Avant
        {1, 1, 1},   // 1: Haut-Droite-Avant
        {1, 1, -1},  // 2: Haut-Droite-Arrière
        {-1, 1, -1}, // 3: Haut-Gauche-Arrière
        {-1, -1, 1}, // 4: Bas-Gauche-Avant
        {1, -1, 1},  // 5: Bas-Droite-Avant
        {1, -1, -1}, // 6: Bas-Droite-Arrière
        {-1, -1, -1} // 7: Bas-Gauche-Arrière
    };

    for (int i = 0; i < 8; i++)
    {
        cube.pieces[index].originalPos.x = cornerCoords[i][0];
        cube.pieces[index].originalPos.y = cornerCoords[i][1];
        cube.pieces[index].originalPos.z = cornerCoords[i][2];
        cube.pieces[index].currentPos = cube.pieces[index].originalPos;

        for (int f = 0; f < 6; f++)
        {
            cube.pieces[index].faceColors[f] = COLOR_NONE;
        }

        if (cornerCoords[i][1] == 1)
            cube.pieces[index].faceColors[0] = COLOR_WHITE; // Haut
        if (cornerCoords[i][1] == -1)
            cube.pieces[index].faceColors[1] = COLOR_YELLOW; // Bas
        if (cornerCoords[i][0] == -1)
            cube.pieces[index].faceColors[2] = COLOR_ORANGE; // Gauche
        if (cornerCoords[i][0] == 1)
            cube.pieces[index].faceColors[3] = COLOR_RED; // Droite
        if (cornerCoords[i][2] == 1)
            cube.pieces[index].faceColors[4] = COLOR_GREEN; // Avant
        if (cornerCoords[i][2] == -1)
            cube.pieces[index].faceColors[5] = COLOR_BLUE; // Arrière

        index++;
    }

    // 2. Initialisation des 12 arêtes (12 pièces)
    int edgeCoords[12][3] = {
        {0, 1, 1},   // 8:  Haut-Avant
        {1, 1, 0},   // 9:  Haut-Droite
        {0, 1, -1},  // 10: Haut-Arrière
        {-1, 1, 0},  // 11: Haut-Gauche
        {0, -1, 1},  // 12: Bas-Avant
        {1, -1, 0},  // 13: Bas-Droite
        {0, -1, -1}, // 14: Bas-Arrière
        {-1, -1, 0}, // 15: Bas-Gauche
        {-1, 0, 1},  // 16: Milieu-Gauche-Avant
        {1, 0, 1},   // 17: Milieu-Droite-Avant
        {1, 0, -1},  // 18: Milieu-Droite-Arrière
        {-1, 0, -1}  // 19: Milieu-Gauche-Arrière
    };

    for (int j = 0; j < 12; j++)
    {
        cube.pieces[index].originalPos.x = edgeCoords[j][0];
        cube.pieces[index].originalPos.y = edgeCoords[j][1];
        cube.pieces[index].originalPos.z = edgeCoords[j][2];
        cube.pieces[index].currentPos = cube.pieces[index].originalPos;

        for (int f = 0; f < 6; f++)
        {
            cube.pieces[index].faceColors[f] = COLOR_NONE;
        }

        if (edgeCoords[j][1] == 1)
            cube.pieces[index].faceColors[0] = COLOR_WHITE; // Haut
        if (edgeCoords[j][1] == -1)
            cube.pieces[index].faceColors[1] = COLOR_YELLOW; // Bas
        if (edgeCoords[j][0] == -1)
            cube.pieces[index].faceColors[2] = COLOR_ORANGE; // Gauche
        if (edgeCoords[j][0] == 1)
            cube.pieces[index].faceColors[3] = COLOR_RED; // Droite
        if (edgeCoords[j][2] == 1)
            cube.pieces[index].faceColors[4] = COLOR_GREEN; // Avant
        if (edgeCoords[j][2] == -1)
            cube.pieces[index].faceColors[5] = COLOR_BLUE; // Arrière

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

    for (i = 0; i < 20; i++)
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

    for (i = 0; i < 20; i++)
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

    for (i = 0; i < 20; i++)
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