#include "print_planner.h"
#include <unistd.h>

// Eventually, we'll do a homing step as part of step, but for now just assume the extruder starts at the center, zeroed
PrintPlanner::PrintPlanner(const PrinterConfig& config):
  _frontBufferIndex(0), _backBufferIndex(0), _position(0.0f, 0.0f, 0.0f),
  _solver(config), _config(config)
{
  int i;

  _solver.getHeightsAt(_position, _stepperPosition);

  // Set initial conditions for buffers to be filled
  // (Basically, we're faking that the buffers have been "consumed" so that the provider knows to start putting data into them)
  for(i = 0; i < RING_BUFFER_LENGTH; i++) {
    _buffers[i].done = true;
  }

  // Reset the stepper counters
  for(i = STEPPER_A; i <= STEPPER_C; i++) {
    _stepperProgress[i] = 0.0f;
  }
}

bool PrintPlanner::queueMove(const Vec3& target) {
  BlockBuffer* backBuffer = getBackBuffer();

  // Prepare the distance between the current location and the target, and normalize a unit vector for determining incremental extruder positions
  Vec3 direction = target - _position;
  float distance = direction.mag();
  Vec3 unit = direction.normalized();

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
    printf("\t[%i] ", i); vars.debug();

    // Using our unit vector, project the new extruder location
    Vec3 newPosition = _position + (unit * vars.d);

    // Determine the new location of the steppers
    float heights[NUMBER_OF_AXES];
    _solver.getHeightsAt(newPosition, heights);
    for(int j = STEPPER_A; j <= STEPPER_C; j++) {
      float delta = heights[j] - _stepperPosition[j];
      if(fabs(_stepperProgress[j] + delta) >= (_config.zUnitsPerStep * 2)) {
        printf("ERROR: Speed too fast for stepper %i (will lag behind)\n", i);
      }
      _stepperProgress[j] += delta;
      _stepperPosition[j] = heights[j];
      if       (_stepperProgress[j] >=  _config.zUnitsPerStep) {
        backBuffer->blocks[i].step[j]    = true;
        backBuffer->blocks[i].forward[j] = true;
        _stepperProgress[j] -= _config.zUnitsPerStep;
        printf("\t\tStepper %i moves forward\n", j);
      } else if(_stepperProgress[j] <= -_config.zUnitsPerStep) {
        backBuffer->blocks[i].step[j]    = true;
        backBuffer->blocks[i].forward[j] = false;
        _stepperProgress[j] += _config.zUnitsPerStep;
        printf("\t\tStepper %i moves backward\n", j);
      } else {
        backBuffer->blocks[i].step[j]    = false;
      }
    }
    blockIndex++;
  }
  backBuffer->numberOfBlocks = blockIndex;
  commitBackBuffer();

  return true;
}

BlockBuffer* PrintPlanner::getFrontBuffer() {
  printf("Fetching front buffer at %i\n", _frontBufferIndex);
  return &_buffers[_frontBufferIndex];
}

void PrintPlanner::releaseFrontBuffer() {
  printf("Releasing front buffer %i for production\n", _frontBufferIndex);
  BlockBuffer* frontBuffer = &_buffers[_frontBufferIndex];
  frontBuffer->done = true;
  _frontBufferIndex++;
  if(_frontBufferIndex >= RING_BUFFER_LENGTH) { _frontBufferIndex -= RING_BUFFER_LENGTH; }
}

void PrintPlanner::commitBackBuffer() {
  printf("Committing back buffer %i for consumption\n", _backBufferIndex);
  BlockBuffer* backBuffer = &_buffers[_backBufferIndex];
  backBuffer->ready = true;
  _backBufferIndex++;
  if(_backBufferIndex >= RING_BUFFER_LENGTH) { _backBufferIndex -= RING_BUFFER_LENGTH; }
}

BlockBuffer* PrintPlanner::getBackBuffer() {
  BlockBuffer* ret = &_buffers[_backBufferIndex];
  while(!ret->done) {
    // Wait for the interrupt to finish consuming this buffer
    printf("Print planner waiting for buffer %i to be finished\n", _backBufferIndex);
    sleep(1);
  }
  ret->ready = false;
  ret->done = false;
  return ret;
}
