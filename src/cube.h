#ifndef CUBE_H
#define CUBE_H

// 1. Énumération des 6 couleurs des faces
enum CubeColor
{
    COLOR_NONE = 0,
    COLOR_WHITE,
    COLOR_YELLOW,
    COLOR_RED,
    COLOR_ORANGE,
    COLOR_BLUE,
    COLOR_GREEN
};

// 2. Position spatiale sur la grille 3D (-1, 0, 1)
struct Point3D
{
    int x;
    int y;
    int z;
};

// 3. Structure d'une pièce individuelle (cubie)
struct Cubie
{
    Point3D currentPos;  // Position actuelle (-1, 0, 1)
    Point3D originalPos; // Position cible (état résolu)

    // Couleurs des 6 faces locales :
    // [0]: Haut, [1]: Bas, [2]: Gauche, [3]: Droite, [4]: Avant, [5]: Arrière
    unsigned char faceColors[6];
};

// 4. Structure globale du jeu
struct RubikCube
{
    Cubie pieces[26];       // 8 coins + 12 arêtes + 6 centres
    unsigned int moveCount; // Nombre de rotations effectuées
    int isMixed;            // Status du mélange (0: résolu, 1: mélangé)

    unsigned long startTime;   // Horodatage du début
    unsigned long elapsedTime; // Temps écoulé
    int timerRunning;          // État du chrono (0: pause, 1: en cours)
};

void initCube(RubikCube &cube);
// Fonctions de rotation des faces
void rotateX(RubikCube &cube, int targetX, int clockwise);
void rotateY(RubikCube &cube, int targetY, int clockwise);
void rotateZ(RubikCube &cube, int targetZ, int clockwise);
struct LayerAnim
{
    int active;
    int axis;
    int layer;
    int sign;
    int step;
    int steps;
};
void rotateLayer(RubikCube &cube, int axis, int layer, int sign);

#endif