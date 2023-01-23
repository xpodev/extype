from extypes.core import *
from extypes import extension, extend_type_with, Protocol


def _():
    ...

function = type(_)

implement_protocol_on_type(function, Protocol.Number)


class FunctionExtension:
    @extension
    def __matmul__(self, other):

        def _(*args, **kwargs):
            return self(other(*args, **kwargs))
        return _

    @extension
    def __lshift__(self, other):
        assert isinstance(other, (tuple, dict))

        def wrapper(*args, **kwargs):
            if isinstance(other, tuple):
                return self(*other, *args, **kwargs)
            return self(*args, **other, **kwargs)
        return wrapper


class IntExtension(int):
    @extension
    @property
    def as_bytes(self):
        return self.to_bytes(4, "little")


def foo(*args, **kwargs):
    print(*args, **kwargs)


def bar(x, y):
    return x + y


extend_type_with(function, FunctionExtension)
extend_type_with(int, IntExtension)


print(foo @ bar)
(foo @ bar)(1, 2)

print(bar << ())
print((bar << (3, 4))())


print_hello = foo << ("Hello",)  # I would've used print directly, but we can't yet extend builtin functions
print_hello("World!")

print((2).as_bytes)
