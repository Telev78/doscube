#include <math.h>
#include <mem.h>
#include <dos.h>
#include "renderer.h"

// Déclaration externe obligatoire du tampon partagé (défini dans main.cpp)
extern unsigned char far backBuffer[];

// Correspondance des couleurs pour la palette 256 couleurs du Mode 13h
unsigned char getMode13Color(unsigned char cubeColor)
{
    switch (cubeColor)
    {
    case COLOR_WHITE:
        return 15; // Blanc brillant
    case COLOR_YELLOW:
        return 14; // Jaune
    case COLOR_RED:
        return 4; // Rouge
    case COLOR_ORANGE:
        return 41; // Orange
    case COLOR_BLUE:
        return 1; // Bleu
    case COLOR_GREEN:
        return 2; // Vert
    default:
        return 0; // Noir / Transparent
    }
}

// Structure d'une face 3D projetée
struct Face3D
{
    unsigned char color;
    double avgZ;
    int pts[8];
    int owner;
    int faceIdx;
};

// --- RASTERISEUR DE TRIANGLES PLEINS POUR LE BACK-BUFFER ---
void fillFlatBottomTriangle(int x1, int y1, int x2, int y2, int x3, int y3, unsigned char color)
{
    if (y1 == y2)
        return;
    float invslope1 = (float)(x2 - x1) / (y2 - y1);
    float invslope2 = (float)(x3 - x1) / (y3 - y1);
    float curx1 = x1;
    float curx2 = x1;

    for (int scany = y1; scany <= y2; scany++)
    {
        if (scany >= 0 && scany < 200)
        {
            int startx = (int)curx1;
            int endx = (int)curx2;
            if (startx > endx)
            {
                int t = startx;
                startx = endx;
                endx = t;
            }
            if (startx < 0)
                startx = 0;
            if (endx >= 320)
                endx = 319;

            unsigned char *rowPtr = &backBuffer[(long)scany * 320L];
            for (int x = startx; x <= endx; x++)
            {
                rowPtr[x] = color;
            }
        }
        curx1 += invslope1;
        curx2 += invslope2;
    }
}

void fillFlatTopTriangle(int x1, int y1, int x2, int y2, int x3, int y3, unsigned char color)
{
    if (y1 == y3)
        return;
    float invslope1 = (float)(x3 - x1) / (y3 - y1);
    float invslope2 = (float)(x3 - x2) / (y3 - y2);
    float curx1 = x1;
    float curx2 = x2;

    for (int scany = y1; scany <= y3; scany++)
    {
        if (scany >= 0 && scany < 200)
        {
            int startx = (int)curx1;
            int endx = (int)curx2;
            if (startx > endx)
            {
                int t = startx;
                startx = endx;
                endx = t;
            }
            if (startx < 0)
                startx = 0;
            if (endx >= 320)
                endx = 319;

            unsigned char *rowPtr = &backBuffer[(long)scany * 320L];
            for (int x = startx; x <= endx; x++)
            {
                rowPtr[x] = color;
            }
        }
        curx1 += invslope1;
        curx2 += invslope2;
    }
}

void fillTriangle(int x1, int y1, int x2, int y2, int x3, int y3, unsigned char color)
{
    int tx, ty;
    if (y1 > y2)
    {
        tx = x1;
        x1 = x2;
        x2 = tx;
        ty = y1;
        y1 = y2;
        y2 = ty;
    }
    if (y1 > y3)
    {
        tx = x1;
        x1 = x3;
        x3 = tx;
        ty = y1;
        y1 = y3;
        y3 = ty;
    }
    if (y2 > y3)
    {
        tx = x2;
        x2 = x3;
        x3 = tx;
        ty = y2;
        y2 = y3;
        y3 = ty;
    }

    if (y2 == y3)
    {
        fillFlatBottomTriangle(x1, y1, x2, y2, x3, y3, color);
    }
    else if (y1 == y2)
    {
        fillFlatTopTriangle(x1, y1, x2, y2, x3, y3, color);
    }
    else
    {
        int x4 = (int)(x1 + ((float)(y2 - y1) / (y3 - y1)) * (x3 - x1));
        int y4 = y2;
        fillFlatBottomTriangle(x1, y1, x2, y2, x4, y4, color);
        fillFlatTopTriangle(x2, y2, x4, y4, x3, y3, color);
    }
}

