#ifndef HISTORY_H
#define HISTORY_H

/* Historique des coups du joueur, navigable dans les deux sens.
   - un coup joue a la main s'ajoute a la position courante ;
   - si on est revenu en arriere, il remplace tous les coups "a refaire" ;
   - annuler / refaire se deplacent sur l'historique sans le modifier.
   Un coup = une rotation de 90 degres : axe (0=X 1=Y 2=Z), couche (-1, 0, 1),
   sens (+1 / -1, regle de la main droite), comme rotateLayer(). */

#define HISTORY_MAX 512

void histClear();

/* Enregistre un coup manuel (tronque les coups a refaire) */
void histPush(int axis, int layer, int sign);

int histCanUndo();
int histCanRedo();

/* Donne le coup a JOUER pour annuler (c'est-a-dire l'inverse du dernier coup)
   ou pour refaire. Retournent 0 s'il n'y a rien a faire. N'avancent pas. */
int histPeekUndo(int &axis, int &layer, int &sign);
int histPeekRedo(int &axis, int &layer, int &sign);

/* A appeler une fois le coup effectivement joue */
void histCommitUndo();
void histCommitRedo();

#endif