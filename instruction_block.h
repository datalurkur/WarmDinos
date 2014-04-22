#ifndef INSTRUCTION_BLOCK_H
#define INSTRUCTION_BLOCK_H

#include "defines.h"

struct InstructionBlock {
  // Set if a stepper should step
  bool step[NUMBER_OF_STEPPERS];

  // Set if a stepper should step forwards, cleared if a stepper should step backwards
  // Disregarded if step is not set
  bool forward[NUMBER_OF_STEPPERS];
};

#endif
