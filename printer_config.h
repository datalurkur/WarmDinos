#ifndef PRINTER_CONFIG_H
#define PRINTER_CONFIG_H

#include <stdio.h>

struct PositionProfile {
  float a;
  float v;
  float d;
  float t;

  PositionProfile(): a(0), v(0), d(0), t(0) {}
  void debug() { printf("%f acc | %f vel | %f dist | %f time\n", a, v, d, t); }
};

#define MOVEMENT_PROFILE_MAX 3

struct MovementProfile {
  float minimumDistance;
  float maxVelocity;

  // intervals[0] is ticks needed to reach max acceleration
  // intervals[1] is ticks before acceleration rampdown begins
  // intervals[2] is ticks needed to reach max velocity and zero acceleration
  // intervals[3] is ticks needed to reach max deceleration
  // intervals[4] is ticks before deceleration rampdown begins
  // intervals[5] is ticks before zero velocity and deceleration
  int intervals[6];
};

struct PrinterConfig {
  // Physical dimensions of the printer
  float sideLength;
  float zMax;
  float rodLength;
  float rodOffset;

  // The distance that each of the A/B/C axes move per stepper step
  float zUnitsPerStep;

  // The length of filament the extruder extrudes/retracts per stepper step
  float eUnitsPerStep;

  // The time (in seconds) between interrupts
  float interruptInterval;

  // Extruder movement maximums
  float jerk;
  float maxAcceleration;
  float maxVelocity;

  // DEPENDENT DATAPOINTS
  float jerkPerTick;
  float maxAccelerationPerTick;
  float maxVelocityPerTick;

  // Precomputed movement profiles
  // 0 - maximum velocity
  // 1 - half velocity
  // 2 - quarter velocity
  MovementProfile movementProfiles[MOVEMENT_PROFILE_MAX];

  // Functions
  void setup();

private:
  void computeMoveProfile(float scale, MovementProfile& profile);
};

extern void ComputeStep(const PrinterConfig& config, float jerkDirection, PositionProfile& vars);

#endif
