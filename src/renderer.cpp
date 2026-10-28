#include <math.h>
#include <mem.h>
#include <dos.h>
#include "renderer.h"

/* Tampon partage, defini dans main.cpp */
extern unsigned char far backBuffer[];

#define SPACING 28 /* distance entre deux cubies (pixels)            */
#define HS 12      /* demi-taille d'un autocollant (pixels)          */
#define CENTER_X 160
#define CENTER_Y 100
#define MAX_FACES 130

/* ------------------------------------------------------------------ */
/* Structures et donnees                                              */
/* ------------------------------------------------------------------ */

struct Face3D
{
    unsigned char color;
    int z;       /* profondeur du centre de la face (8.8)  */
    int owner;   /* index du cubie                         */
    int faceIdx; /* 0..5 : Haut, Bas, Gauche, Droite, ...  */
    int pts[8];  /* 4 sommets ecran                        */
};

/* Vecteurs des axes du modele, projetes pour une vue donnee (tout en 8.8) */
struct ViewVecs
{
    int ux[3], uy[3], uz[3]; /* axes * SPACING (x ecran, y ecran, profondeur) */
    int hx[3], hy[3], hz[3]; /* axes * HS                                     */
    int vis[6];              /* face visible (1) ou cachee (0)                */
};

static Face3D facesToDraw[MAX_FACES];
static int order[MAX_FACES];
static int faceCount = 0;

/* Axe (0=X 1=Y 2=Z) et sens de la normale de chaque face */
static const signed char faceAxis[6] = {1, 1, 0, 0, 2, 2};
static const signed char faceSign[6] = {1, -1, -1, 1, 1, -1};

static const signed char faceTemplates[6][4][3] = {
    {{-1, 1, -1}, {1, 1, -1}, {1, 1, 1}, {-1, 1, 1}},     /* 0: Haut    */
    {{-1, -1, -1}, {-1, -1, 1}, {1, -1, 1}, {1, -1, -1}}, /* 1: Bas     */
    {{-1, -1, -1}, {-1, -1, 1}, {-1, 1, 1}, {-1, 1, -1}}, /* 2: Gauche  */
    {{1, -1, -1}, {1, 1, -1}, {1, 1, 1}, {1, -1, 1}},     /* 3: Droite  */
    {{-1, -1, 1}, {1, -1, 1}, {1, 1, 1}, {-1, 1, 1}},     /* 4: Avant   */
    {{1, -1, -1}, {1, 1, -1}, {-1, 1, -1}, {-1, -1, -1}}  /* 5: Arriere */
};

unsigned rowOffset[200]; /* rowOffset[y] = y * 320 */

/* ------------------------------------------------------------------ */
/* Trigonometrie en point fixe (table Q14 de 360 entrees)              */
/* ------------------------------------------------------------------ */

static int sinTab[360];

static void initTrig()
{
    int a;
    int q[91];

    /* Seulement 91 appels a sin() : le reste vient des symetries */
    for (a = 0; a <= 90; a++)
        q[a] = (int)(sin(a * 3.14159265 / 180.0) * 16384.0 + 0.5);

    for (a = 0; a < 360; a++)
    {
        if (a <= 90)
            sinTab[a] = q[a];
        else if (a <= 180)
            sinTab[a] = q[180 - a];
        else if (a <= 270)
            sinTab[a] = -q[a - 180];
        else
            sinTab[a] = -q[360 - a];
    }
}

/* Version Q14 (1.0 = 16384) */
static int sinQ(int a)
{
    a %= 360;
    if (a < 0)
        a += 360;
    return sinTab[a];
}

static int cosQ(int a)
{
    return sinQ(a + 90);
}

/* Version 8.8 (1.0 = 256) */
static int sinI(int a)
{
    return (sinQ(a) + 32) >> 6;
}

static int cosI(int a)
{
    return (cosQ(a) + 32) >> 6;
}

void initRenderer()
{
    int y;
    for (y = 0; y < 200; y++)
        rowOffset[y] = (unsigned)y * 320U;
    initTrig();
    resetView(30, 315);
}

/* ------------------------------------------------------------------ */
/* Couleurs                                                           */
/* ------------------------------------------------------------------ */

