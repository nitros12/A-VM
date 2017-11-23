#include "cpu.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

CPU *make_cpu(size_t mem_size) {
    CPU *cpu = malloc(sizeof(CPU) + sizeof(uint8_t) * mem_size);
    cpu->meta.errd = false;
    cpu->cycles = 0;
    cpu->regs[cur].u8 = 0;
    cpu->flags.running = true;
    return cpu;
}

// TODO: see if this can be made better
uint64_t cpu_unpack(cpu_union mem, cpu_size size) {
    switch (size) {
    case w8:
        return mem.u8;
    case w4:
        return mem.u4;
    case w2:
        return mem.u2;
    case w1:
        return mem.u1;
    }
}

cpu_union cpu_pack(uint64_t n, cpu_size size) {
    cpu_union r;
    switch (size) {
    case w8:
        r.u8 = (uint64_t)n;
    case w4:
        r.u4 = (uint32_t)n;
    case w2:
        r.u2 = (uint16_t)n;
    case w1:
        r.u1 = (uint8_t)n;
    }
    return r;
}

void write_memory(CPU *cpu, uint16_t loc, uint64_t mem, cpu_size size) {
    switch (size) {
    case w8:
        cpu->memory[loc + 7] = mem >> (8 * 7) & 0xff;
        cpu->memory[loc + 6] = mem >> (8 * 6) & 0xff;
        cpu->memory[loc + 5] = mem >> (8 * 5) & 0xff;
        cpu->memory[loc + 4] = mem >> (8 * 4) & 0xff;
    case w4:
        cpu->memory[loc + 3] = mem >> (8 * 3) & 0xff;
        cpu->memory[loc + 2] = mem >> (8 * 2) & 0xff;
    case w2:
        cpu->memory[loc + 1] = mem >> (8 * 1) & 0xff;
    case w1:
        cpu->memory[loc + 0] = mem >> (8 * 0) & 0xff;
    } // muh alignment
}

cpu_union read_memory(CPU *cpu, uint16_t loc, cpu_size size) {
#define get_index(l)                                                           \
    ((uint64_t)(cpu->memory[loc + l] << (8 * l) & (0xFF << (8 * l))))
    cpu_union t = {0};

    switch (size) {
    case w8: {
        // t.u8 = 0;
        for (int i = 0; i < 8; i++)
            t.u8 |= get_index(i);
    }
    case w4: {
        // t.u4 = 0;
        for (int i = 0; i < 4; i++)
            t.u4 |= get_index(i);
    }
    case w2: {
        // t.u2 = 0;
        for (int i = 0; i < 2; i++)
            t.u2 |= get_index(i);
    }
    case w1: {
        t.u1 = 0xFF & get_index(0);
    }
    }
    return t;
#undef get_index
}

cpu_union cpu_getloc(CPU *cpu, uint16_t loc, cpu_size size) {
    bool reg = TO_REG(loc);
    bool deref = TO_LOC(loc);
    uint64_t res = STRIP_FLAGS(loc);
    if (reg)
        res = cpu_unpack(cpu->regs[res], size);
    if (deref)
        res = cpu_unpack(read_memory(cpu, loc, size), size);
    return cpu_pack(res, size);
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
        // Read reg/ mem, write to location
        cpu_union res = cpu_getloc(cpu, STRIP_DEREF(loc), w2);
        write_memory(cpu, (uint16_t)cpu_unpack(res, w2), cpu_unpack(src, size),
                     size);
    } else {
        loc = STRIP_FLAGS(loc);
        if (reg) {
            switch (size) {
            case w1:
                cpu->regs[loc].u1 = src.u1;
                break;
            case w2:
                cpu->regs[loc].u2 = src.u2;
                break;
            case w4:
                cpu->regs[loc].u4 = src.u4;
                break;
            case w8:
                cpu->regs[loc].u8 = src.u8;
                break;
            }
        }
        else
            write_memory(cpu, loc, cpu_unpack(src, size), size);
    }
}

void cpu_push(CPU *cpu, cpu_union val, cpu_size size) {
    write_memory(cpu, cpu->regs[stk].u2, cpu_unpack(val, size), size);
    cpu->regs[stk].u2 += size;
}

cpu_union cpu_pop(CPU *cpu, cpu_size size) {
    cpu->regs[stk].u2 -= size;
    return read_memory(cpu, cpu->regs[stk].u2, size);
}

void run_cpu(CPU *cpu) {
    instr_fp current;
    cpu_size size;
    while (cpu->flags.running) {
        current = GET_INSTR(GET_CUR(cpu));
        size = GET_SIZE(GET_CUR(cpu));
        cpu->regs[cur].u8++;
        /* printf("%ld | %3d -> %p, s: %d\n", cpu->regs[cur].u8 - 1, */
        /*        GET_CUR(cpu) & 0x3F, current, size); */
        current(cpu, size);
    }
}

void cpu_panic(CPU *cpu, char *err) {
    cpu->meta.errmsg = err;
    cpu->meta.errd = true;
    cpu->flags.running = false;
}

// Read file into start of cpu memory
void load_file(CPU *cpu, const char *name) {
    FILE *fp = fopen(name, "r");
    if (fp == NULL) {
        fputs("Unable to open file\n", stderr);
        return;
    }

    fseek(fp, 0, SEEK_END);
    size_t size = (size_t)ftell(fp);
    rewind(fp);

    fread(cpu->memory, 2, size, fp);
    fclose(fp);
}

int main(int argc, char **argv) {
    if (argc != 2)
        fprintf(stderr, "Usage: %s <program file>\n", *argv);
    CPU *cpu = make_cpu(1 << 16);
    load_file(cpu, argv[1]);
    run_cpu(cpu);
}
