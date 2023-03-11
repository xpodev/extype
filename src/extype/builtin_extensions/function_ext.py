from typing import Callable, TypeVar
from ..extension_utils import extend_type_with, extension


__all__ = [
    "extend",
    "FunctionExtension"
]


_T = TypeVar("_T")
_U = TypeVar("_U")


class FunctionExtension:
    """
    A class that contains methods to extend the builtin `function` type.
    """

    @extension
    def __matmul__(self: Callable[..., _T], other: Callable[[_T], _U]) -> Callable[..., _U]:
        def wrapper(*args, **kwargs) -> _U:
            return other(self(*args, **kwargs))
        
        return wrapper


def extend():
    extend_type_with(type(extend), FunctionExtension)
    extend_type_with(type(lambda: None), FunctionExtension)
