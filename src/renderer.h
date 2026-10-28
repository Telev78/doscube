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
#define HUD_X1 239
#define HUD_Y1 11

/* Bouton "Melanger" en bas a droite */
#define BTN_X0 244
#define BTN_Y0 178
#define BTN_X1 315
#define BTN_Y1 195
#define IN_BUTTON(x, y) ((x) >= BTN_X0 && (x) <= BTN_X1 && (y) >= BTN_Y0 && (y) <= BTN_Y1)

void initRenderer();
void fillRect(int x0, int y0, int x1, int y1, unsigned char color);
void drawButton(int x0, int y0, int x1, int y1, const char *text, int pressed);
void clearRect(int x0, int y0, int x1, int y1);
void blitRect(int x0, int y0, int x1, int y1);
void drawCursorVRAM(int x, int y);
void drawText(int x, int y, const char *str, unsigned char color);

/* Retrouve la face coloree situee sous le pixel (px, py) */
int pickFace(int px, int py, int &owner, int &faceIdx);

/* Sens des glissements de souris pour la rotation de la vue :
   1 = la face tournee vers nous suit le pointeur, -1 = sens inverse */
#define DRAG_X_SIGN 1
#define DRAG_Y_SIGN 1

/* Orientation de la vue (trackball) */
void resetView(int angleX, int angleY);
void rotateView(int dx, int dy);

/* Projection ecran (y vers le bas) d'un vecteur unite du modele */
void projectDir(int ax, int ay, int az, double &sx, double &sy);

/* Rendu du cube dans le back-buffer (arithmetique entiere) */
void drawCube(const RubikCube &cube, const LayerAnim *anim = 0);

#endif