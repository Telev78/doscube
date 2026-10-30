#ifndef CONTROL_H
#define CONTROL_H

#include "cube.h"

/* Boutons de l'interface (valeur de Input::btnDown) */
#define BTN_NONE    0
#define BTN_SHUFFLE 1
#define BTN_UNDO    2
#define BTN_REDO    3
#define BTN_HELP    4
#define BTN_QUIT    5

/* Etat des entrees (souris et clavier) et evenements produits.
   Les evenements (want..., moveRequested, redraw) ne vivent qu'une iteration :
   inputUpdate() les remet a zero a chaque appel. */
struct Input
{
    /* Position et boutons de la souris (pixels ecran, 320x200) */
    int px, py;
    int left, right;

    /* Etat interne du glisser-deposer */
    int viewDrag;            /* clic droit en cours : rotation de la vue          */
    int faceDrag;            /* clic gauche en cours                              */
    int moveDone;            /* 1 : le glissement en cours ne declenchera plus rien */
    int startX, startY;      /* point d'appui du clic gauche                      */
    int prevX, prevY;        /* position precedente (rotation de la vue)          */
    int owner, fIdx;         /* cubie et face designes par le clic gauche         */

    /* Bouton de l'interface actuellement enfonce (BTN_...) */
    int btnDown;

    /* Evenements */
    int wantShuffle, wantUndo, wantRedo, wantHelp, wantQuit, wantLanguage;
    int redraw;              /* la vue ou l'etat d'un bouton a change : redessiner  */

    /* Coup demande par un glissement sur une face (rotation de 90 degres) */
    int moveRequested;
    int moveAxis, moveLayer, moveSign;
};

/* Initialise l'etat des entrees */
void inputInit(Input &in);

/* Abandonne tout clic en cours (a appeler au retour d'une fenetre modale) */
void inputReset(Input &in);

/* Lit la souris et le clavier et met a jour l'etat et les evenements.
     busy    : une animation ou un melange est en cours (aucun coup ne peut demarrer)
     canUndo : le bouton "annuler" est utilisable
     canRedo : le bouton "refaire" est utilisable */
void inputUpdate(Input &in, const RubikCube &cube, int busy, int canUndo, int canRedo);

/* Transforme un glissement de souris (dx, dy) sur la face f du cubie owner en une
   rotation (axe, couche, sens). Retourne 0 si la direction est inexploitable. */
int computeMove(const RubikCube &cube, int owner, int f, int dx, int dy,
                int &axis, int &layer, int &sign);

#endif