// --- RASTERISEUR DIRECT DE QUADRILATÈRES PAR BALAYAGE (SCANLINE) ---
void fillQuad(int pts[8], unsigned char color)
{
    int minY = pts[1];
    int maxY = pts[1];
    int idx, scanY, a, b_idx, x;

    for (idx = 1; idx < 4; idx++)
    {
        if (pts[idx * 2 + 1] < minY)
            minY = pts[idx * 2 + 1];
        if (pts[idx * 2 + 1] > maxY)
            maxY = pts[idx * 2 + 1];
    }

    if (minY < 0)
        minY = 0;
    if (maxY > 199)
        maxY = 199;

    for (scanY = minY; scanY <= maxY; scanY++)
    {
        int intersections[4];
        int count = 0;

        for (idx = 0; idx < 4; idx++)
        {
            int x1 = pts[idx * 2];
            int y1 = pts[idx * 2 + 1];
            int x2 = pts[((idx + 1) % 4) * 2];
            int y2 = pts[((idx + 1) % 4) * 2 + 1];

            if ((y1 <= scanY && y2 > scanY) || (y2 <= scanY && y1 > scanY))
            {
                if (y2 != y1)
                {
                    int intersectionX = x1 + (int)((long)(scanY - y1) * (x2 - x1) / (y2 - y1));
                    if (count < 4)
                    {
                        intersections[count++] = intersectionX;
                    }
                }
            }
        }

        if (count >= 2)
        {
            for (a = 1; a < count; a++)
            {
                int key = intersections[a];
                b_idx = a - 1;
                while (b_idx >= 0 && intersections[b_idx] > key)
                {
                    intersections[b_idx + 1] = intersections[b_idx];
                    b_idx--;
                }
                intersections[b_idx + 1] = key;
            }

            unsigned char *rowPtr = &backBuffer[(long)scanY * 320L];
            for (a = 0; a < count - 1; a += 2)
            {
                int startx = intersections[a];
                int endx = intersections[a + 1];

                if (startx < 0)
                    startx = 0;
                if (endx >= 320)
                    endx = 319;

                for (x = startx; x <= endx; x++)
                {
                    rowPtr[x] = color;
                }
            }
        }
    }
}

static Face3D facesToDraw[130];
static int faceCount = 0;

// Coordonnée d'un point selon l'axe (0=X, 1=Y, 2=Z)
static int coordOf(const Point3D &p, int axis)
{
    if (axis == 0)
        return p.x;
    if (axis == 1)
        return p.y;
    return p.z;
}

// Rotation d'un point autour d'un axe (c = cos, s = sin de l'angle)
static void rotAxis(double &x, double &y, double &z, int axis, double c, double s)
{
    double t;
    if (axis == 0)
    {
        t = y * c - z * s;
        z = y * s + z * c;
        y = t;
    }
    else if (axis == 1)
    {
        t = x * c + z * s;
        z = -x * s + z * c;
        x = t;
    }
    else
    {
        t = x * c - y * s;
        y = x * s + y * c;
        x = t;
    }
}

