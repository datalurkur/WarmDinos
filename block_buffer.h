#ifndef BLOCK_BUFFER_H
#define BLOCK_BUFFER_H

#include "instruction_block.h"

// This...seems like a good number
#define BLOCK_BUFFER_LENGTH 2048

struct BlockBuffer {
  // Set when the buffer is ready for consumption
  bool ready;

  // Set when the buffer has been consumed
  bool done;

  // Used to limit the number of blocks consumed if the buffer is not full
  int numberOfBlocks;

  InstructionBlock blocks[BLOCK_BUFFER_LENGTH];
};

#endif
