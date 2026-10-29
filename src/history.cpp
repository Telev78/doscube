#include <string.h>
#include "history.h"

struct Move
{
    signed char axis;
    signed char layer;
    signed char sign;
};

static Move moves[HISTORY_MAX];
static int count = 0; /* nombre de coups memorises                      */
static int pos = 0;   /* nombre de coups actuellement appliques (0..count) */

void histClear()
{
    count = 0;
    pos = 0;
}

void histPush(int axis, int layer, int sign)
{
    /* Nouveau coup apres un retour en arriere : les coups a refaire disparaissent */
    count = pos;

    /* Historique plein : on oublie le plus ancien coup */
    if (count == HISTORY_MAX)
    {
        memmove(&moves[0], &moves[1], (HISTORY_MAX - 1) * sizeof(Move));
        count--;
    }

    moves[count].axis = (signed char)axis;
    moves[count].layer = (signed char)layer;
    moves[count].sign = (signed char)sign;
    count++;
    pos = count;
}

int histCanUndo()
{
    return pos > 0;
}

int histCanRedo()
{
    return pos < count;
}

int histPeekUndo(int &axis, int &layer, int &sign)
{
    if (pos <= 0)
        return 0;
    axis = moves[pos - 1].axis;
    layer = moves[pos - 1].layer;
    sign = -moves[pos - 1].sign; /* l'inverse du coup joue */
    return 1;
}

int histPeekRedo(int &axis, int &layer, int &sign)
{
    if (pos >= count)
        return 0;
    axis = moves[pos].axis;
    layer = moves[pos].layer;
    sign = moves[pos].sign;
    return 1;
}

void histCommitUndo()
{
    if (pos > 0)
        pos--;
}

void histCommitRedo()
{
    if (pos < count)
        pos++;
}