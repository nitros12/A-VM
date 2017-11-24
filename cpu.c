#include "cpu.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @file cpu.c
 * @author Ben Simms
 * @date 2017-11-24
 * @brief Integral VM file.
 */

/**
 * @brief Construct the cpu struct, returning a pointer.
 * @param mem_size Number of bytes to allocate for the cpu's memory.
 */
CPU *make_cpu(size_t mem_size) {
    CPU *cpu = malloc(sizeof(CPU) + sizeof(uint8_t) * mem_size);
    cpu->meta.errd = false;
    cpu->cycles = 0;
    cpu->regs[cur].u8 = 0;
    cpu->flags.running = true;
    return cpu;
}

/**
 * @brief Unpack a cpu_union into a the maximum size safely.
 * @param mem The union to unpack.
 * @param size The size of the union to unpack.
 */
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

/**
 * @brief Pack a uint64_t into a cpu union.
 * @param n The uint64_t to pack.
 * @param size The size to pack.
 */
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

/**
 * @brief Write number into memory.
 * @param loc Location to write to.
 * @param mem Memory to write into memory.
 * @param size Size to write.
 *
 * This writes numbers into memory lsb first, 0xABCD is written into memory [0xCD][0xAB]
 */
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

/**
 * @brief Read location in memory into a cpu union.
 * @param loc Location to read from.
 * @param size Size to read.
 */
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

/**
 * @brief Get a location, either immediate, register or dereference.
 * @param loc Location to get.
 * @param size Size of union to return. (Also size of memory to read if dereferencing).
 *
 * If the 16th bit is set, read from a register.
 * If the 15th bit is set, read from memory.
 * Register read is done before memory read, so [%reg] reads the memory at location reg.
 * If none of the 16th or 15th bits are set, return the location passed packed into a cpu_union.
 */
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

/**
 * @brief Set a location, either memory location, register.
 * @param loc Location to write to.
 * @param size Size to write.
 * @param src Data to write.
 *
 * If no bits set, Write to memory at location `loc`.
 * If register bit (16th) is set and deref bit is not, write to a register.
 * If deref bit is set, pass to getloc and write to memory at returned location.
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

/**
 * @brief Push onto the stack.
 * @param val Value to push onto stack.
 * @param size Size to push.
 *
 * Pushing is post increment, the stack pointer ends after the written data.
 */
void cpu_push(CPU *cpu, cpu_union val, cpu_size size) {
    write_memory(cpu, cpu->regs[stk].u2, cpu_unpack(val, size), size);
    cpu->regs[stk].u2 += size;
}

/**
 * @brief Pop from stack.
 * @param size Size to pop off of the stack.
 */
cpu_union cpu_pop(CPU *cpu, cpu_size size) {
    cpu->regs[stk].u2 -= size;
    return read_memory(cpu, cpu->regs[stk].u2, size);
}

/**
 * @brief Run a cpu until halted.
 */
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
    if (cpu->meta.errd)
        puts(cpu->meta.errmsg);
}

/**
 * @brief Set cpu fail data and halt.
 */
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
