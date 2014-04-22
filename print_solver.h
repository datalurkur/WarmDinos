#ifndef PRINT_SOLVER_H
#define PRINT_SOLVER_H

#include "defines.h"
#include "vec.h"
#include "printer_config.h"

class PrintSolver {
public:
  PrintSolver(const PrinterConfig& config);

  bool isInsidePrintArea(const Vec3& point);
  bool getHeightsAt(const Vec3& target, float heights[NUMBER_OF_AXES]);

private:
  float _rodLength, _rodOffset, _sideLength, _zMax;

  float _lSquared;
  Vec2 _anchors[NUMBER_OF_AXES];
  Vec2 _offsets[NUMBER_OF_AXES];
};

#endif
