#ifndef MOUSE_H
#define MOUSE_H

// Initialisation du pilote de la souris (AX = 0x0000)
// Retourne 0 si absente, sinon le nombre de boutons (-1 = pilote ne le précisant pas)
int initMouse();

// Récupération de l'état et de la position de la souris (AX = 0x0003)
// Le curseur est dessiné par le programme (renderer.cpp), pas par le pilote.
void getMouseStatus(int &x, int &y, int &leftButton, int &rightButton);

#endif