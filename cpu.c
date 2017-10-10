#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "cpu.h"

CPU *make_cpu(size_t mem_size) {
  CPU *cpu = malloc(sizeof(CPU) + sizeof(uint8_t) * mem_size);
  cpu->cycles = 0;
  cpu->regs[cur].u8 = 0;
  return cpu;
}

/*
* Instruction decode spec
* Opcodes are 8 bit
* Operands are 8 to 64 bit
* length of operands is dependant on instruction
*/
instr_fp decode_instr(CPU *cpu) {
  uint8_t op = cpu->memory[cpu->regs[cur].u8];
  return instruction_set[op-1];
}

cpu_union cpu_getloc(CPU *cpu, uint16_t loc) {
    bool reg = TO_REG(loc);
    bool deref = TO_LOC(loc);
    cpu_union res = {.u8 = loc & 0b0011111111111111};

    if (reg)
        res.u8 = cpu->regs[res.u8].u8;
    if (deref)
        res.u8 = *(uint64_t *)&cpu->memory[res.u8];
    return res;
}


/*
* :param loc: start location to set to
* :param size: size to set
* :param src: source to copy from
 */
void cpu_setloc(CPU *cpu, uint16_t loc, cpu_size size, cpu_union src) {

    bool reg = TO_REG(loc);
    bool deref = TO_LOC(loc);

    if (deref) {
        loc = cpu_getloc(cpu, loc).u2;
        switch (size) {
            case w1: *(uint8_t *)&cpu->memory[loc] = src.u1; break;
            case w2: *(uint16_t *)&cpu->memory[loc] = src.u2; break;
            case w4: *(uint32_t *)&cpu->memory[loc] = src.u4; break;
            case w8: *(uint64_t *)&cpu->memory[loc] = src.u8; break;
        }
    } else {
        loc &= 0b0011111111111111;
        if (reg)
            switch (size) {
                case w1: cpu->regs[loc].u1 = src.u1; break;
                case w2: cpu->regs[loc].u2 = src.u2; break;
                case w4: cpu->regs[loc].u4 = src.u4; break;
                case w8: cpu->regs[loc].u8 = src.u8; break;
            }
        else
            switch (size) {
                case w1: *(uint8_t *)&cpu->memory[loc] = src.u1; break;
                case w2: *(uint16_t *)&cpu->memory[loc] = src.u2; break;
                case w4: *(uint32_t *)&cpu->memory[loc] = src.u4; break;
                case w8: *(uint64_t *)&cpu->memory[loc] = src.u8; break;
            }
    }
}
