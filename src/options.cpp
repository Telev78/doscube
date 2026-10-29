#include <stdio.h>
#include <string.h>
#include "lang.h"
#include "options.h"
#include "cpu.h"

Options options = {CPU_AUTO, CPU_CLASS_386};

static char badArg[64] = "";

static int isOptionChar(char c)
{
    return c == '/' || c == '-';
}

static int isHelpArg(const char *a)
{
    return isOptionChar(a[0]) &&
           ((a[1] == '?' && a[2] == 0) ||
            ((a[1] == 'H' || a[1] == 'h') && a[2] == 0));
}

static int rejectArg(const char *a)
{
    strncpy(badArg, a, sizeof(badArg) - 1);
    badArg[sizeof(badArg) - 1] = 0;
    return OPT_ERROR;
}

int parseOptions(int argc, char *argv[])
{
    int i;
    char *a;

    options.cpuMode = CPU_AUTO;
    badArg[0] = 0;

    /* L'aide a la priorite sur tout le reste */
    for (i = 1; i < argc; i++)
        if (isHelpArg(argv[i]))
            return OPT_HELP;

    for (i = 1; i < argc; i++)
    {
        a = argv[i];

        if (!isOptionChar(a[0]) || a[1] == 0 || a[2] != ':')
            return rejectArg(a);

        if (a[1] == 'L' || a[1] == 'l')
        {
            /* la langue elle-meme est choisie par initLanguage() ; ici on valide */
            if (stricmp(a + 3, "EN") != 0 && stricmp(a + 3, "FR") != 0 &&
                stricmp(a + 3, "DE") != 0)
                return rejectArg(a);
        }
        else if (a[1] == 'C' || a[1] == 'c')
        {
            if (stricmp(a + 3, "AUTO") == 0)
                options.cpuMode = CPU_AUTO;
            else if (stricmp(a + 3, "SLOW") == 0)
                options.cpuMode = CPU_SLOW;
            else if (stricmp(a + 3, "FAST") == 0)
                options.cpuMode = CPU_FAST;
            else
                return rejectArg(a);
        }
        else
            return rejectArg(a);
    }
    return OPT_OK;
}

void resolveCpuMode()
{
    options.cpuClass = detectCpuClass();
    if (options.cpuMode == CPU_AUTO)
        options.cpuMode = (options.cpuClass <= CPU_CLASS_286) ? CPU_SLOW : CPU_FAST;
}

const char *badOption()
{
    return badArg;
}

void printHelp()
{
    printf("%s\n\n", T(S_HELP_TITLE));
    printf(T(S_HELP_USAGE), PROGRAM_NAME);
    printf("\n\n");
    printf("%s\n", T(S_HELP_OPT_L));
    printf("%s\n", T(S_HELP_OPT_C));
    printf("%s\n\n", T(S_HELP_OPT_HELP));
    printf("%s\n", T(S_HELP_KEYS));
    printf("%s\n", T(S_HELP_MOUSE));
}