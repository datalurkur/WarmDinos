#ifndef INSTRUCTION_BLOCK_H
#define INSTRUCTION_BLOCK_H

enum {
  STEPPER_A = 0,
  STEPPER_B = 1,
  STEPPER_C = 2,
  STEPPER_E = 3,
  NUMBER_OF_STEPPERS = 4
};

struct InstructionBlock {
  // Set if a stepper should step
  bool step[NUMBER_OF_STEPPERS];

  // Set if a stepper should step forwards, cleared if a stepper should step backwards
  // Disregarded if step is not set
  bool forward[NUMBER_OF_STEPPERS];
};

#endif
