from typing import Union, overload
from ..extension_utils import extend_type_with, extension


__all__ = [
    "extend",
    "FloatExtension"
]


class FloatExtension:
    """
    A class that contains methods to extend the builtin `float` type.
    """

    @overload
    def round(self: float) -> int: ...
    @overload
    def round(self: float, ndigits: int) -> Union[float, int]: ...

    @extension
    def round(self: float, ndigits: int = None) -> Union[int, float]:
        return round(self, ndigits)


def extend():
    extend_type_with(float, FloatExtension)
