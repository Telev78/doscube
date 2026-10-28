#include <math.h>
#include "cube.h"
#include "renderer.h"

static const int faceAxis[6] = {1, 1, 0, 0, 2, 2};    // axe de la normale de chaque face
static const int faceSign[6] = {1, -1, -1, 1, 1, -1}; // son sens

static int levi(int i, int j, int k) // signe de la permutation (i,j,k) de {0,1,2}
{
    return (((j - i + 3) % 3) == 1 && ((k - j + 3) % 3) == 1) ? 1 : -1;
}

int computeMove(const RubikCube &cube, int owner, int f, int dx, int dy,
                int angleX, int angleY, int &axis, int &layer, int &sign)
{
    int n = faceAxis[f], ns = faceSign[f];
    int pos[3], v[3], t, a = -1, b;
    double best = -1.0, sx, sy, len, d, dirSign = 1.0;

    pos[0] = cube.pieces[owner].currentPos.x;
    pos[1] = cube.pieces[owner].currentPos.y;
    pos[2] = cube.pieces[owner].currentPos.z;

    for (t = 0; t < 3; t++)
    {
        if (t == n)
            continue;
        v[0] = v[1] = v[2] = 0;
        v[t] = 1;
        projectDir(v[0], v[1], v[2], angleX, angleY, sx, sy);
        len = sqrt(sx * sx + sy * sy);
        if (len < 0.2)
            continue; // axe quasi vu de bout : direction peu fiable
        d = (dx * sx + dy * sy) / len;
        if (fabs(d) > best)
        {
            best = fabs(d);
            a = t;
            dirSign = (d > 0) ? 1.0 : -1.0;
        }
    }
    if (a < 0)
        return 0;

    b = 3 - n - a; // axe de rotation = l'autre axe tangent
    layer = pos[b];
    axis = b;
    // vitesse = ? × r  =>  s = dirSign * ns * ?(b, n, a)
    sign = (int)dirSign * ns * levi(b, n, a);
    return 1;
}