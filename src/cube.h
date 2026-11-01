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
    Point3D currentPos; // Position actuelle (-1, 0, 1)

    // Couleurs des 6 faces locales :
    // [0]: Haut, [1]: Bas, [2]: Gauche, [3]: Droite, [4]: Avant, [5]: Arrière
    unsigned char faceColors[6];
};

// 4. Structure globale du jeu
struct RubikCube
{
    Cubie pieces[26];       // 8 coins + 12 arêtes + 6 centres
    unsigned int moveCount; // Coups du joueur : tenu par l'appelant (rotateLayer n'y touche pas)
};

/* Axe (0=X, 1=Y, 2=Z) et sens (+1/-1) de la normale de chacune des 6 faces :
   0 Haut, 1 Bas, 2 Gauche, 3 Droite, 4 Avant, 5 Arriere */
extern const int faceAxis[6];
extern const int faceSign[6];

void initCube(RubikCube &cube);

struct LayerAnim
{
    int active;
    int axis;
    int layer;
    int sign;
    int step;
    int steps;
};

// axis : 0=X 1=Y 2=Z ; layer : -1, 0 ou 1 ; sign : +1 = +90° règle de la main droite
// autour de +axis, -1 = -90°. Ne modifie pas moveCount.
void rotateLayer(RubikCube &cube, int axis, int layer, int sign);
int isSolved(const RubikCube &cube);
void randomMove(int lastAxis, int lastLayer, int &axis, int &layer, int &sign);

#endif