#include "cpu.h"
#include <stdio.h>

/**
 * @file instructions.c
 * @author Ben Simms
 * @date 2017-11-24
 * @brief VM Instruction Set.
 */

#define UNUSED(x) ((void)x)

/**
 * @brief Get an operand, incrementing the current instruction pointer.
 * @param size Size to read and to increment cur by.
 */
uint64_t get_operand(CPU *cpu, cpu_size size) {
    cpu_union r = read_memory(cpu, cpu->regs[cur].u8, size);
    cpu->regs[cur].u8 += (1 << (size - 1));

    // We read params in little endian but our assembler gives big endian, so we need to convert here.
    switch (size) { // AAAAAAAAAAAAAAAAAAHHHHHHHHHHHH
    case w1:
        return r.u1;
    case w2:
        return ((r.u2 & 0xff00) >> 8) | ((r.u2 & 0x00ff) << 8);
    case w4:
        return ((r.u4 & 0x000000ff) << 24)
            | ((r.u4 & 0x0000ff00) << 8)
            | ((r.u4 & 0x0ff00000) >> 8)
            | ((r.u4 & 0xff000000) >> 24);
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

/**
 * @brief Build a binary operation function.
 */
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


/**
 * @brief Build a unary operation function
 */
#define UNARY_FUNC(NAME, OPERATOR)                  \
    void NAME(CPU *cpu, cpu_size size) {            \
        uint16_t a_l = get_operand(cpu, w2);        \
        uint16_t to_l = get_operand(cpu, w2);       \
        cpu_union val = cpu_getloc(cpu, a_l, size); \
        cpu_union out;                              \
        switch (size) {                             \
        case w1:                                    \
            out.u1 = OPERATOR val.u1;               \
            break;                                  \
        case w2:                                    \
            out.u2 = OPERATOR val.u2;               \
            break;                                  \
        case w4:                                    \
            out.u4 = OPERATOR val.u4;               \
            break;                                  \
        case w8:                                    \
            out.u8 = OPERATOR val.u8;               \
            break;                                  \
        }                                           \
        cpu_setloc(cpu, to_l, size, out);           \
    }                                               \


/**
 * @brief data manipulation instructions.
 *
 * These all take 3 operands, op1 and op2 are parameters to the operation. op3 is the destination.
 */

MATH_FUNC(add, +, u)
MATH_FUNC(sub, -, u)
MATH_FUNC(mul, *, u)
MATH_FUNC(udiv, /, u)
MATH_FUNC(idiv, /, s)
MATH_FUNC(shl, <<, u)
MATH_FUNC(shr, >>, u)
MATH_FUNC(sal, <<, s)
MATH_FUNC(sar, >>, s)
MATH_FUNC(and, &, u)
MATH_FUNC(or, |, u)
MATH_FUNC (xor, ^, u)

/**
 * @brief unary operation instructions.
 * Each take two operands, op1 is the parameter, op2 is the destination.
 */

UNARY_FUNC(neg, -)
UNARY_FUNC(pos, +)
UNARY_FUNC(lnot, !)  //: Logical not
UNARY_FUNC(bnot, ~)  //: Bitwise not


/**
 * @brief MOV instruction: copy op1 into op2
 *
 * op1: from location
 * op2: to location
 */
void mov(CPU *cpu, cpu_size size) {
    uint16_t loc = get_operand(cpu, w2);
    uint16_t src = get_operand(cpu, w2);
    cpu_union val = cpu_getloc(cpu, src, size);
    cpu_setloc(cpu, loc, size, val);
}

/**
 * @brief Unsigned extend: extend a number to required size.
 */
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

/**
 * @brief Signed extend: extend a number to required size.
 */
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

/**
 * @brief Jump instruction.
 *
 * Op1: condition, Op2: location to jump to.
 * cond:
 * 0 => always
 * 1 => LessThan
 * 2 => LessEqual
 * 3 => Equal
 * 4 => NotEqual
 * 5 => GreaterThan
 * 6 => GreaterEqual
 */
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
        break;
    case 1:
        if (cpu->flags.le) {
            cpu->regs[cur] = loc;
        }
        break;
    case 2:
        if (cpu->flags.le || cpu->flags.eq) {
            cpu->regs[cur] = loc;
        }
        break;
    case 3:
        if (cpu->flags.eq) {
            cpu->regs[cur] = loc;
        }
        break;
    case 4:
        if (!cpu->flags.eq) {
            cpu->regs[cur] = loc;
        }
        break;
    case 5:
        if (!cpu->flags.le && !cpu->flags.eq) {
            cpu->regs[cur] = loc;
        }
        break;
    case 6:
        if (!cpu->flags.le) {
            cpu->regs[cur] = loc;
        }
        break;
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
= {mov, add, sub, mul, udiv, idiv, shl,  shr, sal,  sar, and,   or,   xor,
       sxu, sxi, halt, jmp,  stks, push, pop, call, ret, getc_, putc_};
