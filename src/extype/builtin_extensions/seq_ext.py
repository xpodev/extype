from functools import reduce
from typing import Callable, Iterable, List, TypeVar, overload
from ..extension_utils import extend_type_with, extension


__all__ = [
    "extend",
    "SeqExtension"
]


_T = TypeVar("_T")
_U = TypeVar("_U")


class SeqExtension:
    """
    A class that contains methods to extend the builtin `map` and `filter` types.
    """

    @extension
    def map(self: Iterable[_T], fn: Callable[[_T], _U]) -> Iterable[_U]:
        return map(fn, self)

    @extension
    def tolist(self: Iterable[_T]) -> List[_T]:
        return list(self)

    @extension
    def filter(self: Iterable[_T], predicate: Callable[[_T], bool]) -> Iterable[_T]:
        return filter(predicate, self)


def extend():
    extend_type_with(map, SeqExtension)
    extend_type_with(filter, SeqExtension)
    extend_type_with(range, SeqExtension)
    extend_type_with(zip, SeqExtension)
