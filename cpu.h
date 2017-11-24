#include <inttypes.h>
#include <stdlib.h>

/**
 * @file cpu.h
 * @author Ben Simms
 * @date 2017-11-24
 */

#define GET_SIZE(X) ((X & 0xC0) >> 6)
#define GET_INSTR(X) (instruction_set[(X & 0x3F)])
#define GET_CUR(X) (X->memory[X->regs[cur].u8])

#define TO_REG(X) ((1 << 15) & X)
#define TO_LOC(X) ((1 << 14) & X)

#define REG(X) ((1 << 15) | X)
#define DEREF(X) ((1 << 14) | X)

#define STRIP_DEREF(X) (X & ~(1 << 14))
#define STRIP_REG(X) (X & ~(1 << 15))
#define STRIP_FLAGS(X) (STRIP_DEREF(STRIP_REG(X)))

/**
 *  @brief Union to simplify different sizes possible in the vm.
 */
typedef union {
    int64_t s8;
    uint64_t u8;
    int32_t s4;
    uint32_t u4;
    int16_t s2;
    uint16_t u2;
    int8_t s1;
    uint8_t u1;
} cpu_union;

/**
 * @brief Map enums to width of cpu_union types.
 */
typedef enum { w1 = 1, w2 = 2, w4 = 3, w8 = 4 } cpu_size;

/**
 * @brief Registers available to the VM.
 */
typedef enum {
    aaa = 0,
    bbb,
    stk,
    bsp,
    acc,
    cur,
} registers;

/**
 * @brief Core VM state.
 */
typedef struct {
    struct {
        uint8_t running : 1;
        uint8_t eq : 1;
        uint8_t le : 1;
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
void cpu_push(CPU *, cpu_union, cpu_size);
cpu_union cpu_pop(CPU *, cpu_size);

extern const instr_fp instruction_set[];
