#ifndef LANG_H
#define LANG_H

/* Langues disponibles (la premiere est la langue par defaut) */
enum Lang
{
    LANG_EN = 0,
    LANG_FR,
    LANG_DE,
    LANG_COUNT
};

/* Identifiants des textes affiches */
enum StrId
{
    S_ERR_NO_VGA = 0,
    S_ERR_MODE13,
    S_ERR_NO_MOUSE,
    S_ERR_MOUSE_BUTTONS,
    S_MOVES,  /* format : un %u (nombre de coups)          */
    S_SOLVED, /* format : un %u (nombre de coups)          */
    S_SHUFFLING,
    S_BTN_SHUFFLE,
    S_COUNT
};

/* Choix de la langue : option /L:EN, /L:FR ou /L:DE sur la ligne de commande,
   sinon code pays DOS (INT 21h, fonction 38h), sinon anglais */
void initLanguage(int argc, char *argv[]);

/* Passe a la langue suivante (EN -> FR -> DE -> EN ...) */
void nextLanguage();

/* Retourne le texte dans la langue courante */
const char *T(int id);

#endif