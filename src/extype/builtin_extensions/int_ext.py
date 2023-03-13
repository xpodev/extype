from ..extension_utils import extend_type_with, extension


__all__ = [
    "extend",
    "IntExtension"
]


class IntExtension:
    """
    A class that contains methods to extend the builtin `int` type.
    """

    @extension
    def hex(self: int) -> str:
        return hex(self)

    @extension
    def oct(self: int) -> str:
        return oct(self)

    @extension
    def bin(self: int) -> str:
        return bin(self)


def extend():
    extend_type_with(int, IntExtension)
