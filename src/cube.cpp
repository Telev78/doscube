#include "cube.h"
#include <stdlib.h>

extern const int faceAxis[6] = {1, 1, 0, 0, 2, 2};
extern const int faceSign[6] = {1, -1, -1, 1, 1, -1};

static int coordOf(const Point3D &p, int axis)
{
    if (axis == 0)
        return p.x;
    if (axis == 1)
        return p.y;
    return p.z;
}

static void setCoord(Point3D &p, int axis, int value)
{
    if (axis == 0)
        p.x = value;
    else if (axis == 1)
        p.y = value;
    else
        p.z = value;
}

// Indice de la face (0..5) dont la normale est +axe (sign > 0) ou -axe (sign < 0)
static int faceIndex(int axis, int sign)
{
    static const int idx[3][2] = {{3, 2},  // X : Droite, Gauche
                                  {0, 1},  // Y : Haut, Bas
                                  {4, 5}}; // Z : Avant, Arrière
    return idx[axis][sign > 0 ? 0 : 1];
}

// Initialise une pièce à la position (x, y, z) : une face colorée par coordonnée à ±1
static void setupPiece(Cubie &p, int x, int y, int z)
{
    int f;
    p.currentPos.x = x;
    p.currentPos.y = y;
    p.currentPos.z = z;

    for (f = 0; f < 6; f++)
        p.faceColors[f] = COLOR_NONE;

    if (y == 1)
        p.faceColors[0] = COLOR_WHITE; // Haut
    if (y == -1)
        p.faceColors[1] = COLOR_YELLOW; // Bas
    if (x == -1)
        p.faceColors[2] = COLOR_ORANGE; // Gauche
    if (x == 1)
        p.faceColors[3] = COLOR_RED; // Droite
    if (z == 1)
        p.faceColors[4] = COLOR_GREEN; // Avant
    if (z == -1)
        p.faceColors[5] = COLOR_BLUE; // Arrière
}

void initCube(RubikCube &cube)
{
    int x, y, z;
    int index = 0;

    cube.moveCount = 0;

    for (x = -1; x <= 1; x++)
        for (y = -1; y <= 1; y++)
            for (z = -1; z <= 1; z++)
            {
                if (x == 0 && y == 0 && z == 0)
                    continue; // pas de pièce au centre du cube
                setupPiece(cube.pieces[index], x, y, z);
                index++;
            }
}

/* Rotation de 90° d'une tranche (règle de la main droite autour de +axis si sign > 0).
   Pour l'axe a, on note u et v les deux autres axes, de sorte que (a, u, v) soit une
   permutation circulaire de (X, Y, Z). Une rotation de +90° envoie :
     - la position (u, v) sur (-v, u) ;
     - la face +u sur +v, +v sur -u, -u sur -v et -v sur +u (anneau ring[0..3]).
   Chaque pièce ne dépend que de son propre état : la mise à jour se fait sur place. */
void rotateLayer(RubikCube &cube, int axis, int layer, int sign)
{
    int u = (axis + 1) % 3;
    int v = (axis + 2) % 3;
    int dir = (sign > 0) ? 1 : 3; // avance de 1 ou de -1 dans l'anneau
    int ring[4];
    int i, k, pu, pv;
    unsigned char old[6];
    Cubie *p;

    ring[0] = faceIndex(u, 1);
    ring[1] = faceIndex(v, 1);
    ring[2] = faceIndex(u, -1);
    ring[3] = faceIndex(v, -1);

    for (i = 0; i < 26; i++)
    {
        p = &cube.pieces[i];
        if (coordOf(p->currentPos, axis) != layer)
            continue;

        pu = coordOf(p->currentPos, u);
        pv = coordOf(p->currentPos, v);
        if (sign > 0)
        {
            setCoord(p->currentPos, u, -pv);
            setCoord(p->currentPos, v, pu);
        }
        else
        {
            setCoord(p->currentPos, u, pv);
            setCoord(p->currentPos, v, -pu);
        }

        for (k = 0; k < 6; k++)
            old[k] = p->faceColors[k];
        for (k = 0; k < 4; k++)
            p->faceColors[ring[(k + dir) & 3]] = old[ring[k]];
    }
}

// 1 si chacune des 6 faces n'a qu'une seule couleur
// (fonctionne même si les tranches centrales ont déplacé les centres)
int isSolved(const RubikCube &cube)
{
    int f, i, coord, ref, c;
    const Point3D *p;

    for (f = 0; f < 6; f++)
    {
        ref = -1;
        for (i = 0; i < 26; i++)
        {
            p = &cube.pieces[i].currentPos;
            coord = coordOf(*p, faceAxis[f]);
            if (coord != faceSign[f])
                continue;
            c = cube.pieces[i].faceColors[f];
            if (c == COLOR_NONE)
                return 0;
            if (ref < 0)
                ref = c;
            else if (c != ref)
                return 0;
        }
    }
    return 1;
}

// Tire une tranche au hasard (face extérieure ou tranche centrale) et un sens,
// en évitant de retomber sur la même tranche que le coup précédent
void randomMove(int lastAxis, int lastLayer, int &axis, int &layer, int &sign)
{
    do
    {
        axis = rand() % 3;
        layer = (rand() % 3) - 1; // -1, 0 ou 1
    } while (axis == lastAxis && layer == lastLayer);
    sign = (rand() % 2) ? 1 : -1;
}