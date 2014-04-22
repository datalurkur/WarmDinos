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

bool PrintPlanner::queueMove(const Vec3& target) {
  BlockBuffer* backBuffer = getBackBuffer();

  // Prepare the distance between the current location and the target, and normalize a unit vector for determining incremental extruder positions
  Vec3 direction = target - _position;
  float distance = direction.mag();
  Vec3 unit = direction.normalize();

  // Determine the fastest movement profile that this move can use
  int moveProfileIndex;
  for(moveProfileIndex = 0;
      moveProfileIndex < MOVEMENT_PROFILE_MAX &&
        _config.movementProfiles[moveProfileIndex].minimumDistance > distance;
      moveProfileIndex++) {}

  if(moveProfileIndex == MOVEMENT_PROFILE_MAX) {
    printf("Error: No movement profile short enough to accomodate move\n");
    return false;
  } else {
    printf("Using movement profile %i\n", moveProfileIndex);
  }
  MovementProfile* profile = &_config.movementProfiles[moveProfileIndex];

  PositionProfile vars;

  // Cache some indices for quick lookups
  int constSpeedDuration = (distance - profile->minimumDistance) / profile->maxVelocity;
  int decelStart         = profile->intervals[2] + constSpeedDuration;
  int thirdJerkUpper     = decelStart + profile->intervals[3];
  int fourthJerkLower    = decelStart + profile->intervals[4];
  int totalBlocks        = decelStart + profile->intervals[5];

  int blockIndex = 0;
  for(int i = 0; i < totalBlocks; i++) {
    // Is this block buffer full?
    if(blockIndex >= BLOCK_BUFFER_LENGTH) {
      // Commit this block buffer and move to the next one
      backBuffer->numberOfBlocks = blockIndex;
      commitBackBuffer();
      backBuffer = getBackBuffer();
    }

    // Determine whether our acceleration is ramping up, ramping down, or staying constant for this block
    float jerkDirection = 0;
    if(i < profile->intervals[0] || i >= fourthJerkLower) {
      jerkDirection = 1.0f;
    } else if(
      (i >= profile->intervals[1] && i < profile->intervals[2]) ||
      (i >= decelStart            && i < thirdJerkUpper)
    ) {
      jerkDirection = -1.0f;
    }

    // Iterate our position profile
    ComputeStep(_config, jerkDirection, vars);

    // Using our unit vector, project the new extruder location
    Vec3 newPosition = _position + (unit * vars.d);

    // Determine the new location of the steppers
    float heights[NUMBER_OF_AXES];
    _solver.getHeightsAt(newPosition, heights);
    for(int i = STEPPER_A; i <= STEPPER_C; i++) {
      _stepperProgress[i] += heights[i] - _stepperPosition[i];
      if       (_stepperProgress[i] >=  _config.zUnitsPerStep) {
        backBuffer->blocks[i].step[i]    = true;
        backBuffer->blocks[i].forward[i] = true;
      } else if(_stepperProgress[i] <= -_config.zUnitsPerStep) {
        backBuffer->blocks[i].step[i]    = true;
        backBuffer->blocks[i].forward[i] = false;
      } else {
        backBuffer->blocks[i].step[i]    = false;
      }
    }
    blockIndex++;
  }
  backBuffer->numberOfBlocks = blockIndex;
  commitBackBuffer();

  return true;
}

void PrintPlanner::commitBackBuffer() {
  BlockBuffer* backBuffer = &buffers[backBufferIndex];
  backBuffer->ready = true;
  backBufferIndex++;
}

BlockBuffer* PrintPlanner::getBackBuffer() {
  BlockBuffer* ret = &buffers[backBufferIndex];
  while(!ret->done) {
    // Wait for the interrupt to finish consuming this buffer
  }
  ret->ready = false;
  ret->done = false;
  return ret;
}