static unsigned char getMode13Color(unsigned char cubeColor)
{
    switch (cubeColor)
    {
    case COLOR_WHITE:
        return 15;
    case COLOR_YELLOW:
        return 14;
    case COLOR_RED:
        return 4;
    case COLOR_ORANGE:
        return 41;
    case COLOR_BLUE:
        return 1;
    case COLOR_GREEN:
        return 2;
    default:
        return 0;
    }
}

/* ------------------------------------------------------------------ */
/* Matrices 3x3 en point fixe 8.8                                     */
/* ------------------------------------------------------------------ */

/* Matrice de vue : rotation Y puis rotation X (comme l'ancien code flottant) */
static void buildViewMatrix(int angleX, int angleY, int m[3][3])
{
    int cX = cosI(angleX), sX = sinI(angleX);
    int cY = cosI(angleY), sY = sinI(angleY);

    m[0][0] = cY;
    m[0][1] = 0;
    m[0][2] = -sY;

    m[1][0] = (int)(-((long)sY * sX) >> 8);
    m[1][1] = cX;
    m[1][2] = (int)(-((long)cY * sX) >> 8);

    m[2][0] = (int)(((long)sY * cX) >> 8);
    m[2][1] = sX;
    m[2][2] = (int)(((long)cY * cX) >> 8);
}

/* Rotation d'une couche : +deg degres (regle de la main droite) autour de l'axe */
static void buildLayerMatrix(int axis, int deg, int r[3][3])
{
    int c = cosI(deg), s = sinI(deg);
    int i, j;

    for (i = 0; i < 3; i++)
        for (j = 0; j < 3; j++)
            r[i][j] = (i == j) ? 256 : 0;

    if (axis == 0)
    {
        r[1][1] = c;
        r[1][2] = -s;
        r[2][1] = s;
        r[2][2] = c;
    }
    else if (axis == 1)
    {
        r[0][0] = c;
        r[0][2] = s;
        r[2][0] = -s;
        r[2][2] = c;
    }
    else
    {
        r[0][0] = c;
        r[0][1] = -s;
        r[1][0] = s;
        r[1][1] = c;
    }
}

static void mulMat(int a[3][3], int b[3][3], int out[3][3])
{
    int i, j, k;
    long sum;
    for (i = 0; i < 3; i++)
        for (j = 0; j < 3; j++)
        {
            sum = 0;
            for (k = 0; k < 3; k++)
                sum += (long)a[i][k] * b[k][j];
            out[i][j] = (int)(sum >> 8);
        }
}

/* Transforme la matrice en vecteurs prets a l'emploi + visibilite des 6 faces.
   Bornes : chaque coordonnee reste < 31000 (tient dans un int 16 bits). */
static void buildVecs(int m[3][3], ViewVecs &v)
{
    int k, f, my;
    for (k = 0; k < 3; k++)
    {
        my = m[1][k] * 5 / 6; /* correction d'aspect 4/3 */
        v.ux[k] = m[0][k] * SPACING;
        v.uy[k] = my * SPACING;
        v.uz[k] = m[2][k] * SPACING;
        v.hx[k] = m[0][k] * HS;
        v.hy[k] = my * HS;
        v.hz[k] = m[2][k] * HS;
    }
    for (f = 0; f < 6; f++)
        v.vis[f] = (faceSign[f] * m[2][faceAxis[f]] > 0);
}

/* ------------------------------------------------------------------ */
/* Orientation de la vue : matrice cumulee en Q14 (trackball)         */
/* Un glissement horizontal tourne toujours autour de l'axe vertical  */
/* de l'ECRAN, un glissement vertical autour de son axe horizontal,   */
/* quelle que soit l'orientation actuelle du cube.                    */
/* ------------------------------------------------------------------ */

static int viewM[3][3]; /* Q14 : 1.0 = 16384 */

static void mulQ14(int a[3][3], int b[3][3], int out[3][3])
{
    int i, j, k;
    long sum;
    for (i = 0; i < 3; i++)
        for (j = 0; j < 3; j++)
        {
            sum = 0;
            for (k = 0; k < 3; k++)
                sum += (long)a[i][k] * b[k][j];
            out[i][j] = (int)((sum + 8192L) >> 14);
        }
}

/* Une iteration de Newton : M = 1.5*M - 0.5*(M*Mt)*M
   ramene la matrice vers une vraie rotation (evite la derive d'arrondi) */
