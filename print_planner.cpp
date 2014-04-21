#include "print_planner.h"

PrintPlanner::PrintPlanner(const PrinterConfig& config):
  frontBufferIndex(0), backBufferIndex(0),
  // Eventually, we'll do a homing step as part of step, but for now just assume the extruder starts at the center, zeroed
  _position((config.sideLength/2.0f), (config.sideLength/2.0f)*sqrt(3.0f), 0),
  _solver(config), _config(config)
{
  // Set initial conditions for buffers to be filled
  // (Basically, we're faking that this buffer has been "consumed" so that the provider knows to start putting data into it)
  buffers[backBufferIndex].done = true;
}

void PrintPlanner::queueMove(const Vec3& target, float maxSpeed, float maxAccel, float maxJerk) {
  BlockBuffer* backBuffer = &buffers[backBufferIndex];
  while(!backBuffer.done) {
    // Wait for the interrupt to finish consuming this buffer
  }

  backBuffer.ready = false;
  backBuffer.done = false;

  int numberOfBlocks = 0;
  Vec3 speed(0,0,0);
  Vec3 accel(0,0,0);
}
