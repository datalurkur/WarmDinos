#ifndef PRINTER_CONFIG_H
#define PRINTER_CONFIG_H

struct PrinterConfig {
  // Physical dimensions of the printer
  float sideLength;
  float zMax;
  float rodLength;
  float rodOffset;

  // The distance that each of the A/B/C axes move per stepper step
  float zUnitsPerStep;

  // The length of filament the extruder extrudes/retracts per stepper step
  float eUnitsPerStep;

  // The time (in seconds) between interrupts
  float interruptInterval;
};

#endif
