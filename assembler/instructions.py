import enum


class ops(enum.IntEnum):
    (mov, add, sub, mul, udiv, idiv, shl, shr, sar,
     and_, or_, xor, sxu, sxi, halt, jmp, stks, push,
     pop, call, ret, getc, putc, neg, pos, bnot, lnot) = range(27)


class Register(enum.IntEnum):
    aaa = 0
    bbb = 1
    stk = 2
    bsp = 3
    acc = 4
    cur = 5


def hexpad(val: int, size: int = 4):
    """Compile integer into 4 char padded hex."""
    if val > (1 << 8 * size) - 1:
        raise Exception(
            f"Cannot pack value larger than {hex((1 << 8 * size) - 1)}")
    return hex(val)[2:].zfill(size).upper()


def wrap_hexpad(size: int = 4):
    def aaa(func):
        """A decorator to hexpad the result of a function."""

        def inner(*args, **kwargs):
            return hexpad(func(*args, **kwargs), size)

        return inner

    return aaa


def pack_address(value, *, is_reg=False, is_deref=False):
    """Pack an address into an integer."""
    return value | is_reg << 15 | is_deref << 14


class Compilable:
    """Base class for compilable objects."""

    @staticmethod
    def compile(context):
        return NotImplemented


class Instruction:
    def __init__(self, instr, size, *args):
        self.instr = instr
        self.args = args
        self.size = {"1": 1, "2": 2, "4": 3, "8": 4}[size]

    def __str__(self):
        args = " ".join(map(str, self.args))
        return f"{self.instr.name}{self.size} {args}".strip()

    def __len__(self):
        return len(self.args) + 1

    def compile(self, context):
        comp = hexpad(self.instr | self.size << 6, 4) + "".join(
            i.compile(context) for i in self.args)
        print(f"compiling: {self} -> {comp}")
        return comp
