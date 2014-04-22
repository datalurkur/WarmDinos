#ifndef PRINT_PLANNER_H
#define PRINT_PLANNER_H

#include "block_buffer.h"
#include "printer_config.h"
#include "print_solver.h"
#include "vec.h"

#define RING_BUFFER_LENGTH 4

class PrintPlanner {
public:
  PrintPlanner(const PrinterConfig& config);

  bool queueMove(const Vec3& target);

  // Get the front buffer
  BlockBuffer* getFrontBuffer();
  // Mark the front buffer as done
  void releaseFrontBuffer();

private:
  // Mark the current block buffer as ready to be consumed
  void commitBackBuffer();
  // Get and wait for the next block buffer to be ready
  BlockBuffer* getBackBuffer();

private:
  // A buffer of buffers, oh my!
  BlockBuffer _buffers[RING_BUFFER_LENGTH];

  // The buffer currently being consumed by the interrupt
  int _frontBufferIndex;
  // The buffer currently being filled by the planner
  int _backBufferIndex;

  Vec3 _position;

  // Used to track the absolute position of the steppers
  float _stepperPosition[NUMBER_OF_AXES];
  // Used to track the threshold at which a stepper should step
  float _stepperProgress[NUMBER_OF_AXES];

  PrintSolver _solver;
  PrinterConfig _config;
};

#endif
