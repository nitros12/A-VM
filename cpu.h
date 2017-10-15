#include <inttypes.h>
#include <stdlib.h>

// get an operand of type X, and increment the cur by that amount
#define GET_OPERAND(X, CPU) ((*(X *)&CPU->memory[CPU->regs[cur].u8])++)

#define GET_SIZE(X) ((X & 0b11000000) >> 6)
#define GET_INSTR(X) (instruction_set[(X & 0b00111111) - 1])
#define GET_CUR(X) (X->memory[X->regs[cur].u8])

#define TO_REG(X) ((1 << 15) & X)
#define TO_LOC(X) ((1 << 14) & X)

#define REG(X)    ((1 << 15) | X)
#define DEREF(X)  ((1 << 14) | X)

#define STRIP_FLAGS(X) (X & ~((1 << 15) | (1 << 14)))


typedef union {
    int64_t  s8;
    uint64_t u8;
    int32_t  s4;
    uint32_t u4;
    int16_t  s2;
    uint16_t u2;
    int8_t   s1;
    uint8_t  u1;
} cpu_union;

typedef enum {
    w1 = 1,
    w2 = 2,
    w4 = 4,
    w8 = 8
} cpu_size;

typedef enum {
    aaa = 0,
    bbb,
    stk,
    bsp,
    acc,
    cur,
} registers;

typedef struct {
    uint64_t cycles;
    cpu_union regs[6];
    struct {
        int running:1;
        int pos:1;
        int eq:1;
    } flags;
    uint8_t *memory;
} CPU;

typedef void (*instr_fp)(CPU *, cpu_size);

cpu_union cpu_getloc(CPU *, uint16_t);
void cpu_setloc(CPU *, uint16_t, cpu_size, cpu_union);

extern const instr_fp instruction_set[];