static void orthoViewM()
{
    int a[3][3], t[3][3];
    int i, j, k;
    long sum;

    for (i = 0; i < 3; i++)
        for (j = 0; j < 3; j++)
        {
            sum = 0;
            for (k = 0; k < 3; k++)
                sum += (long)viewM[i][k] * viewM[j][k];
            a[i][j] = (int)((sum + 8192L) >> 14);
        }

    mulQ14(a, viewM, t);

    for (i = 0; i < 3; i++)
        for (j = 0; j < 3; j++)
            viewM[i][j] = (int)((3L * viewM[i][j] - t[i][j]) >> 1);
}

/* Vue initiale a partir de deux angles (degres) */
void resetView(int angleX, int angleY)
{
    int m[3][3];
    int i, j, n;

    buildViewMatrix(angleX, angleY, m);
    for (i = 0; i < 3; i++)
        for (j = 0; j < 3; j++)
            viewM[i][j] = m[i][j] << 6; /* 8.8 -> Q14 */
    for (n = 0; n < 3; n++)
        orthoViewM();
}

/* Glissement de souris : dx pixels vers la droite, dy pixels vers le bas.
   La face tournee vers nous suit le pointeur. */
void rotateView(int dx, int dy)
{
    int cy, sy, cx, sx;
    int ry[3][3], rx[3][3], d[3][3], n[3][3];
    int i, j;

    if (dx == 0 && dy == 0)
        return;

    cy = cosQ(dx);
    sy = sinQ(dx); /* rotation autour de l'axe Y de l'ecran */
    cx = cosQ(dy);
    sx = sinQ(dy); /* rotation autour de l'axe X de l'ecran */

    for (i = 0; i < 3; i++)
        for (j = 0; j < 3; j++)
        {
            ry[i][j] = (i == j) ? 16384 : 0;
            rx[i][j] = (i == j) ? 16384 : 0;
        }

    ry[0][0] = cy;
    ry[0][2] = sy;
    ry[2][0] = -sy;
    ry[2][2] = cy;

    rx[1][1] = cx;
    rx[1][2] = -sx;
    rx[2][1] = sx;
    rx[2][2] = cx;

    mulQ14(rx, ry, d);
    mulQ14(d, viewM, n);

    for (i = 0; i < 3; i++)
        for (j = 0; j < 3; j++)
            viewM[i][j] = n[i][j];

    orthoViewM();
}

/* Matrice de vue en 8.8 pour le rendu */
static void viewToMatrix(int m[3][3])
{
    int i, j;
    for (i = 0; i < 3; i++)
        for (j = 0; j < 3; j++)
            m[i][j] = (viewM[i][j] + 32) >> 6;
}

void projectDir(int ax, int ay, int az, double &sx, double &sy)
{
    int m[3][3];
    viewToMatrix(m);
    sx = (ax * m[0][0] + ay * m[0][1] + az * m[0][2]) / 256.0;
    sy = -(ax * m[1][0] + ay * m[1][1] + az * m[1][2]) / 256.0 * 5.0 / 6.0;
}

static int coordOf(const Point3D &p, int axis)
{
    if (axis == 0)
        return p.x;
    if (axis == 1)
        return p.y;
    return p.z;
}

/* ------------------------------------------------------------------ */
/* Selection a la souris                                              */
/* ------------------------------------------------------------------ */

static int pointInQuad(const int pts[8], int px, int py)
{
    int i, j, pos = 0, neg = 0;
    long cr;
    for (i = 0; i < 4; i++)
    {
        j = (i + 1) % 4;
        cr = (long)(pts[j * 2] - pts[i * 2]) * (py - pts[i * 2 + 1]) - (long)(pts[j * 2 + 1] - pts[i * 2 + 1]) * (px - pts[i * 2]);
        if (cr > 0)
            pos = 1;
        else if (cr < 0)
            neg = 1;
    }
    return !(pos && neg);
}

/* Les faces sont dessinees de l'arriere vers l'avant : on parcourt a l'envers */
int pickFace(int px, int py, int &owner, int &faceIdx)
{
    int i, idx;
    for (i = faceCount - 1; i >= 0; i--)
    {
        idx = order[i];
        if (pointInQuad(facesToDraw[idx].pts, px, py))
        {
            owner = facesToDraw[idx].owner;
            faceIdx = facesToDraw[idx].faceIdx;
            return 1;
        }
    }
    return 0;
}

/* ------------------------------------------------------------------ */
/* Remplissage de quadrilatere convexe                                */
/* ------------------------------------------------------------------ */

static int edgeL[200], edgeR[200];

