#include <inttypes.h>
#include <stdlib.h>

#define GET_OPERAND(X) (((X *)cpu->memory)[((*(X *)&cpu->regs)[registers.cur])++])

#define TO_REG(X) (0b100000000000 & X)
#define TO_LOC(X) (0b010000000000 & X)

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
  uint8_t memory[];
} CPU;

typedef void (*instr_fp)(CPU *);

extern const instr_fp instruction_set[];
