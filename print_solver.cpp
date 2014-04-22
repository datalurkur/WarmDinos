#include "print_solver.h"
#include <stdio.h>

PrintSolver::PrintSolver(const PrinterConfig& config):
  _rodLength(config.rodLength), _rodOffset(config.rodOffset), _sideLength(config.sideLength), _zMax(config.zMax)
{
  _lSquared = (_rodLength * _rodLength);

  float halfSide = _sideLength / 2;
  float height = halfSide * sqrt(3.0f);

  _anchors[STEPPER_A] = Vec2(0,0);
  _anchors[STEPPER_B] = Vec2(_sideLength, 0);
  _anchors[STEPPER_C] = Vec2(halfSide, height);

  Vec2 center(halfSide, height / 2.0f);
  for(int i = STEPPER_A; i < STEPPER_C; i++) {
    _offsets[i] = (_anchors[i] - center).normalize() * _rodOffset;
  }
}

bool PrintSolver::getHeightsAt(const Vec3& target, float heights[NUMBER_OF_AXES]) {
  float zOffset = target.z;
  Vec2 t(target.x, target.y);

  for(int i = STEPPER_A; i < STEPPER_C; i++) {
    Vec2 target = t + _offsets[i] - _anchors[i];
    float mag = target.magSquared();
    float heightSquared = _lSquared - mag;
    if(heightSquared < 0) {
      printf("Height not computable\n");
      return false;
    }
    heights[i] = sqrt(heightSquared) + zOffset;
  }

  return true;
}