static void scanEdge(int x1, int y1, int x2, int y2)
{
    long x, slope;
    int y, xi, t;
    if (y1 == y2)
        return;
    if (y1 > y2)
    {
        t = x1;
        x1 = x2;
        x2 = t;
        t = y1;
        y1 = y2;
        y2 = t;
    }
    slope = ((long)(x2 - x1) << 8) / (y2 - y1);
    x = (long)x1 << 8;
    for (y = y1; y < y2; y++)
    {
        if (y >= 0 && y < 200)
        {
            xi = (int)(x >> 8);
            if (xi < edgeL[y])
                edgeL[y] = xi;
            if (xi > edgeR[y])
                edgeR[y] = xi;
        }
        x += slope;
    }
}

static void fillQuad(int pts[8], unsigned char color)
{
    int minY = pts[1], maxY = pts[1];
    int i, y, l, r;

    for (i = 1; i < 4; i++)
    {
        if (pts[i * 2 + 1] < minY)
            minY = pts[i * 2 + 1];
        if (pts[i * 2 + 1] > maxY)
            maxY = pts[i * 2 + 1];
    }
    if (minY < 0)
        minY = 0;
    if (maxY > 199)
        maxY = 199;

    for (y = minY; y <= maxY; y++)
    {
        edgeL[y] = 32767;
        edgeR[y] = -32768;
    }

    for (i = 0; i < 4; i++)
        scanEdge(pts[i * 2], pts[i * 2 + 1],
                 pts[((i + 1) % 4) * 2], pts[((i + 1) % 4) * 2 + 1]);

    for (y = minY; y <= maxY; y++)
    {
        l = edgeL[y];
        r = edgeR[y];
        if (l > r)
            continue;
        if (l < 0)
            l = 0;
        if (r > 319)
            r = 319;
        if (l <= r)
            _fmemset((unsigned char far *)backBuffer + rowOffset[y] + l, color, r - l + 1);
    }
}

/* ------------------------------------------------------------------ */
/* Primitives d'affichage                                             */
/* ------------------------------------------------------------------ */

void fillRect(int x0, int y0, int x1, int y1, unsigned char color)
{
    int y;
    for (y = y0; y <= y1; y++)
        _fmemset((unsigned char far *)backBuffer + rowOffset[y] + x0, color, x1 - x0 + 1);
}

void clearRect(int x0, int y0, int x1, int y1)
{
    fillRect(x0, y0, x1, y1, 0);
}

/* Bouton en relief dans le back-buffer (texte centre, noir sur gris) */
void drawButton(int x0, int y0, int x1, int y1, const char *text, int pressed)
{
    int len = 0, tx, ty;
    const char *p = text;
    unsigned char light = pressed ? 8 : 15;
    unsigned char dark = pressed ? 15 : 8;

    fillRect(x0, y0, x1, y1, 7);     /* fond gris clair   */
    fillRect(x0, y0, x1, y0, light); /* bord haut         */
    fillRect(x0, y0, x0, y1, light); /* bord gauche       */
    fillRect(x0, y1, x1, y1, dark);  /* bord bas          */
    fillRect(x1, y0, x1, y1, dark);  /* bord droit        */

    while (*p++)
        len++;
    tx = x0 + ((x1 - x0 + 1) - len * 8) / 2 + (pressed ? 1 : 0);
    ty = y0 + ((y1 - y0 + 1) - 8) / 2 + (pressed ? 1 : 0);
    drawText(tx, ty, text, 0);
}

void blitRect(int x0, int y0, int x1, int y1)
{
    int y, w;
    if (x0 < 0)
        x0 = 0;
    if (y0 < 0)
        y0 = 0;
    if (x1 > 319)
        x1 = 319;
    if (y1 > 199)
        y1 = 199;
    if (x1 < x0 || y1 < y0)
        return;
    w = x1 - x0 + 1;
    for (y = y0; y <= y1; y++)
        _fmemcpy(MK_FP(0xA000, rowOffset[y] + x0),
                 (unsigned char far *)backBuffer + rowOffset[y] + x0, w);
}

void drawCursorVRAM(int x, int y)
{
    int i;
    for (i = -3; i <= 3; i++)
    {
        if (x + i >= 0 && x + i < 320 && y >= 0 && y < 200)
            *((unsigned char far *)MK_FP(0xA000, rowOffset[y] + x + i)) = 15;
        if (y + i >= 0 && y + i < 200 && x >= 0 && x < 320)
            *((unsigned char far *)MK_FP(0xA000, rowOffset[y + i] + x)) = 15;
    }
}

