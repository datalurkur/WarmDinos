#include "print_solver.h"
#include "printer_config.h"
#include <stdio.h>
#include <fstream>

PrinterConfig config;

std::ofstream fileStream;

void testSolver() {
  PrintSolver solver(config);

  Vec3 p0(5, 5, 5),
       p1(11, 15, 5);

  fileStream.open("motors.dat");
  
  int steps = 100;
  float hA, hB, hC;
  for(int c = 0; c < steps; c++) {
    float t = (float)c / steps;
    Vec3 target = p0 + ((p1 - p0) * t);
    printf("Solving for point at [%f,%f]...\n", target.x, target.y);
    if(!solver.getHeightsAt(target, hA, hB, hC)) {
      printf("\tFAILED\n");
      break;
    } else {
      printf("\tFound heights %f / %f / %f\n", hA, hB, hC);
    }
    fileStream << t << " " << hA << " " << hB << " " << hC << " " << target.x << " " << target.y << " " << target.z << "\n";
  }
  fileStream.close();
}

void iterateTestVars(int start, int stop, float jerkDirection, PositionProfile& vars) {
  for(int i = start; i < stop; i++) {
    ComputeStep(config, jerkDirection, vars);
    fileStream << vars.t << " " << vars.d << " " << vars.v << " " << vars.a << "\n";
  }
}

void testMoveProfile() {
  float totalDistance = 0.1f;

  MovementProfile* move = &config.movementProfiles[0];
  PositionProfile vars;

  float constVDistance = totalDistance - move->minimumDistance;
  int ticksOfConstV = constVDistance / move->maxVelocity;

  if(totalDistance < move->minimumDistance) {
    printf("Velocity profile does not allow movements of less than %f\n", move->minimumDistance);
    return;
  }

  fileStream.open("movement.dat");

  printf("Attempting to move %f units with maximum speed %f and minimum distance %f\n", totalDistance, config.maxVelocity, move->minimumDistance);

  printf("[0,%i] - positive jerk applied, acceleration is ramping up\n", move->intervals[0]);
  iterateTestVars(0, move->intervals[0], 1, vars);
  printf("[%i,%i] - no jerk, interval of constant acceleration\n", move->intervals[0], move->intervals[1]);
  iterateTestVars(move->intervals[0], move->intervals[1], 0, vars);
  printf("[%i,%i] - negative jerk applied, acceleration is ramping down\n", move->intervals[1], move->intervals[2]);
  iterateTestVars(move->intervals[1], move->intervals[2], -1, vars);
  int tickOffset = ticksOfConstV + move->intervals[2];
  printf("[%i,%i] - period of constant velocity\n", move->intervals[2], tickOffset);
  iterateTestVars(move->intervals[2], tickOffset, 0, vars);
  printf("[%i,%i] - negative jerk applied, negative acceleration is ramping up\n", tickOffset, tickOffset + move->intervals[3]);
  iterateTestVars(tickOffset, tickOffset + move->intervals[3], -1, vars);
  printf("[%i,%i] - no jerk, negative acceleration is holding\n", tickOffset + move->intervals[3], tickOffset + move->intervals[4]);
  iterateTestVars(tickOffset + move->intervals[3], tickOffset + move->intervals[4], 0, vars);
  printf("[%i,%i] - positive jerk applied, negative acceleration is ramping down\n", tickOffset + move->intervals[4], tickOffset + move->intervals[5]);
  iterateTestVars(tickOffset + move->intervals[4], tickOffset + move->intervals[5], 1, vars);
  printf("Done - travelled %f units in %f seconds\n", vars.d, vars.t);

  fileStream.close();
}

int main() {
  config.sideLength = 20; // Width of one side
  config.rodLength = 20; // Length of the fixed rods
  config.rodOffset = 2; // Radius of the extruder platform
  config.zMax = 10;
  config.jerk = 2.0f;
  config.interruptInterval = 1.0f / 9600.0f;
  config.maxAcceleration = 10.0f;
  config.maxVelocity = 50.0f;
  config.setup();

  testMoveProfile();

  return 0;
}
