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

uint8_t getOperand1(CPU *cpu) {
  return GET_OPERAND(uint8_t);
}

uint16_t getOperand2(CPU *cpu) {
  return GET_OPERAND(uint16_t);
}

uint32_t getOperand3(CPU *cpu) {
  return GET_OPERAND(uint32_t);
}

uint64_t getOperand4(CPU *cpu) {
  return GET_OPERAND(uint64_t);
}

/*
* :param loc: start location to set to
* :param size: size to set
* :param src: source to copy from
 */
void cpu_setloc(CPU *cpu, uint16_t loc, size_t size, uint8_t *src) {
  /* TODO:  implement setloc to set the register/ memory location on the cpu*/
}
