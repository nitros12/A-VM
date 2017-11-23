#include "cpu.h"
#include <stdio.h>

/*
  sxi = 8  # sign extend,  1 -> 2, 10000000 -> 1111111110000000

  class Register(IntEnum):
  stk = 1  # stack pointer
  bas = 2  # base pointer
  acc = 3  # accumulator
  aaa = 4
  bbb = 5
  ccc = 6
  ddd = 7
*/

#define UNUSED(x) ((void)x)

uint64_t get_operand(CPU *cpu, cpu_size size) {
    cpu_union r = read_memory(cpu, cpu->regs[cur].u8, size);
    cpu->regs[cur].u8 += (1 << (size - 1));

    switch (size) { // AAAAAAAAAAAAAAAAAAHHHHHHHHHHHH
    case w1:
        return r.u1;
    case w2:
        return ((r.u2 & 0xff00) >> 8) | ((r.u2 & 0x00ff) << 8);
    case w4:
        return ((r.u4 & 0x000000ff) << 24) | ((r.u4 & 0x0000ff00) << 8)
               | ((r.u4 & 0x00ff00000) >> 8) | ((r.u4 & 0xff000000) >> 24);
    case w8:
        return ((r.u8 & 0xff00000000000000) << 56)
               | ((r.u8 & 0x00ff000000000000) >> 40)
               | ((r.u8 & 0x0000ff0000000000) >> 24)
               | ((r.u8 & 0x000000ff00000000) >> 8)
               | ((r.u8 & 0x00000000ff000000) << 8)
               | ((r.u8 & 0x0000000000ff0000) << 24)
               | ((r.u8 & 0x000000000000ff00) << 40)
               | ((r.u8 & 0x00000000000000ff) << 56);
    }
}

#define MATH_FUNC(NAME, OPERATOR, SIGN)                                        \
    void NAME(CPU *cpu, cpu_size size) {                                       \
        uint16_t a_l = get_operand(cpu, w2);                                   \
        uint16_t b_l = get_operand(cpu, w2);                                   \
        uint16_t to_l = get_operand(cpu, w2);                                  \
        cpu_union a = cpu_getloc(cpu, a_l, size);                              \
        cpu_union b = cpu_getloc(cpu, b_l, size);                              \
        cpu_union out;                                                         \
        switch (size) {                                                        \
        case w1:                                                               \
            out.SIGN##1 = a.SIGN##1 OPERATOR b.SIGN##1;                        \
            break;                                                             \
        case w2:                                                               \
            out.SIGN##2 = a.SIGN##2 OPERATOR b.SIGN##2;                        \
            break;                                                             \
        case w4:                                                               \
            out.SIGN##4 = a.SIGN##4 OPERATOR b.SIGN##4;                        \
            break;                                                             \
        case w8:                                                               \
            out.SIGN##8 = a.SIGN##8 OPERATOR b.SIGN##8;                        \
            break;                                                             \
        }                                                                      \
        cpu_setloc(cpu, to_l, size, out);                                      \
    }

MATH_FUNC(add, +, u)
MATH_FUNC(mul, -, u)
MATH_FUNC(udiv, /, u)
MATH_FUNC(idiv, /, s)
MATH_FUNC(shl, <<, u)
MATH_FUNC(shr, >>, u)
MATH_FUNC(sal, <<, s)
MATH_FUNC(sar, >>, s)
MATH_FUNC(and, &, u)
MATH_FUNC(or, |, u)
MATH_FUNC (xor, ^, u)

// move src into val, 2x u2 operands
void mov(CPU *cpu, cpu_size size) {
    uint16_t loc = get_operand(cpu, w2);
    uint16_t src = get_operand(cpu, w2);
    cpu_union val = cpu_getloc(cpu, src, size);
    cpu_setloc(cpu, loc, size, val);
}

void sxu(CPU *cpu, cpu_size size) {
    uint16_t src = get_operand(cpu, w2);
    uint16_t des = get_operand(cpu, w2);
    uint16_t siz = get_operand(cpu, w2);

    uint64_t val = cpu_unpack(cpu_getloc(cpu, src, size), size);

    switch (siz) {
    case 1:
        cpu_setloc(cpu, des, w1, (cpu_union){.u1 = val});
        break;
    case 2:
        cpu_setloc(cpu, des, w2, (cpu_union){.u2 = val});
        break;
    case 3:
        cpu_setloc(cpu, des, w4, (cpu_union){.u4 = val});
        break;
    case 4:
        cpu_setloc(cpu, des, w8, (cpu_union){.u8 = val});
        break;
    default:
        cpu_panic(cpu, "Invalid size used in instruction: SXD");
    }
}

