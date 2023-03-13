from enum import IntFlag
from typing import Generic, TypeVar
from .core import enable_magic_method, get_type_dict


_T = TypeVar("_T")


class Protocol(IntFlag):
    Nothing = 0
    Number = 1 << 0
    Sequence = 1 << 1
    Mapping = 1 << 2


def extend_type_with(typ, cls, recurse_parents=True):
    """
    Extends the given type with methods from the given class

    :typ: The type to extend
    :cls: The class containing the methods to add to the given type
    """

    if not isinstance(typ, type):
        raise TypeError(f"argument 'typ' must be a valid type, got {type(typ)}")

    type_dict = get_type_dict(typ)

    if recurse_parents:
        for base in cls.__bases__:
            extend_type_with(typ, base)

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


del _T

__all__ = [
    "extension",
    "extend_type_with"
]
