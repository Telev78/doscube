#include <dos.h>
#include <stdio.h>
#include "timer.h"

/* 1 : precision a la milliseconde (reprogramme le canal 0 du PIT en mode 2)
   0 : ticks du BIOS seuls (resolution de 55 ms, aucun acces au materiel)     */
#define TIMER_USE_PIT 1

#define TICKS_PER_DAY 1573040UL /* le compteur BIOS revient a 0 a minuit */
#define PIT_HZ 1193182L

static int pitProgrammed = 0;
static int running = 0;
static unsigned long startTicks = 0;
static unsigned startFrac = 0;
static unsigned long frozenMs = 0;

/* ------------------------------------------------------------------ */
/* Lecture de l'horloge : ticks BIOS + fraction de tick (0..65535)    */
/* ------------------------------------------------------------------ */

static void readClock(unsigned long &ticks, unsigned &frac)
{
#if TIMER_USE_PIT
    unsigned lo, hi, count;
    unsigned char irr;

    disable();
    outportb(0x43, 0x00); /* verrouille le compteur du canal 0    */
    lo = inportb(0x40);
    hi = inportb(0x40);
    outportb(0x20, 0x0A); /* prochaine lecture du 8259 = IRR      */
    irr = inportb(0x20);
    ticks = *(unsigned long far *)MK_FP(0x40, 0x6C);
    enable();

    count = (hi << 8) | lo;        /* mode 2 : decompte de 65536 a 1        */
    frac = (unsigned)(0U - count); /* 65536 - count (0 juste apres reload)  */

    /* Le compteur vient de recharger mais l'IRQ0 n'a pas encore ete traitee :
       le tick BIOS a du retard d'une unite. */
    if ((irr & 0x01) && count > 0x8000U)
        ticks++;
#else
    ticks = *(unsigned long far *)MK_FP(0x40, 0x6C);
    frac = 0;
#endif
}

/* Difference en millisecondes entre deux lectures (gere le passage de minuit) */
static unsigned long diffMs(unsigned long t0, unsigned f0, unsigned long t1, unsigned f1)
{
    unsigned long dt;
    long ms, fr;

    if (t1 < t0)
        t1 += TICKS_PER_DAY;
    dt = t1 - t0;
    if (dt > 200000UL) /* plus de ~3 heures : on plafonne */
        dt = 200000UL;

    /* 1 tick = 65536 / 1193182 s = 54,9254 ms */
    ms = (long)(dt * 54UL + (dt * 9254UL) / 10000UL);
    fr = ((long)f1 - (long)f0) * 1000L / PIT_HZ; /* -55..+55 ms */
    ms += fr;
    if (ms < 0)
        ms = 0;
    return (unsigned long)ms;
}

/* ------------------------------------------------------------------ */
/* Programmation du PIT                                               */
/* ------------------------------------------------------------------ */

void timerInit()
{
#if TIMER_USE_PIT
    /* Mode 2 (generateur de taux), compteur de 65536 : meme frequence d'IRQ0
       que le BIOS (18,2 Hz), mais le compteur decroit de 1 en 1 et ne boucle
       qu'une fois par tick (en mode 3 il boucle deux fois : ambigu). */
    disable();
    outportb(0x43, 0x34); /* canal 0, lo/hi, mode 2, binaire */
    outportb(0x40, 0x00);
    outportb(0x40, 0x00);
    enable();
    pitProgrammed = 1;
#endif
}

void timerRestore()
{
#if TIMER_USE_PIT
    if (pitProgrammed)
    {
        pitProgrammed = 0;
        disable();
        outportb(0x43, 0x36); /* canal 0, lo/hi, mode 3 (mode du BIOS) */
        outportb(0x40, 0x00);
        outportb(0x40, 0x00);
        enable();
    }
#endif
}

/* ------------------------------------------------------------------ */
/* Interface du chrono                                                */
/* ------------------------------------------------------------------ */

void timerReset()
{
    running = 0;
    frozenMs = 0;
}

void timerStart()
{
    readClock(startTicks, startFrac);
    frozenMs = 0;
    running = 1;
}

unsigned long timerElapsedMs()
{
    unsigned long t;
    unsigned f;
    if (!running)
        return frozenMs;
    readClock(t, f);
    return diffMs(startTicks, startFrac, t, f);
}

void timerStop()
{
    if (running)
    {
        frozenMs = timerElapsedMs();
        running = 0;
    }
}

int timerRunning()
{
    return running;
}

void formatTime(char *buf, unsigned long ms)
{
    unsigned minutes, seconds, millis;

    if (ms > 5999999UL) /* 99:59,999 maximum */
        ms = 5999999UL;
    minutes = (unsigned)(ms / 60000UL);
    seconds = (unsigned)((ms / 1000UL) % 60UL);
    millis = (unsigned)(ms % 1000UL);
    sprintf(buf, "%02u:%02u,%03u", minutes, seconds, millis);
}