from functools import reduce
from typing import Callable, List, TypeVar, overload
from ..extension_utils import extend_type_with, extension


__all__ = [
    "extend",
    "ListExtension"
]


_T = TypeVar("_T")
_U = TypeVar("_U")

_DEFAULT = object()


class ListExtension:
    """
    A class that contains methods to extend the builtin `list` type.
    """

    @overload
    def any(self: List[_T]) -> bool: ...
    @overload
    def any(self: List[_T], fn: Callable[[_T], bool]) -> bool: ...

    @extension
    def any(self: List[_T], fn: Callable[[_T], bool] = bool) -> bool:
        return any(map(fn, self))

    @overload
    def all(self: List[_T]) -> bool: ...
    @overload
    def all(self: List[_T], fn: Callable[[_T], bool]) -> bool: ...

    @extension
    def all(self: List[_T], fn: Callable[[_T], bool] = bool) -> bool:
        return all(map(fn, self))

    @extension
    def map(self: List[_T], fn: Callable[[_T], _U]) -> List[_U]:
        return list(map(fn, self))

    @overload
    def reduce(self: List[_T], fn: Callable[[_T, _T], _T]) -> List[_T]: ...
    @overload
    def reduce(self: List[_T], fn: Callable[[_T, _T], _T], initial: _T) -> List[_T]: ...

    @extension
    def reduce(self: List[_T], fn: Callable[[_T, _T], _T], initial: _T = _DEFAULT) -> List[_T]:
        if initial is _DEFAULT:
            return reduce(fn, self)
        return reduce(fn, self, initial)

    @extension
    def filter(self: List[_T], predicate: Callable[[_T], bool]) -> List[_T]:
        return list(filter(predicate, self))

    @extension
    def first(self: List[_T]) -> _T:
        return self[0]

    @extension
    def last(self: List[_T]) -> _T:
        return self[-1]


def extend():
    extend_type_with(list, ListExtension)