/* Texte dans le back-buffer avec la police 8x8 de la ROM (F000:FA6E) */
void drawText(int x, int y, const char *str, unsigned char color)
{
    unsigned char far *font = (unsigned char far *)MK_FP(0xF000, 0xFA6E);
    int row, bit, px, py;
    unsigned char bits;

    while (*str)
    {
        for (row = 0; row < 8; row++)
        {
            py = y + row;
            if (py < 0 || py >= 200)
                continue;
            bits = font[(unsigned)(unsigned char)*str * 8 + row];
            for (bit = 0; bit < 8; bit++)
            {
                if (bits & (0x80 >> bit))
                {
                    px = x + bit;
                    if (px >= 0 && px < 320)
                        backBuffer[(long)py * 320L + px] = color;
                }
            }
        }
        x += 8;
        str++;
    }
}

/* ------------------------------------------------------------------ */
/* Rendu du cube (entierement en entiers)                             */
/* ------------------------------------------------------------------ */

void drawCube(const RubikCube &cube, const LayerAnim *anim)
{
    int m[3][3], m2[3][3], r[3][3];
    ViewVecs vv, vv2;
    const ViewVecs *V;
    int animating, deg;
    int i, f, v, a, b, n, key, kz;
    int px, py, pz, tx, ty, tz;
    int cxs, cys, czs, sxv, syv;
    unsigned char col;
    Face3D *fc;

    faceCount = 0;

    /* Matrice de vue, et matrice combinee pour la couche animee */
    viewToMatrix(m);
    buildVecs(m, vv);

    animating = (anim != 0 && anim->active);
    if (animating)
    {
        deg = (int)((long)anim->sign * 90L * anim->step / anim->steps);
        buildLayerMatrix(anim->axis, deg, r);
        mulMat(m, r, m2);
        buildVecs(m2, vv2);
    }

    /* 1. Construction de la liste des faces visibles */
    for (i = 0; i < 26; i++)
    {
        const Cubie &p = cube.pieces[i];

        px = p.currentPos.x;
        py = p.currentPos.y;
        pz = p.currentPos.z;

        if (animating && coordOf(p.currentPos, anim->axis) == anim->layer)
            V = &vv2;
        else
            V = &vv;

        cxs = px * V->ux[0] + py * V->ux[1] + pz * V->ux[2];
        cys = px * V->uy[0] + py * V->uy[1] + pz * V->uy[2];
        czs = px * V->uz[0] + py * V->uz[1] + pz * V->uz[2];

        for (f = 0; f < 6; f++)
        {
            col = p.faceColors[f];
            if (col == COLOR_NONE || !V->vis[f])
                continue;
            if (faceCount >= MAX_FACES)
                break;

            n = faceAxis[f];
            fc = &facesToDraw[faceCount];

            for (v = 0; v < 4; v++)
            {
                tx = faceTemplates[f][v][0];
                ty = faceTemplates[f][v][1];
                tz = faceTemplates[f][v][2];

                sxv = cxs + tx * V->hx[0] + ty * V->hx[1] + tz * V->hx[2];
                syv = cys + tx * V->hy[0] + ty * V->hy[1] + tz * V->hy[2];

                fc->pts[v * 2] = CENTER_X + ((sxv + 128) >> 8);
                fc->pts[v * 2 + 1] = CENTER_Y - ((syv + 128) >> 8);
            }

            fc->color = getMode13Color(col);
            fc->z = czs + faceSign[f] * V->hz[n];
            fc->owner = i;
            fc->faceIdx = f;
            faceCount++;
        }
    }

    /* 2. Tri par insertion d'un tableau d'indices (de l'arriere vers l'avant) */
    for (a = 0; a < faceCount; a++)
        order[a] = a;

    for (a = 1; a < faceCount; a++)
    {
        key = order[a];
        kz = facesToDraw[key].z;
        b = a - 1;
        while (b >= 0 && facesToDraw[order[b]].z > kz)
        {
            order[b + 1] = order[b];
            b--;
        }
        order[b + 1] = key;
    }

    /* 3. Dessin */
    for (a = 0; a < faceCount; a++)
        fillQuad(facesToDraw[order[a]].pts, facesToDraw[order[a]].color);
}