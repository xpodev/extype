from functools import reduce
from typing import Callable, List, TypeVar, overload
import typing
from ..extension_utils import extend_type_with, extension


__all__ = [
    "list",
    "extend"
]


_T = TypeVar("_T")
_U = TypeVar("_U")

_DEFAULT = object()

list = list


def _extend_list():
    global list

    class ListExtension(type([])):
        """
        A class that contains methods to extend the builtin `list` type.

        This class inherits the `list` type so that `self` would be of type `list`
        """

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

    extend_type_with(list, ListExtension)

    if typing.TYPE_CHECKING:
        list = ListExtension


def extend():
    _extend_list()
