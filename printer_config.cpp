#include "printer_config.h"
#include <stdio.h>

void PrinterConfig::setup() {
    jerkPerTick = jerk * interruptInterval;
    maxAccelerationPerTick = maxAcceleration * interruptInterval;
    maxVelocityPerTick = maxVelocity * interruptInterval;
    printf("Per-tick jerk %f, acceleration %f, and velocity %f\n", jerkPerTick, maxAccelerationPerTick, maxVelocityPerTick);

    computeMoveProfile(1.00f, movementProfiles[0]);
    computeMoveProfile(0.50f, movementProfiles[1]);
    computeMoveProfile(0.25f, movementProfiles[2]);
}

void PrinterConfig::computeMoveProfile(float scale, MovementProfile& profile) {
  int i;

  float maxV = maxVelocityPerTick * scale;
  float halfV = maxV / 2.0f;

  // For avoiding problems with signed zero
  float epsilon = jerkPerTick * 0.1f;
  //printf("Computing movement profile with max velocity %f\n", maxV);

  PositionProfile vars;

  // Acceleration ramp-up / velocity ramp-up
  //printf("=== A U / V U ===\n");
  for(i = 0; vars.a < maxAccelerationPerTick && (vars.v + vars.a) < halfV; i++) {
    ComputeStep(*this, 1.0f, vars);
    //printf("\t[%i] ", i); vars.debug();
  }
  profile.intervals[0] = i;
  float velocityPerRampUp = vars.v;

  // Constant acceleration / velocity ramp-up
  //printf("=== A C / V U ===\n");
  for(; vars.v < maxV - velocityPerRampUp; i++) {
    ComputeStep(*this, 0.0f, vars);
    //printf("\t[%i] ", i); vars.debug();
  }
  profile.intervals[1] = i;

  // Acceleration ramp-down / velocity ramp-up
  //printf("=== A D / V U ===\n");
  for(; vars.a > 0.0f; i++) {
    ComputeStep(*this, -1.0f, vars);
    //printf("\t[%i] ", i); vars.debug();
  }
  profile.intervals[2] = i;

  // Ignoring the constant-velocity portion...
  profile.maxVelocity = vars.v;

  // Acceleration ramp-up / velocity ramp-down
  //printf("=== A D / V D ===\n");
  for(i = 0; vars.a > -maxAccelerationPerTick && (vars.v > (maxV - velocityPerRampUp)); i++) {
    ComputeStep(*this, -1.0f, vars);
    //printf("\t[%i] ", i); vars.debug();
  }
  profile.intervals[3] = i;

  // Constant acceleration / velocity ramp-down
  // Ensure that the velocity gets re-zeroed
  //printf("=== A C / V D ===\n");
  for(; i < (profile.intervals[3] + (profile.intervals[1] - profile.intervals[0])); i++) {
    ComputeStep(*this, 0.0f, vars);
    //printf("\t[%i] ", i); vars.debug();
  }
  profile.intervals[4] = i;

  // Acceleration ramp-down / velocity ramp-down
  //printf("=== A U / V D ===\n");
  for(; vars.a < 0.0f - epsilon && vars.v + vars.a > 0.0f; i++) {
    ComputeStep(*this, 1.0f, vars);
    //printf("\t[%i] ", i); vars.debug();
  }
  profile.intervals[5] = i;

  profile.minimumDistance = vars.d;
}

void ComputeStep(const PrinterConfig& config, float jerkDirection, PositionProfile& vars) {
  vars.a += config.jerkPerTick * jerkDirection;
  if(vars.a > config.maxAccelerationPerTick) { vars.a = config.maxAccelerationPerTick; }
  if(vars.a < -config.maxAccelerationPerTick) { vars.a = -config.maxAccelerationPerTick; }

  vars.v += vars.a;
  if(vars.v > config.maxVelocityPerTick) { vars.v = config.maxVelocityPerTick; }
  if(vars.v < -config.maxVelocityPerTick) { vars.v = -config.maxVelocityPerTick; }

  vars.d += vars.v;

  vars.t += config.interruptInterval;
}
