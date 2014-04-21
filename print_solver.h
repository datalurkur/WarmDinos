#ifndef PRINT_SOLVER_H
#define PRINT_SOLVER_H

#include "vec.h"
#include "printer_config.h"

class PrintSolver {
public:
  PrintSolver(const PrinterConfig& config);

  bool isInsidePrintArea(const Vec3& point);
  bool getHeightsAt(const Vec3& target, float& aHeight, float& bHeight, float& cHeight);

private:
  float _rodLength, _rodOffset, _sideLength, _zMax;

  float _lSquared;
  Vec2 _anchorA, _anchorB, _anchorC;
  Vec2 _offsetA, _offsetB, _offsetC;
};

#endif
