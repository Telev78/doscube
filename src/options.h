#ifndef OPTIONS_H
#define OPTIONS_H

/* Identite du programme */
#define APP_NAME "DosCube"
#define APP_VERSION "1.1"
#define APP_AUTHOR "(c) Telev"

/* Nom du programme affiche dans l'aide (adapter au nom de l'executable) */
#define PROGRAM_NAME "DOSCUBE"

/* Vitesse du processeur : AUTO (defaut, detection), FAST ou SLOW (286...) */
enum CpuMode
{
    CPU_AUTO = 0,
    CPU_FAST,
    CPU_SLOW
};

struct Options
{
    int cpuMode;  /* apres resolveCpuMode() : CPU_FAST ou CPU_SLOW */
    int cpuClass; /* classe detectee (voir cpu.h)                   */
};

extern Options options;

/* Resultats de parseOptions() */
#define OPT_OK 0
#define OPT_HELP 1
#define OPT_ERROR 2

/* Analyse la ligne de commande :
     /L:EN|FR|DE     langue (traitee par initLanguage)
     /C:AUTO|SLOW|FAST   vitesse du processeur (AUTO = detection)
     /?  -?  /H      aide
   Retourne OPT_OK, OPT_HELP ou OPT_ERROR (option inconnue ou valeur invalide). */
int parseOptions(int argc, char *argv[]);

/* Detecte le processeur et remplace AUTO par FAST ou SLOW (a appeler apres parseOptions) */
void resolveCpuMode();

/* Premiere option fautive rencontree (pour le message d'erreur) */
const char *badOption();

/* Affiche l'aide dans la langue courante */
void printHelp();

#endif