#include "print_solver.h"
#include "printer_config.h"
#include "print_planner.h"

#include <stdio.h>
#include <unistd.h>
#include <fstream>
#include <thread>

using namespace std;

PrinterConfig config;
PrintPlanner* planner;

ofstream fileStream;

void testSolver() {
  PrintSolver solver(config);

  Vec3 p0(5, 5, 5),
       p1(11, 15, 5);

  fileStream.open("motors.dat");
  
  int steps = 100;
  float heights[NUMBER_OF_AXES];
  for(int c = 0; c < steps; c++) {
    float t = (float)c / steps;
    Vec3 target = p0 + ((p1 - p0) * t);
    printf("Solving for point at [%f,%f]...\n", target.x, target.y);
    if(!solver.getHeightsAt(target, heights)) {
      printf("\tFAILED\n");
      break;
    } else {
      printf("\tFound heights %f / %f / %f\n", heights[0], heights[1], heights[2]);
    }
    fileStream << t << " " << heights[0] << " " << heights[1] << " " << heights[2] << " " << target.x << " " << target.y << " " << target.z << "\n";
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

bool doneConsuming = false;
void consumePrintBlocks() {
  while(!doneConsuming) {
    BlockBuffer* buffer = planner->getFrontBuffer();
    while(!buffer->ready) {
      printf("Waiting for buffer to be ready\n");
      sleep(1);
      // Wait for the buffer to be ready
    }

    // Fake buffer consumption

    planner->releaseFrontBuffer();
  }
}

void testPrintPlanner() {
  planner = new PrintPlanner(config);

  thread consumeThread = thread(consumePrintBlocks);

  bool ret = planner->queueMove(Vec3(1, 0.5, 0));
  if(ret) {
    printf("Move successfully queued\n");
  } else {
    printf("Failed to queue move\n");
  }

  doneConsuming = true;
  consumeThread.join();

  delete planner;
}

int main() {
  config.sideLength = 20; // Width of one side
  config.rodLength = 20; // Length of the fixed rods
  config.rodOffset = 2; // Radius of the extruder platform
  config.zMax = 10;
  config.jerk = 2.5f;
  config.interruptInterval = 1.0f / 9600.0f;
  config.maxAcceleration = 5.0f;
  config.maxVelocity = 25.0f;
  config.zUnitsPerStep = 0.01f;
  config.eUnitsPerStep = 0.01f;
  config.setup();

  testPrintPlanner();

  return 0;
}
