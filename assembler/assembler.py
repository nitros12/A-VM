"""Parser for asm."""
import pyparsing as pp

from instructions import Compilable, Instruction, ops
from location import Location, match_enum

DEBUG = True


class WrappedExpr:

    def __init__(self, terms):
        if len(terms) > 1:
            raise Exception("Argument somehow has multiple parts")
        self.term = terms.expression

    def __str__(self):
        return str(self.term)


class Label:

    def __init__(self, name):
        self.name = name[1:]


class AssemberContext:

    def __init__(self, data):
        self.data = data
        self.labels = {}

        self.resolve_labels()

    def get_size_before(self, position):
        return sum(map(len, self.data[:position]))

    def resolve_labels(self):
        label_count = 0
        for c, i in enumerate(self.data):
            if isinstance(i, Label):
                self.labels[i.name] = c + label_count
                label_count += 1

        self.data = [i for i in self.data if isinstance(i, Instruction)]
        self.labels = {k: self.get_size_before(v) for k, v in self.labels.items()}

        if DEBUG:
            print("\n".join(str(i) for i in self.data))

    def compile(self):
        return "".join(i.compile(self) for i in self.data)


instr = match_enum(ops) + pp.Optional(pp.oneOf("1 2 4 8"), "1") + pp.ZeroOrMore(Location.expr)
label = pp.Word(":", bodyChars=pp.alphas).setParseAction(lambda t: Label(t[0]))
instr.setParseAction(lambda t: Instruction(*t))
line = instr | label
code = pp.delimitedList(line, delim=";")


def assemble(string: str):
    """Assemble some ASM into machine instructions."""
    data = code.parseString(string)
    print(data)
    context = AssemberContext(data)

    return context.compile()


if __name__ == '__main__':
    import sys
    print(f"\nCompiling {sys.argv[1]}:\n")
    with open(sys.argv[1]) as f:
        program = f.read()
    assembled = assemble(program)
    print(assembled)
    with open(sys.argv[2] if len(sys.argv) > 2 else "a.s", "wb") as f:
        f.write(bytearray.fromhex(assembled))
