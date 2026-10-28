#ifndef COMMON_H
#define COMMON_H

/* Constantes textuelles pour l'interface */

const char *const STR_ERR_NO_VGA = "Erreur : une carte VGA est necessaire (mode 13h, 320x200x256).";
const char *const STR_ERR_MODE13 = "Erreur : impossible d'activer le mode graphique 13h.";
const char *const STR_ERR_NO_MOUSE = "Erreur : souris non detectee !";
const char *const STR_ERR_MOUSE_BUTTONS = "Erreur : souris a 2 boutons requise.";
const char *const STR_MOVES = "Coups : %u";
const char *const STR_SOLVED = "Resolu en %u coups !";
const char *const STR_SHUFFLING = "Melange...";
const char *const STR_BTN_SHUFFLE = "Melanger";

#endif