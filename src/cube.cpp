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

        // Initialisation de toutes les faces à COLOR_NONE
        for (int f = 0; f < 6; f++)
        {
            cube.pieces[index].faceColors[f] = COLOR_NONE;
        }

        // Attribution des couleurs selon les faces d'origine
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