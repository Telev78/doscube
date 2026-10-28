#ifndef CONTROL_H
#define CONTROL_H

int computeMove(const RubikCube &cube, int owner, int f, int dx, int dy,
                int &axis, int &layer, int &sign);
#endif