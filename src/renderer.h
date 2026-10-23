#ifndef RENDERER_H
#define RENDERER_H

#include "cube.h"

// Structure pour stocker un point projeté en 2D
struct Point2D
{
    int x;
    int y;
};

// Fonction de projection d'un point 3D vers 2D avec angles de rotation de la caméra (angleX, angleY)
Point2D project3DTo2D(int x, int y, int z, double angleX, double angleY, int centerX, int centerY, int scale);

// Fonction principale pour dessiner le cube complet à l'écran
void drawCube(const RubikCube &cube, double angleX, double angleY);

#endif