// Fonction principale de rendu du cube dans le back-buffer (avec correction d'aspect 4/3)
void drawCube(const RubikCube &cube, double angleX, double angleY, const LayerAnim *anim)
{
    int centerX = 160;
    int centerY = 100;
    double spacing = 28.0;
    double hs = spacing * 0.42;

    double radX = angleX * 3.14159265 / 180.0;
    double radY = angleY * 3.14159265 / 180.0;
    double cosX = cos(radX), sinX = sin(radX);
    double cosY = cos(radY), sinY = sin(radY);

    int animating = (anim != 0 && anim->active);
    int inLayer = 0;
    double ac = 1.0, as = 0.0, th;
    if (animating)
    {
        th = anim->sign * 1.5707963 * anim->step / anim->steps;
        ac = cos(th);
        as = sin(th);
    }

    faceCount = 0;

    static const signed char faceTemplates[6][4][3] = {
        {{-1, 1, -1}, {1, 1, -1}, {1, 1, 1}, {-1, 1, 1}},     // 0: Haut
        {{-1, -1, -1}, {-1, -1, 1}, {1, -1, 1}, {1, -1, -1}}, // 1: Bas
        {{-1, -1, -1}, {-1, -1, 1}, {-1, 1, 1}, {-1, 1, -1}}, // 2: Gauche
        {{1, -1, -1}, {1, 1, -1}, {1, 1, 1}, {1, -1, 1}},     // 3: Droite
        {{-1, -1, 1}, {1, -1, 1}, {1, 1, 1}, {-1, 1, 1}},     // 4: Avant
        {{1, -1, -1}, {1, 1, -1}, {-1, 1, -1}, {-1, -1, -1}}  // 5: Arrière
    };

    static double faceNormals[6][3] = {
        {0.0, 1.0, 0.0},
        {0.0, -1.0, 0.0},
        {-1.0, 0.0, 0.0},
        {1.0, 0.0, 0.0},
        {0.0, 0.0, 1.0},
        {0.0, 0.0, -1.0}};

    int i, f, v, a, b, c;
    double cx, cy, cz, sumZ, avgZ;
    double rotPts[4][3];

    // 1. Rendu des 26 cubies mobiles
    for (i = 0; i < 26; i++)
    {

        cx = cube.pieces[i].currentPos.x * spacing;
        cy = cube.pieces[i].currentPos.y * spacing;
        cz = cube.pieces[i].currentPos.z * spacing;

        for (f = 0; f < 6; f++)
        {
            unsigned char col = cube.pieces[i].faceColors[f];
            if (col == COLOR_NONE)
                continue;

            double nx = faceNormals[f][0] * hs;
            double ny = faceNormals[f][1] * hs;
            double nz = faceNormals[f][2] * hs;

            if (inLayer)
                rotAxis(nx, ny, nz, anim->axis, ac, as);

            double nx1 = nx * cosY - nz * sinY;
            double nz1 = nx * sinY + nz * cosY;
            double ny1 = ny;

            double rz_n = ny1 * sinX + nz1 * cosX;

            if (rz_n < 0)
                continue;

            sumZ = 0;
            for (v = 0; v < 4; v++)
            {
                double lx = cx + faceTemplates[f][v][0] * hs;
                double ly = cy + faceTemplates[f][v][1] * hs;
                double lz = cz + faceTemplates[f][v][2] * hs;

                if (inLayer)
                    rotAxis(lx, ly, lz, anim->axis, ac, as);

                double x1 = lx * cosY - lz * sinY;
                double z1 = lx * sinY + lz * cosY;
                double y1 = ly;

                rotPts[v][0] = x1;
                rotPts[v][1] = y1 * cosX - z1 * sinX;
                rotPts[v][2] = y1 * sinX + z1 * cosX;
                sumZ += rotPts[v][2];
            }

            avgZ = sumZ / 4.0;

            if (faceCount < 130)
            {
                facesToDraw[faceCount].owner = i;
                facesToDraw[faceCount].faceIdx = f;
                facesToDraw[faceCount].color = getMode13Color(col);
                facesToDraw[faceCount].avgZ = avgZ;

                for (v = 0; v < 4; v++)
                {
                    facesToDraw[faceCount].pts[v * 2] = centerX + (int)rotPts[v][0];
                    // Correction d'aspect 4/3 appliquée sur l'axe Y (facteur 5.0 / 6.0)
                    facesToDraw[faceCount].pts[v * 2 + 1] = centerY - (int)(rotPts[v][1] * 5.0 / 6.0);
                }
                faceCount++;
            }
        }
    }

    // 2. Tri par insertion des faces visibles
    for (a = 0; a < faceCount - 1; a++)
    {
        for (b = a + 1; b < faceCount; b++)
        {
            if (facesToDraw[a].avgZ > facesToDraw[b].avgZ)
            {
                Face3D temp = facesToDraw[a];
                facesToDraw[a] = facesToDraw[b];
                facesToDraw[b] = temp;
            }
        }
    }

    // 3. Dessin de toutes les faces visibles triées
    for (i = 0; i < faceCount; i++)
    {
        fillQuad(facesToDraw[i].pts, facesToDraw[i].color);
    }
}

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

// Les faces sont triées de l'arrière vers l'avant : on parcourt à l'envers
int pickFace(int px, int py, int &owner, int &faceIdx)
{
    int i;
    for (i = faceCount - 1; i >= 0; i--)
        if (pointInQuad(facesToDraw[i].pts, px, py))
        {
            owner = facesToDraw[i].owner;
            faceIdx = facesToDraw[i].faceIdx;
            return 1;
        }
    return 0;
}

// Projection écran (y vers le bas) d'un vecteur unité du modèle
void projectDir(int ax, int ay, int az, double angleX, double angleY, double &sx, double &sy)
{
    double rx = angleX * 3.14159265 / 180.0, ry = angleY * 3.14159265 / 180.0;
    double x1 = ax * cos(ry) - az * sin(ry);
    double z1 = ax * sin(ry) + az * cos(ry);
    sx = x1;
    sy = -(ay * cos(rx) - z1 * sin(rx)) * 5.0 / 6.0;
}

void drawCursor(int x, int y)
{
    int i;
    for (i = -3; i <= 3; i++)
    {
        if (x + i >= 0 && x + i < 320 && y >= 0 && y < 200)
            backBuffer[(long)y * 320L + x + i] = 15;
        if (y + i >= 0 && y + i < 200 && x >= 0 && x < 320)
            backBuffer[(long)(y + i) * 320L + x] = 15;
    }
}

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