void sxi(CPU *cpu, cpu_size size) {
    uint16_t src = get_operand(cpu, w2);
    uint16_t des = get_operand(cpu, w2);
    uint16_t siz = get_operand(cpu, w2);

    uint64_t val = cpu_unpack(cpu_getloc(cpu, src, size), size);

    switch (siz) {
    case 1:
        cpu_setloc(cpu, des, w1, (cpu_union){.s1 = val});
        break;
    case 2:
        cpu_setloc(cpu, des, w2, (cpu_union){.s2 = val});
        break;
    case 3:
        cpu_setloc(cpu, des, w4, (cpu_union){.s4 = val});
        break;
    case 4:
        cpu_setloc(cpu, des, w8, (cpu_union){.s8 = val});
        break;
    default:
        cpu_panic(cpu, "Invalid size used in instruction: SXD");
    }
}

void halt(CPU *cpu, cpu_size size) {
    UNUSED(size);
    cpu_panic(cpu, "CPU halted");
}

void jmp(CPU *cpu, cpu_size size) {
    uint8_t cond = get_operand(cpu, w2);
    cpu_union loc = cpu_getloc(cpu, get_operand(cpu, size), size);
    /*
      cond:
      0 => always
      1 => LessThan
      2 => LessEqual
      3 => Equal
      4 => NotEqual
      5 => GreaterThan
      6 => GreaterEqual
    */

    switch (cond) {
    case 0:
        cpu->regs[cur] = loc;
    case 1:
        if (cpu->flags.le) {
            cpu->regs[cur] = loc;
        }
    case 2:
        if (cpu->flags.le || cpu->flags.eq) {
            cpu->regs[cur] = loc;
        }
    case 3:
        if (cpu->flags.eq) {
            cpu->regs[cur] = loc;
        }
    case 4:
        if (!cpu->flags.eq) {
            cpu->regs[cur] = loc;
        }
    case 5:
        if (!cpu->flags.le && !cpu->flags.eq) {
            cpu->regs[cur] = loc;
        }
    case 6:
        if (!cpu->flags.le) {
            cpu->regs[cur] = loc;
        }
    default:
        cpu_panic(cpu, "Invalid condition passed to jmp instruction");
    }
}

void stks(CPU *cpu, cpu_size size) { // stack setup
    UNUSED(size);
    cpu_union addr = cpu_getloc(cpu, get_operand(cpu, w2), w2);
    cpu->regs[stk].u2 = addr.u2;
}

void push(CPU *cpu, cpu_size size) { // post increment
    cpu_union val = cpu_getloc(cpu, get_operand(cpu, size), size);
    cpu_push(cpu, val, size);
}

void pop(CPU *cpu, cpu_size size) { // pre decrement
    uint16_t des = get_operand(cpu, w2);
    cpu_union val = cpu_pop(cpu, size);
    cpu_setloc(cpu, des, size, val);
}

void call(CPU *cpu, cpu_size size) {
    UNUSED(size);
    uint16_t addr = cpu_getloc(cpu, get_operand(cpu, w2), w2).u2;
    cpu_push(cpu, cpu->regs[cur], w2);
    cpu->regs[cur].u2 = addr;
}

void ret(CPU *cpu, cpu_size size) {
    UNUSED(size);
    uint16_t r = cpu_getloc(cpu, get_operand(cpu, w2), w2).u2;
    cpu->regs[stk].u2 -= r; // clear variables
    uint16_t addr = cpu_pop(cpu, w2).u2;
    cpu->regs[cur].u2 = addr;
}

void getc_(CPU *cpu, cpu_size size) {
    UNUSED(size);
    uint16_t des = get_operand(cpu, w2);
    cpu_setloc(cpu, des, w1, (cpu_union){.u1 = getchar()});
}

void putc_(CPU *cpu, cpu_size size) {
    UNUSED(size);
    uint16_t op = get_operand(cpu, w2);
    uint8_t val = cpu_getloc(cpu, op, w2).u2;
    putchar(val);
}

const instr_fp instruction_set[]
    = {mov, add, udiv, idiv, shl,  shr,  sal, sar,  and, or,    xor,
       sxu, sxi, halt, jmp,  stks, push, pop, call, ret, getc_, putc_};
