#ifndef ABOUT_H
#define ABOUT_H

/* Fenetre "A propos / Aide" : s'affiche en plein ecran, ferme par Echap, Entree,
   Espace ou un clic sur le bouton X. La touche L change de langue sur place.
   Retourne une fois la fenetre fermee et l'ecran efface (a redessiner). */
void showAbout();

/* Petite fenetre "Quitter le jeu ?" affichee par-dessus le jeu.
   Retourne 1 si le joueur confirme (Y/O/J ou bouton Oui), 0 sinon (N, Echap, Entree,
   Espace ou bouton Non). Le centre de l'ecran doit ensuite etre redessine par l'appelant. */
int confirmQuit();

#endif