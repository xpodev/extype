from typing import TypeVar, Union, overload
from ..extension_utils import extend_type_with, extension


__all__ = [
    "extend",
    "StrExtension"
]


_T = TypeVar("_T")

_SENTINEL = object()


class StrExtension:
    """
    A class that contains methods to extend the builtin `str` type.
    """

    @overload
    def to_int(self: str, base: int = 10) -> int: ...
    @overload
    def to_int(self: str, base: int = 10, default: _T = ...) -> Union[int, _T]: ...

    @extension
    def to_int(self: str, base: int = 10, default: _T = _SENTINEL) -> Union[int, _T]:
        try:
            return int(self, base=base)
        except ValueError:
            if default is _SENTINEL:
                raise
            return default

    @overload
    def to_float(self: str) -> float: ...
    @overload
    def to_float(self: str, default: _T = ...) -> Union[float, _T]: ...

    @extension
    def to_float(self: str, default: _T = _SENTINEL) -> Union[float, _T]:
        try:
            return float(self)
        except ValueError:
            if default is _SENTINEL:
                raise
            return default


def extend():
    extend_type_with(str, StrExtension)
