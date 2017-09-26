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

#define INSTR(X) X##1, X##2, X##4, X##8

const instr_fp instruction_set[] = {
    INSTR(mov),  INSTR(add), INSTR(mul),  INSTR(sub), INSTR(udiv),
    INSTR(idiv), INSTR(psh), INSTR(pop),  INSTR(sxt), INSTR(axt),
    INSTR(or),   INSTR(and), INSTR (xor), INSTR(not), INSTR(neg)};

void mov(CPU *cpu) {}
void mov(CPU *cpu) {}
void mov(CPU *cpu) {}
void mov(CPU *cpu) {}
void add(CPU *cpu) {}
void add(CPU *cpu) {}
void add(CPU *cpu) {}
void add(CPU *cpu) {}
void mul(CPU *cpu) {}
void mul(CPU *cpu) {}
void mul(CPU *cpu) {}
void mul(CPU *cpu) {}
void sub(CPU *cpu) {}
void sub(CPU *cpu) {}
void sub(CPU *cpu) {}
void sub(CPU *cpu) {}
void udiv(CPU *cpu) {}
void udiv(CPU *cpu) {}
void udiv(CPU *cpu) {}
void udiv(CPU *cpu) {} // TODO: complete list and implement
