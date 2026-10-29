#ifndef TIMER_H
#define TIMER_H

/* Chrono de precision (milliseconde) base sur le PIT 8253/8254, canal 0.
   Le compteur de ticks du BIOS (18,2 Hz) fournit les ticks entiers, le compteur
   du PIT fournit la fraction de tick (resolution d'environ 0,84 microseconde). */

void timerInit();    /* a appeler une fois au demarrage                       */
void timerRestore(); /* remet le PIT dans son mode d'origine (via atexit)     */

void timerReset();  /* remet a zero, chrono arrete                           */
void timerStart();  /* demarre a partir de zero                              */
void timerStop();   /* fige le temps ecoule                                  */
int timerRunning(); /* 1 si le chrono tourne                                 */

unsigned long timerElapsedMs();               /* temps ecoule en millisecondes     */
void formatTime(char *buf, unsigned long ms); /* "mm:ss,mmm" (buf >= 10 octets) */

#endif