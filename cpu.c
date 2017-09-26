#include <stdio.h>
#include <stdlib.h>
#include "cpu.h"

CPU *make_cpu(size_t mem_size) {
  CPU *cpu = malloc(sizeof(CPU) + sizeof(uint8_t) * mem_size);
  cpu->cycles = 0;
  cpu->regs.cur = 0;
  return cpu;
}

/*
* Instruction decode spec
* Opcodes are 8 bit
* Operands are 8 to 64 bit
* length of operands is dependant on instruction
*/
instr_fp decode_instr(CPU *cpu) {
  uint8_t op = cpu->memory[cpu->regs.cur];
  return instruction_set[op-1];
}
