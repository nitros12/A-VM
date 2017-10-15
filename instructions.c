#include "cpu.h"

/*
  class Instruction(IntEnum):
  mov = 0  # val * 4 + log2(width)  -> 0 * 4 + 1 = mov1, 1 * 4 + 3 = add4
  add = 1
  mul = 2
  sub = 3
  udiv = 4  # unsigned divide
  idiv = 5  # signed divide
  psh = 6
  pop = 7
  sxt = 8  # sign extend,  1 -> 2, 10000000 -> 1111111110000000
  axt = 9  # arith extend, 1 -> 2, 10000000 -> 0000000010000000


  class Register(IntEnum):
  stk = 1  # stack pointer
  bas = 2  # base pointer
  acc = 3  # accumulator
  aaa = 4
  bbb = 5
  ccc = 6
  ddd = 7
*/

#define MATH_FUNC(NAME, OPERATOR, SIGN) void NAME(CPU *cpu, cpu_size size) { \
        uint16_t a_l = GET_OPERAND(uint16_t, cpu);                      \
        uint16_t b_l = GET_OPERAND(uint16_t, cpu);                      \
        uint16_t to_l = GET_OPERAND(uint16_t, cpu);                     \
        cpu_union a = cpu_getloc(cpu, a_l);                             \
        cpu_union b = cpu_getloc(cpu, b_l);                             \
        cpu_union out;                                                  \
        switch (size) {                                                 \
        case w1: out.u1 = a.SIGN##1 OPERATOR b.SIGN##1;                   \
            break;                                                      \
        case w2: out.u2 = a.SIGN##2 OPERATOR b.SIGN##2;                   \
            break;                                                      \
        case w4: out.u4 = a.SIGN##4 OPERATOR b.SIGN##4;                   \
            break;                                                      \
        case w8: out.u8 = a.SIGN##8 OPERATOR b.SIGN##8;                   \
            break;                                                      \
        }                                                               \
        cpu_setloc(cpu, to_l, size, out);                               \
    }

// move src into val, 2x u2 operands
void mov(CPU *cpu, cpu_size size) {
    uint16_t loc = GET_OPERAND(uint16_t, cpu);
    uint16_t src = GET_OPERAND(uint16_t, cpu);
    cpu_union val = cpu_getloc(cpu, src);
    cpu_setloc(cpu, loc, size, val);
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
MATH_FUNC(xor, ^, u)

const instr_fp instruction_set[] = {
    mov,
    add,
    udiv,
    idiv,
    shl,
    shr,
    sal,
    sar,
    and,
    or,
    xor,
};
