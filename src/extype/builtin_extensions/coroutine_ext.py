from inspect import iscoroutinefunction
from typing import Awaitable, Callable, TypeVar

from ..extension_utils import extend_type_with, extension


__all__ = [
    "extend",
    "CoroutineExtension"
]


_T = TypeVar("_T")
_U = TypeVar("_U")
_E = TypeVar("_E", bound=BaseException)


class CoroutineExtension:
    """
    A class that contains methods to extend coroutine objects (async functions).
    """

    @extension
    def then(self: Awaitable[_T], fn: Callable[[_T], Awaitable[_U] | _U]) -> Awaitable[_U]:
        """
        Maps the result of the awaitable via an optionally async function.
        
        If the function is async, it is awaited in the context of the wrapped awaitable.
        
        Args:
            fn: A function that takes the result of the awaitable and returns a value or awaitable.
            
        Returns:
            An awaitable that resolves to the result of the function.
        """
        async def _then():
            result = fn(await self)
            if iscoroutinefunction(fn):
                return await result
            return result
        
        return _then()

    @extension
    def catch(
        self: Awaitable[_T],
        fn: Callable[[_E], Awaitable[_U] | _U],
        *,
        exception: type[_E] = Exception
    ) -> Awaitable[_T | _U]:
        """
        Catches an exception of the given type and calls the passed function with the caught exception.
        
        If no exception was raised inside the wrapped awaitable, the function will not be called.
        The passed function can optionally return a value to be returned in case of an error.
        The passed function can be either sync or async. If it's async, it is awaited.
        
        Args:
            fn: A function that takes the exception and returns a value or awaitable.
            exception: The type of exception to catch (default: Exception).
            
        Returns:
            An awaitable that resolves to the original result or the result of the error handler.
        """
        async def _catch():
            try:
                return await self
            except exception as e:
                result = fn(e)
                if iscoroutinefunction(fn):
                    return await result
                return result
        
        return _catch()


def extend():
    """
    Applies the coroutine extensions to coroutine objects.
    """
    # Get the coroutine type by creating a coroutine and getting its type
    async def _dummy():
        pass
    
    coro = _dummy()
    coroutine_type = type(coro)
    extend_type_with(coroutine_type, CoroutineExtension)
    
    # Close the coroutine to avoid warnings
    coro.close()
