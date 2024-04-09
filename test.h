#ifndef MYHEADER_H
#define MYHEADER_H

// Function declaration
void add_2(int x, int y);

// Simulation
typedef  struct bodies{
    double x, y;    // Position
    double vx, vy;  // Velocity
    double angle;
} Body;


#endif