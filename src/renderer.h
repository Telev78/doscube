#ifndef RENDERER_H
#define RENDERER_H

#include "cube.h"

/* Zone rectangulaire (incluse) qui contient tout le cube, meme en rotation */
#define VIEW_X0 80
#define VIEW_Y0 30
#define VIEW_X1 240
#define VIEW_Y1 170

/* Zone du texte (compteur de coups) */
#define HUD_X0 0
#define HUD_Y0 0
#define HUD_X1 159
#define HUD_Y1 11

void initRenderer();
void clearRect(int x0, int y0, int x1, int y1);
void blitRect(int x0, int y0, int x1, int y1);
void drawCursorVRAM(int x, int y);
void drawText(int x, int y, const char *str, unsigned char color);

/* Retrouve la face coloree situee sous le pixel (px, py) */
int pickFace(int px, int py, int &owner, int &faceIdx);

/* Projection ecran (y vers le bas) d'un vecteur unite du modele.
   Les angles sont des degres entiers. */
void projectDir(int ax, int ay, int az, int angleX, int angleY, double &sx, double &sy);

/* Rendu du cube dans le back-buffer (arithmetique entiere, angles en degres entiers) */
void drawCube(const RubikCube &cube, int angleX, int angleY, const LayerAnim *anim = 0);

#endif