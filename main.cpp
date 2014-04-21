#include "print_solver.h"
#include <stdio.h>
#include <fstream>

void testSolver() {
  float width = 20; // Width of one side
  float rodLength = 20; // Length of the fixed rods
  float rodOffset = 2; // Radius of the extruder platform
  float maxDepth = 10;
  PrintSolver solver(rodLength, rodOffset, width, maxDepth);

  Vec3 p0(5, 5, 5),
       p1(11, 15, 5);

  std::ofstream fileStream;
  fileStream.open("plot.dat");
  
  int steps = 100;
  float hA, hB, hC;
  for(int c = 0; c < steps; c++) {
    float t = (float)c / steps;
    Vec3 target = p0 + ((p1 - p0) * t);
    printf("Solving for point at [%f,%f]...\n", target.x, target.y);
    if(!solver.getHeightsAt(target, hA, hB, hC)) {
      printf("\tFAILED\n");
      break;
    } else {
      printf("\tFound heights %f / %f / %f\n", hA, hB, hC);
    }
    fileStream << t << " " << hA << " " << hB << " " << hC << " " << target.x << " " << target.y << " " << target.z << "\n";
  }
  fileStream.close();
}

int main() {
  testSolver();

  return 0;
}
