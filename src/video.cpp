#include <dos.h>
#include "video.h"

/* Tableau de 64 000 octets en memoire lointaine (far) : tient dans un segment de 64 Ko */
unsigned char far backBuffer[320UL * 200UL];

static int savedMode = 3;  /* mode video d'origine, a restaurer en sortant */
static int modeActive = 0; /* 1 une fois le mode 13h active                */

int getVideoMode()
{
    _AH = 0x0F;
    geninterrupt(0x10);
    return _AL & 0x7F;
}

/* La fonction BIOS 1A00h (lecture du code de combinaison d'affichage) n'existe que
   sur VGA/MCGA : elle renvoie AL = 1Ah. BL donne le type de l'affichage actif :
   07h/08h = VGA, 0Ah a 0Ch = MCGA. */
int detectVGA()
{
    union REGS r;
    r.x.ax = 0x1A00;
    r.x.bx = 0;
    int86(0x10, &r, &r);
    if (r.h.al != 0x1A)
        return 0; /* fonction absente : EGA, CGA, MDA... */
    return (r.h.bl == 0x07 || r.h.bl == 0x08 ||
            (r.h.bl >= 0x0A && r.h.bl <= 0x0C));
}

int initMode13h()
{
    savedMode = getVideoMode();
    _AX = 0x0013;
    geninterrupt(0x10);
    modeActive = (getVideoMode() == 0x13);
    return modeActive;
}

void closeMode13h()
{
    if (modeActive)
    {
        modeActive = 0;
        _AX = savedMode; /* AH = 0 (fixer le mode), AL = ancien mode */
        geninterrupt(0x10);
    }
}

void waitVSync()
{
    unsigned long far *ticks = (unsigned long far *)MK_FP(0x40, 0x6C);
    unsigned long t0 = *ticks;

    while (inportb(0x3DA) & 0x08)
        if (*ticks - t0 > 2)
            return;
    while (!(inportb(0x3DA) & 0x08))
        if (*ticks - t0 > 2)
            return;
}