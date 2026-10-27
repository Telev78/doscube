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
void drawCube(const RubikCube &cube, double angleX, double angleY, const LayerAnim *anim = 0);
int pickFace(int px, int py, int &owner, int &faceIdx);
void projectDir(int ax, int ay, int az, double angleX, double angleY, double &sx, double &sy);
void drawCursor(int x, int y);

#endif