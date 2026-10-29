#include <dos.h>
#include <string.h>
#include "lang.h"

/* Les caracteres accentues sont ecrits en octal : les codes 128..255 sont
   identiques en pages de codes 437 et 850, et le fichier source reste en ASCII
   pur (aucun probleme d'encodage selon l'editeur utilise).
   \201 u-trema   \202 e-aigu   \204 a-trema   \205 a-grave   \207 c-cedille
   \224 o-trema   \341 eszett                                              */

static const char *const strings[LANG_COUNT][S_COUNT] = {
    /* ---- English ---- */
    {
        "Error: a VGA card is required (mode 13h, 320x200x256).",
        "Error: unable to set graphics mode 13h.",
        "Error: mouse not detected!",
        "Error: a 2-button mouse is required.",
        "Moves: %u",
        "Solved in %u moves!",
        "Shuffling...",
        "Shuffle",
        "Unknown or invalid option: %s",
        "Use /? for the list of options.",
        "Rubik's Cube for DOS",
        "Usage: %s [/L:EN|FR|DE] [/C:AUTO|SLOW|FAST] [/?]",
        "  /L:xx   Language: EN, FR or DE (default: from the DOS country code)",
        "  /C:xx   CPU: AUTO (default), SLOW = shorter animations, FAST",
        "  /?      Show this help",
        "Keys: M shuffle, U/<- undo, R/-> redo, L language, F1 help, Esc quit",
        "Mouse: right button turns the view, drag on a face with the left button",
        "Version %s",
        "Mouse",
        "Right + drag : rotate the view",
        "Left + drag on a face : a layer",
        "Keyboard",
        "M shuffle   U or <- undo",
        "R or -> redo   L language",
        "F1 or ? help   Esc quit",
        "Esc / Enter : close",
        "Quit the game?",
        "A game is in progress.",
        "Yes (Y)",
        "No (N)"},
    /* ---- Francais ---- */
    {
        "Erreur : une carte VGA est n\202cessaire (mode 13h, 320x200x256).",
        "Erreur : impossible d'activer le mode graphique 13h.",
        "Erreur : souris non d\202tect\202e !",
        "Erreur : souris \205 2 boutons requise.",
        "Coups : %u",
        "R\202solu en %u coups !",
        "M\202lange...",
        "M\202langer",
        "Option inconnue ou invalide : %s",
        "Utilisez /? pour la liste des options.",
        "Rubik's Cube pour DOS",
        "Usage : %s [/L:EN|FR|DE] [/C:AUTO|SLOW|FAST] [/?]",
        "  /L:xx   Langue : EN, FR ou DE (d\202faut : selon le code pays DOS)",
        "  /C:xx   CPU : AUTO (d\202faut), SLOW = animations courtes, FAST",
        "  /?      Affiche cette aide",
        "Touches : M m\202langer, U/<- annuler, R/-> refaire, L langue, F1 aide, Echap",
        "Souris : clic droit tourne la vue, glisser sur une face tourne une tranche",
        "Version %s",
        "Souris",
        "Clic droit + glisser : la vue",
        "Clic gauche + glisser : tranche",
        "Clavier",
        "M m\202langer   U ou <- annuler",
        "R ou -> refaire   L langue",
        "F1 ou ? aide   Echap quitter",
        "Echap / Entr\202e : fermer",
        "Quitter le jeu ?",
        "Une partie est en cours.",
        "Oui (O)",
        "Non (N)"},
    /* ---- Deutsch ---- */
    {
        "Fehler: Eine VGA-Karte ist erforderlich (Modus 13h, 320x200x256).",
        "Fehler: Grafikmodus 13h kann nicht aktiviert werden.",
        "Fehler: Maus nicht erkannt!",
        "Fehler: Maus mit 2 Tasten erforderlich.",
        "Z\201ge: %u",
        "Gel\224st in %u Z\201gen!",
        "Mischen...",
        "Mischen",
        "Unbekannte oder ung\201ltige Option: %s",
        "Mit /? erhalten Sie die Liste der Optionen.",
        "Rubik's Cube f\201r DOS",
        "Aufruf: %s [/L:EN|FR|DE] [/C:AUTO|SLOW|FAST] [/?]",
        "  /L:xx   Sprache: EN, FR oder DE (Standard: nach DOS-L\204ndercode)",
        "  /C:xx   CPU: AUTO (Standard), SLOW = kurze Animationen, FAST",
        "  /?      Diese Hilfe anzeigen",
        "Tasten: M mischen, U/<- zur\201ck, R/-> vor, L Sprache, F1 Hilfe, Esc",
        "Maus: rechts = Ansicht drehen, links auf einer Fl\204che ziehen = Schicht drehen",
        "Version %s",
        "Maus",
        "Rechts + ziehen : Ansicht drehen",
        "Links + Fl\204che ziehen : Schicht",
        "Tastatur",
        "M mischen   U oder <- zur\201ck",
        "R oder -> vor   L Sprache",
        "F1 oder ? Hilfe   Esc beenden",
        "Esc / Enter : schlie\341en",
        "Spiel beenden?",
        "Ein Spiel l\204uft noch.",
        "Ja (J)",
        "Nein (N)"}};

static int currentLang = LANG_EN;

/* Code pays DOS courant (INT 21h, AH=38h, AL=0).
   Retourne -1 en cas d'erreur. Le code pays est renvoye dans BX (DOS 3.0+). */
static int getCountryCode()
{
    static unsigned char buf[40]; /* 34 octets necessaires */
    union REGS r;
    struct SREGS s;

    segread(&s);
    r.x.ax = 0x3800;
    r.x.dx = FP_OFF((void far *)buf);
    s.ds = FP_SEG((void far *)buf);
    int86x(0x21, &r, &r, &s);

    if (r.x.cflag)
        return -1;
    return r.x.bx;
}

static int langFromCountry(int code)
{
    switch (code)
    {
    case 2:  /* Canada francais */
    case 32: /* Belgique        */
    case 33: /* France          */
        return LANG_FR;

    case 41: /* Suisse (multilingue : touche L pour changer) */
    case 43: /* Autriche        */
    case 49: /* Allemagne       */
        return LANG_DE;

    default:
        return LANG_EN;
    }
}

void initLanguage(int argc, char *argv[])
{
    int i;
    int code;

    /* 1. Option explicite : /L:FR  /L:DE  /L:EN (ou -L:xx) */
    for (i = 1; i < argc; i++)
    {
        if ((argv[i][0] == '/' || argv[i][0] == '-') &&
            (argv[i][1] == 'L' || argv[i][1] == 'l') && argv[i][2] == ':')
        {
            if (stricmp(argv[i] + 3, "FR") == 0)
            {
                currentLang = LANG_FR;
                return;
            }
            if (stricmp(argv[i] + 3, "DE") == 0)
            {
                currentLang = LANG_DE;
                return;
            }
            if (stricmp(argv[i] + 3, "EN") == 0)
            {
                currentLang = LANG_EN;
                return;
            }
        }
    }

    /* 2. Code pays DOS */
    code = getCountryCode();
    if (code >= 0)
        currentLang = langFromCountry(code);
    else
        currentLang = LANG_EN;
}

void nextLanguage()
{
    currentLang = (currentLang + 1) % LANG_COUNT;
}

const char *T(int id)
{
    if (id < 0 || id >= S_COUNT)
        return "";
    return strings[currentLang][id];
}