#include "print_solver.h"
#include <stdio.h>

PrintSolver::PrintSolver(const PrinterConfig& config):
  _rodLength(config.rodLength), _rodOffset(config.rodOffset), _sideLength(config.sideLength), _zMax(config.zMax)
{
  _lSquared = (_rodLength * _rodLength);

  float halfSide = _sideLength / 2;
  float height = halfSide * sqrt(3.0f);

  _anchorA = Vec2(0,0);
  _anchorB = Vec2(_sideLength, 0);
  _anchorC = Vec2(halfSide, height);

  Vec2 center(halfSide, height / 2.0f);
  _offsetA = (_anchorA - center).normalize() * _rodOffset;
  _offsetB = (_anchorB - center).normalize() * _rodOffset;
  _offsetC = (_anchorC - center).normalize() * _rodOffset;
}

bool PrintSolver::getHeightsAt(const Vec3& target, float& aHeight, float& bHeight, float& cHeight) {
  float zOffset = target.z;
  Vec2 t(target.x, target.y);

  Vec2 tA = t + _offsetA - _anchorA,
       tB = t + _offsetB - _anchorB,
       tC = t + _offsetC - _anchorC;

  float tAS = tA.magSquared(),
        tBS = tB.magSquared(),
        tCS = tC.magSquared();

  float aHeightSquared = _lSquared - tAS,
        bHeightSquared = _lSquared - tBS,
        cHeightSquared = _lSquared - tCS;

  if(aHeightSquared < 0 || bHeightSquared < 0 || cHeightSquared < 0) {
    printf("Height not computable (from results %f, %f, %f)\n", aHeightSquared, bHeightSquared, cHeightSquared);
    return false;
  }

  aHeight = sqrt(aHeightSquared) + zOffset;
  bHeight = sqrt(bHeightSquared) + zOffset;
  cHeight = sqrt(cHeightSquared) + zOffset;

  return true;
}
