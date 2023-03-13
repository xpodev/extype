from typing import Callable, TypeVar

from extype.core import implement_protocol_on_type
from ..extension_utils import Protocol, extend_type_with, extension


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
    def __matmul__(self: Callable[[_U], _T], other: Callable[..., _U]) -> Callable[..., _U]:
        def _(*args, **kwargs) -> _U:
            return self(other(*args, **kwargs))
        
        return _


def extend():
    type__function = type(extend)
    type__lambda = type(lambda: None)
    implement_protocol_on_type(type__function, Protocol.Number)
    implement_protocol_on_type(type__lambda, Protocol.Number)

    extend_type_with(type(extend), FunctionExtension)
    extend_type_with(type(lambda: None), FunctionExtension)
