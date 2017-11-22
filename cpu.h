#include <inttypes.h>
#include <stdlib.h>

// get an operand of type X, and increment the cur by that amount
#define GET_OPERAND(T, CPU) ((CPU->memory[CPU->regs[cur].u8]+=(sizeof(T)/sizeof(uint8_t))), \
                             (CPU->memory[CPU->regs[cur].u8-(sizeof(T)/sizeof(uint8_t))]))

#define GET_SIZE(X) ((X & 0xC0) >> 6)
#define GET_INSTR(X) (instruction_set[(X & 0x3F) - 1])
#define GET_CUR(X) (X->memory[X->regs[cur].u8])

#define TO_REG(X) ((1 << 15) & X)
#define TO_LOC(X) ((1 << 14) & X)

#define REG(X)    ((1 << 15) | X)
#define DEREF(X)  ((1 << 14) | X)

#define STRIP_DEREF(X) (X & ~(1 << 14))
#define STRIP_REG(X) (X & ~(1 << 14))
#define STRIP_FLAGS(X) (STRIP_DEREF(STRIP_REG(X)))

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
    struct {
        uint8_t running:1;
        uint8_t pos:1;
        uint8_t eq:1;
    } flags;
    struct {
        char *errmsg;
        char errd;
    } meta;
    uint64_t cycles;
    cpu_union regs[6];
    uint8_t memory[];
} CPU;

typedef void (*instr_fp)(CPU *, cpu_size);

uint64_t cpu_unpack(cpu_union, cpu_size);
cpu_union cpu_pack(uint64_t, cpu_size);
void write_memory(CPU *, uint16_t, uint64_t, cpu_size);
cpu_union read_memory(CPU *, uint16_t, cpu_size);
cpu_union cpu_getloc(CPU *, uint16_t, cpu_size);
void cpu_setloc(CPU *, uint16_t, cpu_size, cpu_union);
CPU *make_cpu(size_t);
void run_cpu(CPU *);
void load_file(CPU *, const char *);
void cpu_panic(CPU *, char *);

extern const instr_fp instruction_set[];
