#ifndef VIDEO_H
#define VIDEO_H

/* Tampon d'image (back-buffer) du mode 13h : 320 x 200 = 64 000 octets, alloue en
   memoire lointaine (far) pour eviter tout depassement de segment. Defini dans video.cpp. */
extern unsigned char far backBuffer[];

/* Numero du mode video courant (INT 10h, fonction 0Fh) */
int getVideoMode();

/* 1 si une carte VGA (ou MCGA) est presente, 0 sinon (EGA, CGA, MDA...) */
int detectVGA();

/* Passe en mode 13h (320x200, 256 couleurs) et verifie que le BIOS l'a accepte.
   Memorise le mode d'origine. Retourne 1 si le mode est actif. */
int initMode13h();

/* Restaure le mode video d'origine (sans effet si le mode 13h n'a jamais ete active) */
void closeMode13h();

/* Attend le prochain retour de balayage vertical de l'ecran */
void waitVSync();

#endif