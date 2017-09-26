#include <inttypes.h>
#include <stdlib.h>

#define REG(x) union {\
    int64_t  s8; \
    uint64_t u8; \
    int32_t  s4; \
    uint32_t u4; \
    int16_t  s2; \
    uint16_t u2; \
    int8_t   s1; \
    uint8_t  u1; \
  } (x)

typedef struct {
  REG(aaa);
  REG(bbb);
  REG(stk);
  REG(bsp);
  REG(acc);
  size_t cur;
} registers;

typedef struct {
  uint64_t cycles;
  registers regs;
  uint8_t memory[];
} CPU;

typedef void (*instr_fp)(CPU *);

extern const instr_fp instruction_set[];
