from enum import IntFlag
from functools import reduce
from typing import Callable, Generic, List, TypeVar, overload
import typing
from .core import get_builtin_type_dict, enable_magic_method


class Protocol(IntFlag):
    Nothing = 0
    Number = 1 << 0
    Sequence = 1 << 1
    Mapping = 1 << 2


_T = TypeVar("_T")
_U = TypeVar("_U")

_DEFAULT = object()

list = list


def extend_type_with(typ, cls):
    """
    Extends the given type with methods from the given class

    :typ: The type to extend
    :cls: The class containing the methods to add to the given type
    """

    type_dict = get_builtin_type_dict(typ)

    for name in vars(cls):
        item = getattr(cls, name)
        if isinstance(item, ExtensionObject):
            item = item.original
            if callable(getattr(item, "__set_name__", None)):
                item.__set_name__(typ, name)
            if callable(item):
                if name.startswith("__") and name.endswith("__"):
                    try:
                        enable_magic_method(typ, name)
                    except ValueError:
                        ...
            type_dict[name] = item


class ExtensionObject(Generic[_T]):
    original: _T

    def __init__(self, original: _T):
        self.original = original


def extension(o: _T) -> _T:
    if callable(o) or hasattr(o, "__get__") or hasattr(o, "__set__") or hasattr(o, "__delete__"):
        return ExtensionObject(o)
    else:
        raise TypeError("Only functions and descriptors may be marked as extensions")


def extend_list():
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


def extend_builtin_types():
    extend_list()
