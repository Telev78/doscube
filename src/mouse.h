#ifndef MOUSE_H
#define MOUSE_H

// Initialisation du pilote de la souris (AX = 0x0000)
int initMouse();

// Affichage du curseur de la souris (AX = 0x0001)
void showMouseCursor();

// Masquage du curseur de la souris (AX = 0x0002)
void hideMouseCursor();

// Récupération de l'état et de la position de la souris (AX = 0x0003)
void getMouseStatus(int &x, int &y, int &leftButton, int &rightButton);

#endif