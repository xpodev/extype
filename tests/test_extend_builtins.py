from functools import partial
import typing_extensions
import extype

# `list` is actually the builtin type, but we import it so we get
# proper typing. Note that the static type checker doesn't recognize
# `[]` as an object of type list so it'll not add the extension methods.

from extype.core import implement_protocol_on_type
from extype.extension_utils import extend_type_with, extension, Protocol
from extype.builtin_extensions import list_ext


def test_extend_builtins():
    list_ext.extend()

    assert hasattr(list, "map")

    assert [1, 2, 3].map((1).__add__) == [2, 3, 4]

    assert [1, 2, 3].reduce(int.__add__) == 6


def test_extend_builtins_with_magic():

    def _():
        ...

    function = type(_)

    implement_protocol_on_type(function, Protocol.Number)

    class CallableProtocol(typing_extensions.Protocol):
        def __call__(self, *args, **kwargs):
            ...

    class FunctionExtension(CallableProtocol):
        def __call__(self, *args, **kwargs):
            assert False, "This should never be called, it's just to make the type checker happy"

        @extension
        def __matmul__(self, other):

            def _(*args, **kwargs):
                return self(other(*args, **kwargs))
            return _
            assert isinstance(other, (tuple, dict))

            def wrapper(*args, **kwargs):
                if isinstance(other, tuple):
                    return self(*other, *args, **kwargs)
                return self(*args, **other, **kwargs)
            return wrapper

    extend_type_with(function, FunctionExtension)

    def stringify(x):
        return str(x)

    def add(x, y):
        return x + y

    assert (stringify @ add)(1, 2) == "3"


def test_reverse_methods():
    class TupleExtensions(tuple):
        @extension
        def __rmatmul__(self, other):
            if callable(other):
                return partial(other, *self)
            return NotImplemented

    class FunctionExtensions:
        @extension
        def __matmul__(self, other):
            # NOTE! this will override previous operator @ implementation
            if callable(other):
                def wrapper(*args, **kwargs):
                    return self(other(*args, **kwargs))
                return wrapper
            return NotImplemented

    def add(x, y):
        return x + y

    def stringify(x):
        return str(x)

    function = type(add)
    extend_type_with(function, FunctionExtensions)
    extend_type_with(tuple, TupleExtensions)

    assert (stringify @ (102,))() == "102"
    assert (stringify @ ([1, 2, 3].map(stringify).reduce(add),))() == "123"

    assert (stringify @ add @ (4,))(5) == "9"
