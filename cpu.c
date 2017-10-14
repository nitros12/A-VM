#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "cpu.h"

CPU *make_cpu(size_t mem_size) {
  CPU *cpu = malloc(sizeof(CPU) + sizeof(uint8_t) * mem_size);
  cpu->cycles = 0;
  cpu->regs[cur].u8 = 0;
  cpu->flags.running = 1;
  return cpu;
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
    // TODO: rewrite these to use incremental copies and shifts instead of the dodgy pointer casting
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

void run_cpu(CPU *cpu) {
    instr_fp current;
    cpu_size size;
    while (cpu->flags.running) {
        current = GET_INSTR(GET_CUR(cpu));
        size = GET_SIZE(GET_CUR(cpu));
        GET_CUR(cpu)++;
        current(cpu, size);
    }
}
