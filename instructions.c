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

void mov(CPU *, cpu_size);
void add(CPU *, cpu_size);
void mul(CPU *, cpu_size);
void sub(CPU *, cpu_size);
void udiv(CPU *, cpu_size); // TODO:  implement
void idiv(CPU *, cpu_size);
void psh(CPU *, cpu_size);
void pop(CPU *, cpu_size);
void sxt(CPU *, cpu_size);
void axt(CPU *, cpu_size);

const instr_fp instruction_set[] = {
    mov,
    add,
    mul,
    sub,
    udiv,
    idiv,
    psh,
    pop,
    sxt,
    axt,
};



void mov(CPU *cpu, cpu_size size) {
    uint16_t loc = GET_OPERAND(uint16_t, cpu);
}
