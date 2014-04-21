#ifndef PRINT_PLANNER_H
#define PRINT_PLANNER_H

#define RING_BUFFER_LENGTH 4

class PrintPlanner {
public:
  PrintPlanner(const PrinterConfig& config);

  void queueMove(const Vec3& target, float maxSpeed, float maxAccel, float maxJerk);

  // A buffer of buffers, oh my!
  BlockBuffer buffers[RING_BUFFER_LENGTH];
  // The buffer currently being consumed by the interrupt
  int frontBufferIndex;
  // The buffer currently being filled by the planner
  int backBufferIndex;

private:
  Vec3 _position;

  PrintSolver _solver;
  PrinterConfig _config;
};

#endif
